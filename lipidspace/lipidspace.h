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
#include "lipidspace/AssistanceFunctions.h"

 
using namespace std;


class LipidSpace {
    
public:
    LipidParser parser;
    map<string, int*> class_matrix;
    vector<LipidAdduct*> all_lipids;
    int cols_for_pca;
    static const vector< vector< vector< vector<int> > > > orders;
    bool keep_sn_position;
    bool ignore_unknown_lipids;
    bool unboundend_distance;
    bool without_quant;
    vector<Table*> lipidomes;
    Table* global_lipidome;
    Progress *progress;
    Array dendrogram_points;
    vector<int> dendrogram_sorting;
    Matrix hausdorff_distances;
    

    LipidSpace();
    ~LipidSpace();
    Table* load_list(string lipid_list_file);
    void compute_PCA_variances(Matrix &m, Array &a);
    void cut_cycle(FattyAcid* fa);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
    double compute_hausdorff_distance(Table* l1, Table* l2);
    //void plot_PCA(Table* table, string output_folder);
    void compute_hausdorff_matrix();
    void report_hausdorff_matrix(string output_folder);
    void compute_global_distance_matrix();
    void separate_matrixes();
    void normalize_intensities();
    void load_table(string table_file);
    void create_dendrogram();
    void store_distance_table(Table* lipidome, string output_folder);
    void run_analysis(Progress *progress = 0);
    std::thread run_analysis_thread(Progress *_progress);
    void reset_analysis();
};


#endif /* LIPIDSPACE_H */
