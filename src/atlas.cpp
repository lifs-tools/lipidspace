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


json Atlas::fit(const vector<string> &species, const Array &weights, int k) {
    json result;

    Array fp;
    double coverage = 0.0;
    if (!fingerprint_query(species, weights, fp, coverage)) {
        result["error"] = "no query lipids fell in the frozen frame";
        result["coverage"] = coverage;
        return result;
    }

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
}
