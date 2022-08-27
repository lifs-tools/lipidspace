#ifndef LIPIDSPACE_H
#define LIPIDSPACE_H

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <sys/stat.h>
#include <immintrin.h>
#include <QThread>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/logging.h"
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/globaldata.h"
#include <chrono>


using namespace std;
using namespace std::chrono;


class LipidSpace : public QThread {
    Q_OBJECT

public:
    LipidParser parser;
    Progress *progress;
    map<string, int*> class_matrix;
    map<string, LipidAdduct*> all_lipids;
    static const int cols_for_pca_init;
    static int cols_for_pca;
    static const vector< vector< vector< vector<int> > > > orders;
    bool keep_sn_position;
    bool ignore_unknown_lipids;
    bool ignore_doublette_lipids;
    bool unboundend_distance;
    bool without_quant;
    vector<Lipidome*> lipidomes;
    vector< map<string, string> > lipid_name_translations;
    Lipidome* global_lipidome;
    vector<Lipidome*> study_lipidomes;
    Array dendrogram_points;
    vector<int> dendrogram_sorting;
    Matrix hausdorff_distances;
    bool analysis_finished;
    map<string, StudyVariableSet> study_variable_values;
    DendrogramNode *dendrogram_root;
    map<string, bool> selection[4];
    vector<Lipidome*> selected_lipidomes;
    map<string, vector<pair<string, double>>> lipid_sortings;
    Matrix global_distances;
    int process_id;
    string target_variable;
    set<string> registered_lipid_classes;
    Matrix statistics_matrix;
    vector<string> statistics_lipids;
    vector< vector<double> > complete_feature_analysis_table;
    vector< set<string> > complete_feature_analysis_lipids;







/*
// An efficient computational algorithm for Hausdorff distance based on points-ruling-out and systematic random sampling
double LipidSpace::HD(Matrix &m1, Matrix &m2){
    double cmax = 0;
    Array U(m1.cols, INFINITY);
    Array V(m2.cols, INFINITY);
    int isp = m1.cols / 2;
    int rows = m1.rows;

    // TMDC computes U, V, and cmax
    int delta = m2.cols / 4;
    int osp = 0;  // TODO random
    for (int x = osp; x < m2.cols; x += delta){
        double cmin = INFINITY;
        double dist1 = INFINITY;
        double dist2 = INFINITY;
        double* m2col = m2.data() + (x * m2.rows);

        for (int y = isp - 1, z = isp; y > 0 || z < m1.cols; y--, z++){
            if (y >= 0){
                double* m1col = m1.data() + (y * m1.rows);
                dist1 = distance_norm(m2col, m1col, rows);
                if (dist1 < U[y]) U[y] = dist1;
            }
            if (z < m1.cols){
                double* m1col = m1.data() + (z * m1.rows);
                dist2 = distance_norm(m2col, m1col, rows);
                if (dist2 < U[z]) U[z] = dist2;
            }
            double dist = min(dist1, dist2);
            if (dist < V[x]) V[x] = dist;
            cmin = min(dist, cmin);

            if (cmin <= cmax){
                isp = dist1 < dist2 ? y : z;
                break;
            }
        }
        if (cmin > cmax) cmax = cmin;
    }

    // RULINGOUT rules out non-contributing points.
    Indexes m1_cols;
    for (int c = 0; c < m1.cols; c++){
        if (U[c] > cmax) m1_cols.push_back(c);
    }
    Matrix m1_r;
    m1_r.rewrite(m1, {}, m1_cols);


    isp = m2.cols / 2;
    for (int x = 0; x < m1_r.cols; ++x){
        double cmin = INFINITY;
        double dist1 = INFINITY;
        double dist2 = INFINITY;
        double* m1col = m1_r.data() + (x * m1_r.rows);

        for (int y = isp - 1, z = isp; y > 0 || z < m2.rows; y--, z++){
            if (y >= 0){
                double* m2col = m2.data() + (y * m2.rows);
                dist1 = distance_norm(m1col, m2col, rows);
                if (dist1 < V[y]) V[y] = dist1;
            }

            if (z < m2.cols){
                double* m2col = m2.data() + (z * m2.rows);
                dist2 = distance_norm(m1col, m2col, rows);
                if (dist2 < V[z]) V[z] = dist2;
            }
            cmin = min(min(dist1, dist2), cmin);
            if (cmin <= cmax){
                isp = dist1 < dist2 ? y : z;
                break;
            }
        }
        if (cmin > cmax) cmax = cmin;
    }

    return cmax;
}
*/



    LipidSpace();
    ~LipidSpace();
    LipidSpace(LipidSpace *ls);
    void run_analysis();
    static void compute_PCA_variances(Matrix &m, Array &a);
    void cut_cycle(FattyAcid* fa);
    static bool is_double(const string& s);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
    double compute_hausdorff_distance(Matrix &m1, Matrix &m2);
    void compute_hausdorff_matrix();
    void report_hausdorff_matrix(string output_folder);
    int compute_global_distance_matrix();
    void separate_matrixes();
    void normalize_intensities();
    void create_dendrogram();
    void store_distance_table(string output_folder, Lipidome* lipidome = 0);
    void run() override;
    void reassembleSelection();
    void reset_analysis();
    LipidAdduct* load_lipid(string lipid_name, map<string, LipidAdduct*> &lipid_set);
    void lipid_analysis(bool report_progress = true);
    void feature_analysis(bool report_progress = true);
    void complete_feature_analysis();

    void load_list(string lipid_list_file);
    void load_table(ImportData *import_data);
    void load_mzTabM(string mzTabM_file);
    int extract_number(string line, int line_number = -1);

signals:
    void fileLoaded();
    void reassembled();

public slots:
    void store_results(string);

private:
    void load_row_table(ImportData *import_data);
    void load_column_table(ImportData *import_data);
    void load_flat_table(ImportData *import_data);

};


#endif /* LIPIDSPACE_H */
