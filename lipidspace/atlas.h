#ifndef ATLAS_H
#define ATLAS_H

#include "lipidspace/Matrix.h"

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

class LipidSpace;
class Lipidome;
class LipidAdduct;

/**
 * Incremental Atlas: a frozen structural frame (lipid -> PCA coords), fixed k-means
 * modules over that frame, and a persisted per-dataset fingerprint store, plus a
 * self-calibrated confidence / out-of-distribution threshold derived from the atlas's
 * own nearest-neighbour distance distribution.
 *
 *   build   : freeze the frame + modules, fingerprint every dataset, calibrate  (one analysis)
 *   fit     : fingerprint a query against the frozen frame/modules -> nearest datasets
 *             + predicted label + confidence + OOD flag + frame coverage         (O(size + N))
 *
 * The frame and per-dataset fingerprints are portable JSON, so an atlas built once can be
 * stored by the caller (e.g. LipidCompass) and passed back with each fit request. Adding a
 * dataset is O(size): fingerprint it and append, with no recompute of the frame or modules.
 */
class Atlas {
public:
    int dims = 0;                                // PCA dimensions of the frame
    int K = 0;                                   // number of modules
    double bandwidth = 0.0;                      // soft-assignment temperature s
    bool soft = true;                            // soft vs hard module assignment
    double ood_threshold = 0.0;                  // 95th percentile of nn_ref
    string label_variable;                       // nominal study variable used for the vote

    map<string, vector<double>> frame;           // canonical lipid name -> coords (dims)
    Matrix centers;                              // K x dims module centers
    vector<string> datasets;                     // fingerprint row order (dataset names)
    vector<Array> fingerprints;                  // N x K compositional module histograms
    vector<map<string, string>> meta;            // per dataset: nominal study variables
    vector<double> nn_ref;                        // sorted nearest-neighbour distances

    // --- Global overview (for the Atlas Explorer view) ---
    // A classical-MDS 2D layout of the datasets from their fingerprint (Hellinger) distances, and an
    // average-linkage leaf order, both precomputed once so GET /atlas/overview is a pass-through.
    vector<vector<double>> embedding;             // N x 2 MDS coordinates (dataset map)
    vector<int> order;                            // clustering leaf order (indices into datasets)

    // --- Nystrom out-of-sample projection (the frame's PCA transform) ---
    // Lets a query lipid that is NOT already in the frame be projected into frame
    // coordinates from its Tanimoto distance row to the reference lipids, instead of
    // being dropped. frame coord[i][k] = sum_c eigenvectors(c,k) * z[i][c], where
    // z[i][c] = (D[i][c] - col_mean[c]) * col_inv_stdev[c] is the column-scaled distance.
    vector<string> ref_names;                    // reference (frame) lipids, distance-column order
    vector<double> col_mean;                     // per-column mean of the L x L distance matrix
    vector<double> col_inv_stdev;                // per-column inverse population stdev (Matrix::scale)
    Matrix eigenvectors;                         // L x dims projection basis (column k = eigenvector k)
    double roundtrip_error = 0.0;                // build self-check: max |reprojected - stored| coord

    // Build the atlas from a completed LipidSpace analysis (frame = global_lipidome->m).
    // Samples in frame_only define the frame (their lipids enter the PCA space + modules)
    // but are NOT stored as datasets or used for calibration -- used to seed a broad
    // support frame (SwissLipids / LIPID MAPS) that outlives the measured corpus.
    void build(LipidSpace &ls, int K, const string &label_variable, bool soft = true,
               const set<string> &frame_only = set<string>());

    // Capture the frame's PCA transform from ls.global_distances for Nystrom projection.
    void capture_transform(LipidSpace &ls);

    // Compute the global overview (MDS embedding + clustering leaf order) from the fingerprints.
    void compute_overview();

    // Project a novel lipid (given its Tanimoto distance row to the reference lipids)
    // into frame coordinates via the captured transform.
    void nystrom_project(const vector<double> &dist_row, vector<double> &coords_out) const;

    // Fingerprint a parsed query lipidome using the frozen frame + modules.
    bool fingerprint_query(const vector<string> &species, const Array &weights,
                           Array &out_fp, double &coverage);

    // Fingerprint a query, projecting lipids not in the frame via Nystrom. ref_lipids are the
    // parsed reference lipids (ref_names order); ls provides the Tanimoto lipid_similarity.
    bool fingerprint_query_projected(LipidSpace &ls, Lipidome *query,
                                     const vector<LipidAdduct*> &ref_lipids,
                                     Array &out_fp, double &coverage, int &n_projected);

    // Rank a query fingerprint: neighbours, per-variable predictions, confidence, OOD, coverage.
    // label_vars selects which nominal study variables to predict; empty = every variable
    // present in the neighbours' metadata (tissue, species, disease, cell type, custom CV terms).
    json rank(Array &fp, double coverage, int k, const set<string> &label_vars = set<string>());

    // Fit a query lipidome -> neighbours, predictions, confidence, OOD flag, coverage.
    json fit(const vector<string> &species, const Array &weights, int k,
             const set<string> &label_vars = set<string>());

    // Fit with Nystrom projection of query lipids not already in the frame.
    json fit_projected(LipidSpace &ls, Lipidome *query,
                       const vector<LipidAdduct*> &ref_lipids, int k,
                       const set<string> &label_vars = set<string>());

    void to_json(json &j);
    void from_json(json &j);
};

#endif /* ATLAS_H */
