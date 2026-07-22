#include "lipidspace/atlas.h"
#include "lipidspace/lipidspace.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

using namespace std;


// Soft-assignment temperature: median nearest-centroid distance over the frame lipids,
// matching Matrix::compute_fingerprint_distance_matrix so build/fit fingerprints agree.
static double compute_bandwidth(Matrix &frame, Matrix &centers) {
    if (frame.rows <= 0 || centers.rows <= 0) return 1e-9;
    vector<double> md(frame.rows);
    for (int i = 0; i < frame.rows; ++i) {
        double best = numeric_limits<double>::infinity();
        for (int k = 0; k < centers.rows; ++k) {
            double d = 0.0;
            for (int c = 0; c < frame.cols; ++c) {
                double diff = frame(i, c) - centers(k, c);
                d += diff * diff;
            }
            if (d < best) best = d;
        }
        md[i] = sqrt(best);
    }
    sort(md.begin(), md.end());
    size_t mid = md.size() / 2;
    double median = (md.size() % 2 == 0) ? (md[mid - 1] + md[mid]) / 2.0 : md[mid];
    return median + 1e-9;
}


static double quantile_sorted(const vector<double> &sorted, double q) {
    if (sorted.empty()) return 0.0;
    double pos = q * (sorted.size() - 1);
    size_t lo = (size_t)pos;
    if (lo + 1 >= sorted.size()) return sorted.back();
    double frac = pos - lo;
    return sorted[lo] * (1.0 - frac) + sorted[lo + 1] * frac;
}


void Atlas::build(LipidSpace &ls, int K_, const string &label_variable_, bool soft_,
                  const set<string> &frame_only) {
    soft = soft_;
    label_variable = label_variable_;

    Lipidome *g = ls.global_lipidome;
    dims = g->m.cols;

    // Freeze the frame: canonical lipid name -> PCA coords.
    frame.clear();
    for (int i = 0; i < (int)g->species.size(); ++i) {
        vector<double> coords(dims);
        for (int c = 0; c < dims; ++c) coords[c] = g->m(i, c);
        frame[g->species[i]] = coords;
    }

    // Fixed modules via k-means on the frame.
    K = K_;
    if (K > g->m.rows) K = g->m.rows;
    if (K < 1) K = 1;
    g->m.kmeans(K, centers);
    bandwidth = soft ? compute_bandwidth(g->m, centers) : 1.0;

    // Fingerprint every selected dataset.
    datasets.clear();
    fingerprints.clear();
    meta.clear();
    for (auto lipidome : ls.selected_lipidomes) {
        // Identify datasets and frame-only samples by the raw sample name (lipidome_name), NOT
        // cleaned_name -- the latter carries a " - <file>" suffix from the loader (e.g. the REST
        // temp file "table_file"), which would break caller-side joins on the sample name and stop
        // FrameOnlySamples (passed as raw sample names) from matching. cleaned_name kept as a fallback.
        if (frame_only.count(lipidome->lipidome_name) || frame_only.count(lipidome->cleaned_name))
            continue;   // frame-defining only
        Array fp;
        lipidome->m.generate_fingerprint(centers, lipidome->original_intensities, fp, bandwidth, soft);
        datasets.push_back(lipidome->lipidome_name);
        fingerprints.push_back(fp);

        map<string, string> mv;
        for (auto &kv : lipidome->study_variables) {
            if (kv.first == FILE_STUDY_VARIABLE_NAME) continue;              // loader "Origin", not a label
            if (kv.second.study_variable_type != NominalStudyVariable) continue;
            if (NA_VALUES.count(kv.second.nominal_value)) continue;         // skip missing / NA labels
            mv[kv.first] = kv.second.nominal_value;
        }
        meta.push_back(mv);
    }

    // Calibration: each dataset's nearest-neighbour distance to another dataset.
    int N = (int)fingerprints.size();
    nn_ref.clear();
    for (int i = 0; i < N; ++i) {
        double best = numeric_limits<double>::infinity();
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            double d = Matrix::hellinger_distance(fingerprints[i], fingerprints[j]);
            if (d < best) best = d;
        }
        if (best < numeric_limits<double>::infinity()) nn_ref.push_back(best);
    }
    sort(nn_ref.begin(), nn_ref.end());
    ood_threshold = quantile_sorted(nn_ref, 0.95);

    compute_module_exemplars(6);
    compute_class_signatures();
    compute_overview();
    capture_transform(ls);
}


void Atlas::compute_overview() {
    int N = (int)fingerprints.size();
    embedding.clear();
    order.clear();
    if (N < 1) {
        return;
    }

    // Full N x N Hellinger distance matrix between the dataset fingerprints.
    vector<vector<double>> D(N, vector<double>(N, 0.0));
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            double d = Matrix::hellinger_distance(fingerprints[i], fingerprints[j]);
            D[i][j] = D[j][i] = d;
        }
    }

    // Average-linkage clustering leaf order (adjacent leaves merged early => similar).
    {
        vector<vector<int>> leaves(N);
        vector<vector<double>> cd = D;
        vector<int> sz(N, 1);
        vector<char> alive(N, 1);
        for (int i = 0; i < N; ++i) leaves[i].push_back(i);
        int root = 0;
        for (int step = 0; step < N - 1; ++step) {
            double best = numeric_limits<double>::infinity();
            int bi = -1, bj = -1;
            for (int i = 0; i < N; ++i) {
                if (!alive[i]) continue;
                for (int j = i + 1; j < N; ++j) {
                    if (alive[j] && cd[i][j] < best) { best = cd[i][j]; bi = i; bj = j; }
                }
            }
            if (bi < 0) break;
            for (int k = 0; k < N; ++k) {
                if (alive[k] && k != bi && k != bj) {
                    double nd = (sz[bi] * cd[bi][k] + sz[bj] * cd[bj][k]) / (double)(sz[bi] + sz[bj]);
                    cd[bi][k] = cd[k][bi] = nd;
                }
            }
            leaves[bi].insert(leaves[bi].end(), leaves[bj].begin(), leaves[bj].end());
            sz[bi] += sz[bj];
            alive[bj] = 0;
            root = bi;
        }
        order = leaves[root];
    }

    // Classical MDS (PCoA): 2D coords from the top-2 eigenvectors of the double-centred matrix.
    embedding.assign(N, vector<double>(2, 0.0));
    if (N >= 3) {
        vector<double> rowmean(N, 0.0);
        double grand = 0.0;
        vector<vector<double>> D2(N, vector<double>(N));
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                double v = D[i][j] * D[i][j];
                D2[i][j] = v;
                rowmean[i] += v;
                grand += v;
            }
        }
        for (int i = 0; i < N; ++i) rowmean[i] /= (double)N;
        grand /= (double)N * (double)N;

        Matrix B(N, N);
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                B(i, j) = -0.5 * (D2[i][j] - rowmean[i] - rowmean[j] + grand);
            }
        }
        Array evals;
        Matrix evecs;
        B.compute_eigen_data(evals, evecs, 2);
        for (int k = 0; k < 2 && k < evecs.cols; ++k) {
            double lambda = (k < (int)evals.size() && evals[k] > 0.0) ? evals[k] : 0.0;
            double s = sqrt(lambda);
            for (int i = 0; i < N; ++i) embedding[i][k] = evecs.at(i, k) * s;
        }
    } else {
        for (int i = 0; i < N; ++i) embedding[i][0] = (double)i;
    }
}


void Atlas::compute_module_exemplars(int E) {
    module_exemplars.assign(K, vector<string>());
    if (frame.empty() || centers.rows < K) return;
    for (int k = 0; k < K; ++k) {
        vector<pair<double, string>> d;
        d.reserve(frame.size());
        for (auto &kv : frame) {
            const vector<double> &coord = kv.second;
            double s = 0.0;
            for (int c = 0; c < dims && c < (int)coord.size(); ++c) {
                double diff = coord[c] - centers(k, c);
                s += diff * diff;
            }
            d.push_back(make_pair(s, kv.first));
        }
        int e = min((int)d.size(), E);
        partial_sort(d.begin(), d.begin() + e, d.end());
        vector<string> names;
        for (int i = 0; i < e; ++i) names.push_back(d[i].second);
        module_exemplars[k] = names;
    }
}


void Atlas::compute_class_signatures() {
    int N = (int)fingerprints.size();
    class_baseline.assign(K, 0.0);
    class_signatures.clear();
    if (N == 0) return;
    for (int i = 0; i < N; ++i)
        for (int k = 0; k < K && k < (int)fingerprints[i].size(); ++k)
            class_baseline[k] += fingerprints[i][k];
    for (int k = 0; k < K; ++k) class_baseline[k] /= (double)N;

    map<string, map<string, int>> counts;
    for (int i = 0; i < N; ++i) {
        for (auto &kv : meta[i]) {
            vector<double> &sig = class_signatures[kv.first][kv.second];
            if ((int)sig.size() != K) sig.assign(K, 0.0);
            for (int k = 0; k < K && k < (int)fingerprints[i].size(); ++k) sig[k] += fingerprints[i][k];
            counts[kv.first][kv.second]++;
        }
    }
    for (auto &var : class_signatures)
        for (auto &val : var.second) {
            int c = counts[var.first][val.first];
            if (c > 0) for (int k = 0; k < K; ++k) val.second[k] /= (double)c;
        }
}


void Atlas::capture_transform(LipidSpace &ls) {
    Lipidome *g = ls.global_lipidome;
    Matrix &D = ls.global_distances;   // raw L x L Tanimoto distance matrix (pre-PCA)
    int L = (int)g->species.size();
    if (D.rows != L || D.cols != L || dims <= 0) {   // frame too small for a PCA transform
        ref_names.clear(); col_mean.clear(); col_inv_stdev.clear();
        eigenvectors.reset(0, 0); roundtrip_error = 0.0;
        return;
    }
    ref_names = g->species;

    // Per-column mean and inverse population stdev, matching Matrix::scale exactly.
    col_mean.assign(L, 0.0);
    col_inv_stdev.assign(L, 0.0);
    for (int c = 0; c < L; ++c) {
        double mean = 0.0;
        for (int r = 0; r < L; ++r) mean += D(r, c);
        mean /= (double)L;
        double ss = 0.0;
        for (int r = 0; r < L; ++r) ss += sq(mean - D(r, c));
        col_mean[c] = mean;
        col_inv_stdev[c] = sqrt((double)L / ss);
    }

    // Recompute the eigenvectors the same way run_analysis's PCA did (deterministic,
    // seeded Lanczos), so they are consistent with the stored frame coords g->m.
    Matrix Z(D);                 // copy the raw distances
    Z.scale();                   // column z-transform (same as PCA)
    Matrix cov;
    Z.covariance_matrix(cov);    // cov = Z^T Z / (L-1), L x L
    Array evals;
    cov.compute_eigen_data(evals, eigenvectors, dims);   // eigenvectors: L x dims

    // Sign-align each eigenvector so its projection reproduces the stored frame coords.
    for (int k = 0; k < dims; ++k) {
        double s = 0.0;
        for (int i = 0; i < L; ++i) {
            double proj = 0.0;
            for (int c = 0; c < L; ++c) proj += eigenvectors.at(c, k) * Z.at(i, c);
            s += proj * g->m.at(i, k);
        }
        if (s < 0.0) for (int c = 0; c < L; ++c) eigenvectors(c, k) = -eigenvectors.at(c, k);
    }

    // Round-trip self-check: reproject every reference lipid and compare to g->m.
    roundtrip_error = 0.0;
    for (int i = 0; i < L; ++i) {
        for (int k = 0; k < dims; ++k) {
            double proj = 0.0;
            for (int c = 0; c < L; ++c) proj += eigenvectors.at(c, k) * Z.at(i, c);
            roundtrip_error = max(roundtrip_error, fabs(proj - g->m.at(i, k)));
        }
    }
}


void Atlas::nystrom_project(const vector<double> &dist_row, vector<double> &coords_out) const {
    int L = (int)ref_names.size();
    coords_out.assign(dims, 0.0);
    for (int k = 0; k < dims; ++k) {
        double acc = 0.0;
        for (int c = 0; c < L; ++c) {
            double z = (dist_row[c] - col_mean[c]) * col_inv_stdev[c];
            acc += eigenvectors.at(c, k) * z;
        }
        coords_out[k] = acc;
    }
}


bool Atlas::fingerprint_query(const vector<string> &species, const Array &weights,
                              Array &out_fp, double &coverage,
                              vector<string> *placed_names, Matrix *contributions) {
    vector<int> present;
    for (int i = 0; i < (int)species.size(); ++i) {
        bool has_weight = (i >= (int)weights.size()) || weights[i] > 0;
        if (has_weight && frame.find(species[i]) != frame.end()) present.push_back(i);
    }
    coverage = species.empty() ? 0.0 : (double)present.size() / (double)species.size();
    if (present.empty()) return false;

    Matrix qm((int)present.size(), dims);
    Array qw;
    qw.resize(present.size());
    for (int r = 0; r < (int)present.size(); ++r) {
        const vector<double> &coords = frame.at(species[present[r]]);
        for (int c = 0; c < dims; ++c) qm(r, c) = coords[c];
        qw[r] = (present[r] < (int)weights.size()) ? weights[present[r]] : 1.0;
        if (placed_names) placed_names->push_back(species[present[r]]);
    }
    qm.generate_fingerprint(centers, qw, out_fp, bandwidth, soft, contributions);
    return true;
}


json Atlas::rank(Array &fp, double coverage, int k, const set<string> &label_vars) {
    json result;

    int N = (int)fingerprints.size();
    vector<pair<double, int>> dist(N);
    for (int i = 0; i < N; ++i) {
        dist[i] = make_pair(Matrix::hellinger_distance(fingerprints[i], fp), i);
    }
    sort(dist.begin(), dist.end());

    int kk = min(k, N);
    json neighbors = json::array();
    map<string, map<string, int>> votes;   // study variable -> value -> count
    for (int r = 0; r < kk; ++r) {
        int idx = dist[r].second;
        json nb;
        nb["dataset"] = datasets[idx];
        nb["distance"] = dist[r].first;
        nb["metadata"] = meta[idx];
        neighbors.push_back(nb);
        for (auto &kv : meta[idx]) {
            if (label_vars.empty() || label_vars.count(kv.first)) votes[kv.first][kv.second]++;
        }
    }

    double nn = dist.empty() ? 0.0 : dist[0].first;
    int greater = 0;
    for (double v : nn_ref) if (v > nn) greater++;
    double confidence = nn_ref.empty() ? 0.0 : (double)greater / (double)nn_ref.size();

    result["neighbors"] = neighbors;
    result["nn_distance"] = nn;
    result["confidence"] = confidence;
    result["ood"] = nn > ood_threshold;
    result["coverage"] = coverage;

    // Per-variable majority-vote predictions over the neighbours. Covers every nominal study
    // variable present (tissue, species, disease, cell type, custom CV terms), or the requested
    // subset in label_vars.
    json predictions = json::object();
    for (auto &var : votes) {
        string pred;
        int best = -1;
        for (auto &vc : var.second) {
            if (vc.second > best) { best = vc.second; pred = vc.first; }
        }
        json p;
        p["prediction"] = pred;
        p["vote"] = kk > 0 ? (double)best / (double)kk : 0.0;
        predictions[var.first] = p;
    }
    result["predictions"] = predictions;

    // Backward-compatible top-level prediction for the default label variable.
    if (!label_variable.empty() && predictions.contains(label_variable)) {
        result["prediction"] = predictions[label_variable]["prediction"];
        result["vote"] = predictions[label_variable]["vote"];
        result["label_variable"] = label_variable;
    }
    return result;
}


json Atlas::fit(const vector<string> &species, const Array &weights, int k,
                const set<string> &label_vars,
                int top_n_dominant, int top_n_lipids, int top_n_modules) {
    Array fp;
    double coverage = 0.0;
    vector<string> placed;
    Matrix C;
    if (!fingerprint_query(species, weights, fp, coverage, &placed, &C)) {
        json result;
        result["error"] = "no query lipids fell in the frozen frame";
        result["coverage"] = coverage;
        return result;
    }
    json result = rank(fp, coverage, k, label_vars);
    result["contributions"] = attribute(placed, C, fp, result["neighbors"], result["predictions"],
                                         top_n_dominant, top_n_lipids, top_n_modules);
    return result;
}


bool Atlas::fingerprint_query_projected(LipidSpace &ls, Lipidome *query,
                                        const vector<LipidAdduct*> &ref_lipids,
                                        Array &out_fp, double &coverage, int &n_projected,
                                        vector<string> *placed_names, Matrix *contributions) {
    int L = (int)ref_names.size();
    int n_total = (int)query->species.size();
    vector<vector<double>> coords_list;
    vector<double> weights_list;
    vector<string> names_list;
    n_projected = 0;

    for (int i = 0; i < n_total; ++i) {
        double w = (i < (int)query->original_intensities.size()) ? query->original_intensities[i] : 1.0;
        if (w <= 0) continue;

        auto it = frame.find(query->species[i]);
        if (it != frame.end()) {
            coords_list.push_back(it->second);
            weights_list.push_back(w);
            names_list.push_back(query->species[i]);
            continue;
        }
        // Not in the frame: Nystrom-project from its Tanimoto distance row to the references.
        LipidAdduct *q = (i < (int)query->lipids.size()) ? query->lipids[i] : nullptr;
        if (q == nullptr || ref_lipids.empty()) continue;

        vector<double> dist_row(L);
        for (int c = 0; c < L; ++c) {
            if (ref_lipids[c] == nullptr) { dist_row[c] = 1.0; continue; }
            int un = 0, in = 0;
            ls.lipid_similarity(q, ref_lipids[c], un, in);
            dist_row[c] = (un > 0) ? (1.0 - (double)in / (double)un) : 1.0;
        }
        vector<double> coords;
        nystrom_project(dist_row, coords);
        coords_list.push_back(coords);
        weights_list.push_back(w);
        names_list.push_back(query->species[i]);
        ++n_projected;
    }

    coverage = n_total > 0 ? (double)coords_list.size() / (double)n_total : 0.0;
    if (coords_list.empty()) return false;

    Matrix qm((int)coords_list.size(), dims);
    Array qw;
    qw.resize(coords_list.size());
    for (int r = 0; r < (int)coords_list.size(); ++r) {
        for (int c = 0; c < dims; ++c) qm(r, c) = coords_list[r][c];
        qw[r] = weights_list[r];
    }
    qm.generate_fingerprint(centers, qw, out_fp, bandwidth, soft, contributions);
    if (placed_names) *placed_names = names_list;
    return true;
}


json Atlas::fit_projected(LipidSpace &ls, Lipidome *query,
                          const vector<LipidAdduct*> &ref_lipids, int k,
                          const set<string> &label_vars,
                          int top_n_dominant, int top_n_lipids, int top_n_modules) {
    Array fp;
    double coverage = 0.0;
    int n_projected = 0;
    vector<string> placed;
    Matrix C;
    if (!fingerprint_query_projected(ls, query, ref_lipids, fp, coverage, n_projected, &placed, &C)) {
        json result;
        result["error"] = "no query lipids could be placed in the frame";
        result["coverage"] = coverage;
        return result;
    }
    json r = rank(fp, coverage, k, label_vars);
    r["projected_lipids"] = n_projected;
    r["contributions"] = attribute(placed, C, fp, r["neighbors"], r["predictions"],
                                    top_n_dominant, top_n_lipids, top_n_modules);
    return r;
}


json Atlas::attribute(const vector<string> &lipid_names, Matrix &C, Array &fp,
                      const json &neighbors, const json &predictions,
                      int top_n_dominant, int top_n_lipids, int top_n_modules) {
    json out;
    int n = (int)lipid_names.size();
    int Kc = (int)fp.size();

    // dominant lipids: row sums of C are the compositional weights.
    json jdom = json::array();
    if (top_n_dominant > 0 && n > 0) {
        vector<pair<double, int>> w(n);
        for (int i = 0; i < n; ++i) {
            double s = 0.0;
            for (int k = 0; k < Kc && k < C.cols; ++k) s += C(i, k);
            w[i] = make_pair(s, i);
        }
        int td = min(top_n_dominant, n);
        partial_sort(w.begin(), w.begin() + td, w.end(), greater<pair<double, int>>());
        for (int r = 0; r < td; ++r) {
            json e; e["lipid"] = lipid_names[w[r].second]; e["weight"] = w[r].first;
            jdom.push_back(e);
        }
    }
    out["dominant_lipids"] = jdom;

    json byvar = json::object();
    for (auto it = predictions.begin(); it != predictions.end(); ++it) {
        const string var = it.key();
        string pred = it.value().value("prediction", string());

        // voting neighbours = those carrying var == pred; collect their fingerprints.
        vector<Array*> qs;
        for (auto &nb : neighbors) {
            if (!nb.contains("metadata")) continue;
            const json &md = nb["metadata"];
            if (!md.contains(var) || md[var].get<string>() != pred) continue;
            string dname = nb.value("dataset", string());
            for (int di = 0; di < (int)datasets.size(); ++di)
                if (datasets[di] == dname) { qs.push_back(&fingerprints[di]); break; }
        }

        vector<double> BC(Kc, 0.0);
        for (Array *q : qs)
            for (int k = 0; k < Kc && k < (int)q->size(); ++k)
                BC[k] += sqrt(fp[k] * (*q)[k]);

        json jmods = json::array();
        if (top_n_modules > 0) {
            vector<pair<double, int>> mk(Kc);
            for (int k = 0; k < Kc; ++k) mk[k] = make_pair(BC[k], k);
            int tm = min(top_n_modules, Kc);
            partial_sort(mk.begin(), mk.begin() + tm, mk.end(), greater<pair<double, int>>());
            for (int r = 0; r < tm; ++r) {
                int k = mk[r].second;
                json m; m["module"] = k; m["score"] = mk[r].first;
                m["exemplars"] = (k < (int)module_exemplars.size()) ? module_exemplars[k] : vector<string>();
                jmods.push_back(m);
            }
        }

        json jlips = json::array();
        if (top_n_lipids > 0 && n > 0) {
            vector<double> contrib(n, 0.0);
            double total = 0.0;
            for (int i = 0; i < n; ++i) {
                double s = 0.0;
                for (int k = 0; k < Kc && k < C.cols; ++k)
                    if (fp[k] > 0.0) s += C(i, k) * (BC[k] / fp[k]);
                contrib[i] = s; total += s;
            }
            vector<pair<double, int>> ci(n);
            for (int i = 0; i < n; ++i) ci[i] = make_pair(contrib[i], i);
            int tl = min(top_n_lipids, n);
            partial_sort(ci.begin(), ci.begin() + tl, ci.end(), greater<pair<double, int>>());
            for (int r = 0; r < tl; ++r) {
                int i = ci[r].second;
                json e; e["lipid"] = lipid_names[i]; e["score"] = contrib[i];
                e["share"] = total > 0.0 ? contrib[i] / total : 0.0;
                jlips.push_back(e);
            }
        }

        json v; v["prediction"] = pred; v["lipids"] = jlips; v["modules"] = jmods;
        byvar[var] = v;
    }
    out["by_variable"] = byvar;
    return out;
}


void Atlas::to_json(json &j) {
    j["dims"] = dims;
    j["K"] = K;
    j["bandwidth"] = bandwidth;
    j["soft"] = soft;
    j["ood_threshold"] = ood_threshold;
    j["label_variable"] = label_variable;

    json jframe = json::object();
    for (auto &kv : frame) jframe[kv.first] = kv.second;
    j["frame"] = jframe;

    json jc = json::array();
    for (int r = 0; r < centers.rows; ++r) {
        json row = json::array();
        for (int c = 0; c < centers.cols; ++c) row.push_back(centers(r, c));
        jc.push_back(row);
    }
    j["centers"] = jc;

    j["datasets"] = datasets;

    json jf = json::array();
    for (auto &f : fingerprints) {
        json row = json::array();
        for (double x : f) row.push_back(x);
        jf.push_back(row);
    }
    j["fingerprints"] = jf;

    json jm = json::array();
    for (auto &mv : meta) jm.push_back(mv);
    j["meta"] = jm;

    j["nn_ref"] = nn_ref;

    // Nystrom projection transform
    j["roundtrip_error"] = roundtrip_error;
    j["ref_names"] = ref_names;
    j["col_mean"] = col_mean;
    j["col_inv_stdev"] = col_inv_stdev;
    json jev = json::array();
    for (int r = 0; r < eigenvectors.rows; ++r) {
        json row = json::array();
        for (int c = 0; c < eigenvectors.cols; ++c) row.push_back(eigenvectors.at(r, c));
        jev.push_back(row);
    }
    j["eigenvectors"] = jev;

    // Global overview
    json jemb = json::array();
    for (auto &e : embedding) {
        json row = json::array();
        row.push_back(e.size() > 0 ? e[0] : 0.0);
        row.push_back(e.size() > 1 ? e[1] : 0.0);
        jemb.push_back(row);
    }
    j["embedding"] = jemb;
    j["order"] = order;

    // Contributions (build-time)
    json jme = json::array();
    for (auto &ex : module_exemplars) jme.push_back(ex);
    j["module_exemplars"] = jme;
    j["class_baseline"] = class_baseline;
    json jcs = json::object();
    for (auto &var : class_signatures) {
        json jv = json::object();
        for (auto &val : var.second) jv[val.first] = val.second;
        jcs[var.first] = jv;
    }
    j["class_signatures"] = jcs;
}


void Atlas::from_json(json &j) {
    dims = j.at("dims").get<int>();
    K = j.at("K").get<int>();
    bandwidth = j.at("bandwidth").get<double>();
    soft = j.value("soft", true);
    ood_threshold = j.value("ood_threshold", 0.0);
    label_variable = j.value("label_variable", string());

    frame.clear();
    for (auto it = j.at("frame").begin(); it != j.at("frame").end(); ++it) {
        frame[it.key()] = it.value().get<vector<double>>();
    }

    json &jc = j.at("centers");
    int R = (int)jc.size();
    int C = R > 0 ? (int)jc[0].size() : dims;
    centers.reset(R, C);
    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) centers(r, c) = jc[r][c].get<double>();
    }

    datasets = j.at("datasets").get<vector<string>>();

    fingerprints.clear();
    for (auto &row : j.at("fingerprints")) {
        Array fp;
        fp.resize(row.size());
        for (int c = 0; c < (int)row.size(); ++c) fp[c] = row[c].get<double>();
        fingerprints.push_back(fp);
    }

    meta.clear();
    for (auto &mv : j.at("meta")) meta.push_back(mv.get<map<string, string>>());

    nn_ref = j.at("nn_ref").get<vector<double>>();

    // Nystrom projection transform (optional; absent in older atlases)
    roundtrip_error = j.value("roundtrip_error", 0.0);
    ref_names = j.value("ref_names", vector<string>());
    col_mean = j.value("col_mean", vector<double>());
    col_inv_stdev = j.value("col_inv_stdev", vector<double>());
    if (j.contains("eigenvectors") && j["eigenvectors"].is_array() && !j["eigenvectors"].empty()) {
        json &jev = j.at("eigenvectors");
        int R = (int)jev.size();
        int C = (int)jev[0].size();
        eigenvectors.reset(R, C);
        for (int r = 0; r < R; ++r) {
            for (int c = 0; c < C; ++c) eigenvectors(r, c) = jev[r][c].get<double>();
        }
    } else {
        eigenvectors.reset(0, 0);
    }

    embedding.clear();
    if (j.contains("embedding") && j["embedding"].is_array()) {
        for (auto &row : j["embedding"]) {
            double x = row.size() > 0 ? row[0].get<double>() : 0.0;
            double y = row.size() > 1 ? row[1].get<double>() : 0.0;
            embedding.push_back({x, y});
        }
    }
    order = j.value("order", vector<int>());

    module_exemplars.clear();
    if (j.contains("module_exemplars"))
        for (auto &ex : j["module_exemplars"]) module_exemplars.push_back(ex.get<vector<string>>());
    class_baseline = j.value("class_baseline", vector<double>());
    class_signatures.clear();
    if (j.contains("class_signatures"))
        for (auto it = j["class_signatures"].begin(); it != j["class_signatures"].end(); ++it)
            for (auto vt = it.value().begin(); vt != it.value().end(); ++vt)
                class_signatures[it.key()][vt.key()] = vt.value().get<vector<double>>();
}
