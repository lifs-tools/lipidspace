#ifndef LIPIDSPACE_H
#define LIPIDSPACE_H

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <sys/stat.h>
#include <immintrin.h>
#include <QtCore>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/logging.h"
#include "lipidspace/AssistanceFunctions.h"

#define UNDEFINED_LIPID "UNDEFINED"
 
using namespace std;

enum TableType {ROW_TABLE, COLUMN_TABLE, PIVOT_TABLE};


class LipidSpace : public QObject {
    Q_OBJECT
    
public:
    LipidParser parser;
    map<string, int*> class_matrix;
    vector<LipidAdduct*> all_lipids;
    static const int cols_for_pca_init;
    static int cols_for_pca;
    static const vector< vector< vector< vector<int> > > > orders;
    bool keep_sn_position;
    bool ignore_unknown_lipids;
    bool ignore_doublette_lipids;
    bool unboundend_distance;
    bool without_quant;
    vector<Table*> lipidomes;
    Table* global_lipidome;
    Progress *progress;
    Array dendrogram_points;
    vector<int> dendrogram_sorting;
    Matrix hausdorff_distances;
    bool analysis_finished;
    map<string, set<string>> feature_values;
    DendrogramNode *dendrogram_root;
    map<string, bool> selection[4];
    vector<Table*> selected_lipidomes;
    map<string, vector<string>> lipid_sortings;
    

    LipidSpace();
    ~LipidSpace();
    static void compute_PCA_variances(Matrix &m, Array &a);
    void cut_cycle(FattyAcid* fa);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
    double compute_hausdorff_distance(Matrix &m1, Matrix &m2);
    void compute_hausdorff_matrix();
    void report_hausdorff_matrix(string output_folder);
    bool compute_global_distance_matrix();
    void separate_matrixes();
    void normalize_intensities();
    void create_dendrogram();
    void store_distance_table(Table* lipidome, string output_folder);
    void run_analysis(Progress *progress = 0);
    void reassembleSelection();
    std::thread run_analysis_thread(Progress *_progress);
    void reset_analysis();
    LipidAdduct* load_lipid(string lipid_name, set<string> &lipid_set, bool &ignore_lipid);
    
    void load_list(string lipid_list_file);
    void load_row_table(string table_file, vector<TableColumnType> *column_types = 0);
    void load_column_table(string table_file, vector<TableColumnType> *column_types);
    void load_pivot_table(string table_file, vector<TableColumnType> *column_types);
    
signals:
    void fileLoaded();
    void reassembled();
};


#endif /* LIPIDSPACE_H */