#ifndef LIPIDSPACE_H
#define LIPIDSPACE_H

#include <iostream>
#include <vector>
#include <string>
#include "cppgoslin/cppgoslin.h"
#include <math.h>
#include <algorithm>
#include <sys/stat.h>
#include <immintrin.h>

//#include "LipidSpace/matplotlibcpp.h"
#include "lipidspace/Matrix.h"

 
using namespace std;
//namespace plt = matplotlibcpp; 


enum Linkage {SINGLE, COMPLETE};



class Node {
public:
    set<int> indexes;
    string name;
    Node* left_child;
    Node* right_child;
    double distance;
    
    Node(int index, string _name);
    Node(Node* n1, Node* n2, double d);
    ~Node();
    double* plot(int i, vector<string>* sorted_ticks);
};




class Table {
public:
    string file_name;
    vector<string> species;
    vector<string> classes;
    vector<LipidAdduct*> lipids;
    Array intensities;
    Matrix m;
    
    Table(string lipid_list_file) : file_name(lipid_list_file) {}
};


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
    
    
    
    LipidSpace();
    ~LipidSpace();
    Table* load_list(string lipid_list_file);
    void compute_PCA_variances(Matrix &m, Array &a);
    void cut_cycle(FattyAcid* fa);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
    double compute_hausdorff_distance(Table* l1, Table* l2);
    //void plot_PCA(Table* table, string output_folder);
    void compute_hausdorff_matrix(Matrix &m);
    void report_hausdorff_matrix(Matrix &distance_matrix, string output_folder);
    void compute_global_distance_matrix();
    void separate_matrixes();
    void normalize_intensities();
    void load_table(string table_file);
    //void plot_dendrogram(vector<Table*>* lipidomes, Matrix &m, string output_folder);
    void store_distance_table(Table* lipidome, string output_folder);
    void run_analysis();
};


#endif /* LIPIDSPACE_H */
