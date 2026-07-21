#include "lipidspace/atlas.h"
#include "lipidspace/lipidspace.h"

#include <algorithm>
#include <cmath>
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


void Atlas::build(LipidSpace &ls, int K_, const string &label_variable_, bool soft_) {
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
        Array fp;
        lipidome->m.generate_fingerprint(centers, lipidome->original_intensities, fp, bandwidth, soft);
        datasets.push_back(lipidome->cleaned_name);
        fingerprints.push_back(fp);

        map<string, string> mv;
        for (auto &kv : lipidome->study_variables) {
            if (kv.second.study_variable_type == NominalStudyVariable) {
                mv[kv.first] = kv.second.nominal_value;
            }
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

    capture_transform(ls);
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
                              Array &out_fp, double &coverage) {
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
    }
    qm.generate_fingerprint(centers, qw, out_fp, bandwidth, soft);
    return true;
}


json Atlas::rank(Array &fp, double coverage, int k) {
    json result;

    int N = (int)fingerprints.size();
    vector<pair<double, int>> dist(N);
    for (int i = 0; i < N; ++i) {
        dist[i] = make_pair(Matrix::hellinger_distance(fingerprints[i], fp), i);
    }
    sort(dist.begin(), dist.end());

    int kk = min(k, N);
    json neighbors = json::array();
    map<string, int> votes;
    for (int r = 0; r < kk; ++r) {
        int idx = dist[r].second;
        json nb;
        nb["dataset"] = datasets[idx];
        nb["distance"] = dist[r].first;
        nb["metadata"] = meta[idx];
        neighbors.push_back(nb);
        if (!label_variable.empty()) {
            auto it = meta[idx].find(label_variable);
            if (it != meta[idx].end()) votes[it->second]++;
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

    if (!votes.empty()) {
        string pred;
        int best = -1, total = 0;
        for (auto &kv : votes) {
            total += kv.second;
            if (kv.second > best) { best = kv.second; pred = kv.first; }
        }
        result["prediction"] = pred;
        result["vote"] = kk > 0 ? (double)best / (double)kk : 0.0;
        result["label_variable"] = label_variable;
    }
    return result;
}


json Atlas::fit(const vector<string> &species, const Array &weights, int k) {
    Array fp;
    double coverage = 0.0;
    if (!fingerprint_query(species, weights, fp, coverage)) {
        json result;
        result["error"] = "no query lipids fell in the frozen frame";
        result["coverage"] = coverage;
        return result;
    }
    return rank(fp, coverage, k);
}


bool Atlas::fingerprint_query_projected(LipidSpace &ls, Lipidome *query,
                                        const vector<LipidAdduct*> &ref_lipids,
                                        Array &out_fp, double &coverage, int &n_projected) {
    int L = (int)ref_names.size();
    int n_total = (int)query->species.size();
    vector<vector<double>> coords_list;
    vector<double> weights_list;
    n_projected = 0;

    for (int i = 0; i < n_total; ++i) {
        double w = (i < (int)query->original_intensities.size()) ? query->original_intensities[i] : 1.0;
        if (w <= 0) continue;

        auto it = frame.find(query->species[i]);
        if (it != frame.end()) {
            coords_list.push_back(it->second);
            weights_list.push_back(w);
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
    qm.generate_fingerprint(centers, qw, out_fp, bandwidth, soft);
    return true;
}


json Atlas::fit_projected(LipidSpace &ls, Lipidome *query,
                          const vector<LipidAdduct*> &ref_lipids, int k) {
    Array fp;
    double coverage = 0.0;
    int n_projected = 0;
    if (!fingerprint_query_projected(ls, query, ref_lipids, fp, coverage, n_projected)) {
        json result;
        result["error"] = "no query lipids could be placed in the frame";
        result["coverage"] = coverage;
        return result;
    }
    json r = rank(fp, coverage, k);
    r["projected_lipids"] = n_projected;
    return r;
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
}
