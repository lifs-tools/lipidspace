#ifndef ATLAS_H
#define ATLAS_H

#include "lipidspace/Matrix.h"

#include <map>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

class LipidSpace;

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

    // Build the atlas from a completed LipidSpace analysis (frame = global_lipidome->m).
    void build(LipidSpace &ls, int K, const string &label_variable, bool soft = true);

    // Fingerprint a parsed query lipidome using the frozen frame + modules.
    bool fingerprint_query(const vector<string> &species, const Array &weights,
                           Array &out_fp, double &coverage);

    // Fit a query lipidome -> neighbours, prediction, confidence, OOD flag, coverage.
    json fit(const vector<string> &species, const Array &weights, int k);

    void to_json(json &j);
    void from_json(json &j);
};

#endif /* ATLAS_H */
