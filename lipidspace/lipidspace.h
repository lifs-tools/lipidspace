#ifndef LIPIDSPACE_H
#define LIPIDSPACE_H

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <sys/stat.h>
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
    map<string, pair<int, int>> class_matrix;
    map<string, LipidAdduct*> all_lipids;
    static const int cols_for_pca_init;
    static int cols_for_pca;
    static const vector< vector< vector< pair<int, int> > > > orders;
    bool keep_sn_position;
    bool ignore_unknown_lipids;
    bool ignore_doublette_lipids;
    bool unboundend_distance;
    bool without_quant;
    vector<Lipidome*> lipidomes;
    vector< map<string, string> > lipid_name_translations;
    Lipidome* global_lipidome;
    map<string, vector<Lipidome*>> group_lipidomes;
    Array dendrogram_points;
    vector<int> dendrogram_sorting;
    Matrix hausdorff_distances;
    bool analysis_finished;
    map<string, StudyVariableSet> study_variable_values;
    DendrogramNode *dendrogram_root;
    map<string, bool> selection[4];
    vector<Lipidome*> selected_lipidomes;
    map<string, SortVector<string, double> > lipid_sortings;
    Matrix global_distances;
    int process_id;
    string target_variable;
    set<string> registered_lipid_classes;
    Matrix statistics_matrix;
    vector<string> statistics_lipids;
    vector< vector<double> > complete_feature_analysis_table;
    vector< set<string> > complete_feature_analysis_lipids;


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
    void analytics(string);

public slots:
    void store_results(string);

private:
    void load_row_table(ImportData *import_data);
    void load_column_table(ImportData *import_data);
    void load_flat_table(ImportData *import_data);

};


#endif /* LIPIDSPACE_H */
