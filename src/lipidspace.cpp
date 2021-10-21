#include <iostream>
#include <Eigen/Dense>
#include "LipidSpace/matplotlibcpp.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <cppgoslin/cppgoslin.h>
#include <math.h>
#include <algorithm>
#include <sys/stat.h>
#include <Spectra/SymEigsSolver.h>
#include <Eigen/Core>
 
using namespace std;
using namespace Eigen;
namespace plt = matplotlibcpp; 
using namespace Spectra;



enum Linkage {SINGLE, COMLETE};



class Node {
public:
    set<int> indexes;
    Node* left_child;
    Node* right_child;
    double distance;
    
    Node(int index);
    Node(Node* n1, Node* n2, double d);
    ~Node();
    double* plot(int i);
};




class Table {
public:
    string file_name;
    vector<string> species;
    vector<string> classes;
    vector<LipidAdduct*> lipids;
    VectorXd intensities;
    MatrixXd m;
    
    Table(string lipid_list_file) : file_name(lipid_list_file) {}
};


class LipidSpace {
public:
    LipidParser parser;
    map<string, int*> class_matrix;
    vector<LipidAdduct*> all_lipids;
    int cols_for_pca;
    static const vector< vector< vector<int> > > orders;
    static const vector<int> order_len;
    bool keep_sn_position;
    
    
    LipidSpace();
    ~LipidSpace();
    Table* load_list(string lipid_list_file);
    ArrayXd compute_PCA_variances(MatrixXd m);
    void cut_cycle(FattyAcid* fa);
    MatrixXd compute_PCA(MatrixXd m);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
    double compute_hausdorff_distance(Table* l1, Table* l2);
    void plot_PCA(Table* table, string output_folder);
    MatrixXd compute_hausdorff_matrix(vector<Table*>* tables);
    void report_hausdorff_matrix(vector<Table*>* l, MatrixXd distance_matrix, string output_folder);
    Table* compute_global_distance_matrix(vector<Table*>* tables);
    void separate_matrixes(vector<Table*>* lipidomes, Table* global_lipidome);
    void normalize_intensities(vector<Table*>* lipidomes, Table* global_lipidome);
    void load_table(string table_file, vector<Table*>* lipidomes);
    void plot_dendrogram(vector<Table*>* lipidomes, MatrixXd m, string output_folder);
    void store_distance_table(Table* lipidome, string output_folder);
};



Node::Node(int index){
    indexes.insert(index);
    left_child = 0;
    right_child = 0;
    distance = 0;
}

Node::~Node(){
    if (left_child) delete left_child;
    if (right_child) delete right_child;
}
    
Node::Node(Node* n1, Node* n2, double d){
    left_child = n1;
    right_child = n2;
    for (auto i : n1->indexes) indexes.insert(i);
    for (auto i : n2->indexes) indexes.insert(i);
    distance = d;
}

double* Node::plot(int cnt){
    if (left_child == 0) return new double[3]{(double)cnt, 0, (double)cnt + 1};

    double* left_result = left_child->plot(cnt);
    double xl = left_result[0];
    double yl = left_result[1];
    cnt = left_result[2];
    delete []left_result;
    
    double* right_result = right_child->plot(cnt);
    double xr = right_result[0];
    double yr = right_result[1];
    cnt = right_result[2];
    delete []right_result;
    
    double yn = distance;
    
    
    plt::plot((vector<double>){xl, xl}, (vector<double>){yl, yn}, (map<string, string>){{"color", "black"}});
    plt::plot((vector<double>){xr, xr}, (vector<double>){yr, yn}, (map<string, string>){{"color", "black"}});
    plt::plot((vector<double>){xl, xr}, (vector<double>){yn, yn}, (map<string, string>){{"color", "black"}});
    
    return new double[3]{(xl + xr) / 2, yn, (double)cnt};
}


double single_linkage(Node* n1, Node* n2, MatrixXd m, Linkage linkage = COMLETE){
    int mi1 = 0, mi2 = 0;
    double v = 1e9 * (linkage == SINGLE);
    for (auto index1 : n1->indexes){
        for (auto index2 : n2->indexes){
            if ((linkage == COMLETE && v < m(index1, index2)) || (linkage == SINGLE && v > m(index1, index2))){
                v = m(index1, index2);
                mi1 = index1;
                mi2 = index2;
            }
        }
    }
    return v;
}


void LipidSpace::plot_dendrogram(vector<Table*>* lipidomes, MatrixXd m, string output_folder){
    string output_file = output_folder + "/" + "dendrogram.pdf";
    
    cout << "Storing dendrogram at '" << output_file << "'" << endl;
    vector<string> ticks;
    int n = m.rows();
    for (int i = 0; i < n; ++i){
        vector<string>* tokens = split_string(lipidomes->at(i)->file_name, '/', '"');
        ticks.push_back(tokens->back());
        delete tokens;
    }
    
    vector<Node*> nodes;
    for (int i = 0; i < n; ++i) nodes.push_back(new Node(i));
    
    while (nodes.size() > 1){
        double min_val = 1e9;
        int ii = 0, jj = 0;
        for (int i = 0; i < nodes.size() - 1; ++i){
            for (int j = i + 1; j < nodes.size(); ++j){
                double val = single_linkage(nodes.at(i), nodes.at(j), m);
                if (min_val > val){
                    min_val = val;
                    ii = i;
                    jj = j;
                }
            }
        }
        
        Node* node1 = nodes.at(ii);
        Node* node2 = nodes.at(jj);
        nodes.erase(nodes.begin() + jj);
        nodes.erase(nodes.begin() + ii);
        nodes.push_back(new Node(node1, node2, min_val));
    }
    
    
    double* ret = nodes.front()->plot(1);
    delete []ret;
    delete nodes.front();
    
    vector<int> x;
    for (int i = 1; i <= m.rows(); ++i) x.push_back(i);
    plt::xticks(x, ticks, {{"rotation", "45"}, {"horizontalalignment", "right"}, {"fontsize", "4"}});
    plt::yticks((vector<int>){});
    plt::title("Hierarchy of lipidomes");
    
    plt::save(output_file);
    plt::close();
}


LipidSpace::LipidSpace(){
    Eigen::initParallel();
    
    cols_for_pca = 2;
    keep_sn_position = false;
    
    // load precomputed class distance matrix
    ifstream infile("data/classes-matrix.csv");
    if (!infile.good()){
        cerr << "Error: file 'data/classes-matrix.csv' not found." << endl;
        exit(-1);
    }
    string line;
    
    while (getline(infile, line)){
        if (line.length() == 0) continue;
        vector<string>* tokens = goslin::split_string(line, '\t', '"');
        
        int* values = new int[2];
        string key = tokens->at(0) + "/" + tokens->at(1);
        string key2 = tokens->at(1) + "/" + tokens->at(0);
        values[0] = abs(atoi(tokens->at(2).c_str()));
        values[1] = abs(atoi(tokens->at(3).c_str()));
        class_matrix.insert({key, values});
        class_matrix.insert({key2, values});
        delete tokens;
    }
}


const vector<int> LipidSpace::order_len{1, 1, 2, 6, 24};
    
const vector< vector< vector<int> > > LipidSpace::orders{
    {
        {}
    },
    {
        {0}
    },
    {
        {0, 1},
        {1, 0}
    },
    {
        {0, 1, 2},
        {0, 2, 1},
        {1, 0, 2},
        {1, 2, 0},
        {2, 0, 1},
        {2, 1, 0}
    },
    {
        {0, 1, 2, 3},
        {0, 2, 1, 3},
        {1, 0, 2, 3},
        {1, 2, 0, 3},
        {2, 0, 1, 3},
        {2, 1, 0, 3},
        {0, 1, 3, 2},
        {0, 2, 3, 1},
        {1, 0, 3, 2},
        {1, 2, 3, 0},
        {2, 0, 3, 1},
        {2, 1, 3, 0},
        {0, 3, 1, 2},
        {0, 3, 2, 1},
        {1, 3, 0, 2},
        {1, 3, 2, 0},
        {2, 3, 0, 1},
        {2, 3, 1, 0},
        {3, 0, 1, 2},
        {3, 0, 2, 1},
        {3, 1, 0, 2},
        {3, 1, 2, 0},
        {3, 2, 0, 1},
        {3, 2, 1, 0}
    }
};



LipidSpace::~LipidSpace(){
    for (auto lipid : all_lipids) delete lipid;
}






void LipidSpace::cut_cycle(FattyAcid* fa){
    if (uncontains_p(fa->functional_groups, "cy")) return;

    if (((Cycle*)fa->functional_groups->at("cy").at(0))->start > -1){
        int start = ((Cycle*)fa->functional_groups->at("cy").at(0))->start;
        
        // shrink numbers of carbon
        fa->num_carbon = start - 1;
        
        // cut all double bonds
        if (fa->double_bonds->num_double_bonds > 0 && fa->double_bonds->double_bond_positions.size() == 0){
            fa->double_bonds->num_double_bonds = max(fa->double_bonds->num_double_bonds, int((start - 1) / 2));
        }
        vector<int> db;
        for (auto kv : fa->double_bonds->double_bond_positions) db.push_back(kv.first);
        for (auto key : db){
            if (key >= start) fa->double_bonds->double_bond_positions.erase(key);
        }
        if (fa->double_bonds->double_bond_positions.size() > 0) {
            fa->double_bonds->num_double_bonds = fa->double_bonds->double_bond_positions.size();
        }
        
                    
        // cut all functional groups
        for (FunctionalGroup* func_group : fa->functional_groups->at("cy")){
            Cycle* c = (Cycle*)func_group;
            delete c;
        }
        fa->functional_groups->erase("cy");
        
        vector<string> del_fgs;
        for (auto kv : *(fa->functional_groups)){
            string fg = kv.first;
            set<FunctionalGroup*> keep_fg;
            for (auto func_group : kv.second){
                if (0 <= func_group->position && func_group->position < start){
                    keep_fg.insert(func_group);
                }
            }
            
            if (keep_fg.size() > 0){
                vector<FunctionalGroup*> all_fg;
                for (auto func_group : fa->functional_groups->at(fg)) all_fg.push_back(func_group);
                fa->functional_groups->at(fg).clear();
                for (auto func_group : all_fg){
                    if (contains(keep_fg, func_group)){
                        fa->functional_groups->at(fg).push_back(func_group);
                    }
                    else {
                        delete func_group;
                    }
                }
            }
            else {
                del_fgs.push_back(fg);
            }
        }
        for (auto fg : del_fgs){
            if (fa->functional_groups->at(fg).size() > 0){
                for (auto func_group : fa->functional_groups->at(fg)) delete func_group;
            }
            fa->functional_groups->erase(fg);
        }
    }
    else {
        fa->num_carbon = 2;
        fa->double_bonds->double_bond_positions.clear();
        
        for (auto kv : *(fa->functional_groups)){
            for (auto func_group : kv.second) delete func_group;
        }
        fa->functional_groups->clear();
    }
}








void LipidSpace::fatty_acyl_similarity(FattyAcid* fa1, FattyAcid* fa2, int& union_num, int& inter_num){
    
    set<LipidFaBondType> lcbs = {LCB_REGULAR, LCB_EXCEPTION};
    
    set<LipidFaBondType> bond_types = {fa1->lipid_FA_bond_type, fa2->lipid_FA_bond_type};
    
    
    if (bond_types.size() == 1){ // both equal
        if (contains(bond_types, ESTER)){
            union_num += 2; // one oxygen plus one double bond
            inter_num += 2;
        }
        if (contains(bond_types, ETHER_PLASMENYL)){
            union_num += 1; // one double bond
            inter_num += 1;
        }
    }
        
    else if (contains(bond_types, ESTER) && contains(bond_types, ETHER_PLASMENYL)){
        union_num += 3;
    }
        
    else if (contains(bond_types, ESTER) && contains(bond_types, ETHER_PLASMANYL)){
        union_num += 2;
    }
        
    else if (contains(bond_types, ETHER_PLASMENYL) && contains(bond_types, ETHER_PLASMANYL)){
        union_num += 1;
    }
        
    else if (contains(bond_types, LCB_REGULAR) && contains(bond_types, ESTER)){
        union_num += 2;
    }
        
    else if (contains(bond_types, LCB_EXCEPTION) && contains(bond_types, ESTER)){
        union_num += 2;
    }
        
    else if (contains(bond_types, LCB_REGULAR) && contains(bond_types, ETHER_PLASMANYL)){
    }
        
    else if (contains(bond_types, LCB_EXCEPTION) && contains(bond_types, ETHER_PLASMANYL)){
    }
        
    else if (contains(bond_types, LCB_REGULAR) && contains(bond_types, ETHER_PLASMENYL)){
        union_num += 1;
    }
        
    else if (contains(bond_types, LCB_EXCEPTION) && contains(bond_types, ETHER_PLASMENYL)){
        union_num += 1;
    }
        
    else if (contains(bond_types, LCB_EXCEPTION) && contains(bond_types, LCB_REGULAR)){
    }
        
    else { 
        throw LipidException("Cannot compute similarity between chains");
    }
    
    // compare lengths
    int m1 = contains(lcbs, fa1->lipid_FA_bond_type) * 2;
    int m2 = contains(lcbs, fa2->lipid_FA_bond_type) * 2;
    
    inter_num += min(fa1->num_carbon - m1, fa2->num_carbon - m1);
    union_num += max(fa1->num_carbon - m2, fa2->num_carbon - m2);
    
    
    // compare double bonds
    int db1 = 0, db2 = 0;
    if (fa1->double_bonds->double_bond_positions.size() > 0 && fa2->double_bonds->double_bond_positions.size() > 0){
        int common = 0;
        for (auto kv : fa1->double_bonds->double_bond_positions){
            common += contains(fa2->double_bonds->double_bond_positions, kv.first) && fa2->double_bonds->double_bond_positions.at(kv.first) == kv.second;
        }
        
        inter_num += common;
        union_num += fa1->double_bonds->get_num() + fa2->double_bonds->get_num() - common;
        db1 = fa1->double_bonds->get_num();
        db2 = fa2->double_bonds->get_num();
    }
    else {
        db1 = fa1->double_bonds->get_num();
        db2 = fa2->double_bonds->get_num();
            
        inter_num += min(db1, db2);
        union_num += max(db1, db2);
    }
    
    // add all single bonds
    inter_num += min(fa1->num_carbon - m1 - db1, fa2->num_carbon - m1 - db2);
    union_num += max(fa1->num_carbon - m2 - db1, fa2->num_carbon - m2 - db2);
    
    // compare functional groups
    for (auto kv : *(fa1->functional_groups)){
        string key = kv.first;
        if (key == "[X]") continue;
            
        FunctionalGroup* func_group = KnownFunctionalGroups::get_functional_group(key);
        int elements = func_group->get_double_bonds();
        ElementTable* e = func_group->get_elements();
        for (auto ekv : *e){
            if (ekv.first != ELEMENT_H) elements += ekv.second;
        }
        delete e;
        delete func_group;
        
        
        if (contains_p(fa2->functional_groups, key)){ // functional group occurs in both fatty acids
            
            // both func groups contain position information
            if (fa1->functional_groups->at(key).at(0)->position > -1 && fa2->functional_groups->at(key).at(0)->position > -1){
                vector<int> keys_fa1;
                for (auto f : fa1->functional_groups->at(key)) keys_fa1.push_back(f->position);
                vector<int> keys_fa2;
                for (auto f : fa2->functional_groups->at(key)) keys_fa2.push_back(f->position);
                
                vector<int> unions;
                set_union(keys_fa1.begin(), keys_fa1.end(), keys_fa2.begin(), keys_fa2.end(), back_inserter(unions));
                vector<int> inters;
                set_intersection(keys_fa1.begin(), keys_fa1.end(), keys_fa2.begin(), keys_fa2.end(), back_inserter(unions));
                
                inter_num += inters.size();
                union_num += unions.size();
            }
        }
        else {
            int num = 0;
            if (fa1->functional_groups->at(key).at(0)->position > -1){
                num = fa1->functional_groups->at(key).size();
            }
            else {
                for (auto f : fa1->functional_groups->at(key)) num += f->count;
            }
            union_num += elements * num;
        }
    }
    
    vector<string> keys_fa1;
    for (auto kv : *(fa1->functional_groups)) keys_fa1.push_back(kv.first);
    vector<string> keys_fa2;
    for (auto kv : *(fa2->functional_groups)) keys_fa1.push_back(kv.first);
    vector<string> diff;
    set_difference(keys_fa2.begin(), keys_fa2.end(), keys_fa1.begin(), keys_fa1.end(), inserter(diff, diff.begin()));
    
    for (auto key : diff){
        if (key == "[X]") continue;
    
        FunctionalGroup* func_group = KnownFunctionalGroups::get_functional_group(key);
        int elements = func_group->get_double_bonds();
        ElementTable* e = func_group->get_elements();
        for (auto ekv : *e){
            if (ekv.first != ELEMENT_H) elements += ekv.second;
        }
        delete e;
        delete func_group;
        
        int num = 0;
        if (fa2->functional_groups->at(key).at(0)->position > -1){
            num = fa2->functional_groups->at(key).size();
        }
        else {
            for (auto f : fa2->functional_groups->at(key)) num += f->count;
        }
        union_num += elements * num;
    }
}



    
    
    
    

void LipidSpace::lipid_similarity(LipidAdduct* lipid1, LipidAdduct* lipid2, int& union_num, int& inter_num){
    string key = lipid1->get_extended_class() + "/" + lipid2->get_extended_class();
    if (!contains(class_matrix, key)){
        cerr << "Error: key '" << key << "' not in precomputed class matrix" << endl;
        exit(-1);
    }
    union_num = class_matrix.at(key)[0];
    inter_num = class_matrix.at(key)[1];
    
    int max_u = 0, max_i = 0;
    double max_q = 0;
    
    vector<FattyAcid*>* orig_fa_list_1 = &lipid1->lipid->fa_list;
    vector<FattyAcid*>* orig_fa_list_2 = &lipid2->lipid->fa_list;
    
    if (orig_fa_list_1->size() < orig_fa_list_2->size()){
        vector<FattyAcid*>* tmp = orig_fa_list_1;
        orig_fa_list_1 = orig_fa_list_2;
        orig_fa_list_2 = tmp;
    }
    int len_fa2 = orig_fa_list_2->size();
    
    for(int ol = 0; ol < order_len.at(len_fa2); ++ol){
        int uu = 0, ii = 0;
        
        vector<FattyAcid*>* fa_list_1 = orig_fa_list_1;
        vector<FattyAcid*>* fa_list_2 = new vector<FattyAcid*>();
        vector<FattyAcid*>* for_del = fa_list_2;
        
        if (len_fa2 > 0){
            for (int index : orders.at(len_fa2).at(ol)) fa_list_2->push_back(orig_fa_list_2->at(index));
        }
        
    
        int l = min(fa_list_1->size(), fa_list_2->size());
        for (int i = 0; i < l; ++i){
            fatty_acyl_similarity(fa_list_1->at(i), fa_list_2->at(i), uu, ii);
        }
        
        
        
            
        if (fa_list_1->size() > fa_list_2->size()){
            for (int i = fa_list_2->size(); i < fa_list_1->size(); ++i){
                FattyAcid* fa = fa_list_1->at(i);
                uu += fa->get_double_bonds();
                ElementTable* e = fa->get_elements();
                for (auto kv : *e){
                    if (kv.first != ELEMENT_H) uu += kv.second;
                }
                delete e;
            }
        }
        
        double q = (double)ii / (double)uu;
        if (max_q < q){
            max_q = q;
            max_u = uu;
            max_i = ii;
        }
        
        delete for_del;
        if (keep_sn_position) break;
    }
    
    union_num += max_u;
    inter_num += max_i;
}








Table* LipidSpace::load_list(string lipid_list_file){
    cout << "reading list '" << lipid_list_file << "'" << endl;
    
    // load and parse lipids
    ifstream infile(lipid_list_file);
    if (!infile.good()){
        cerr << "Error: file '" << lipid_list_file << "' not found." << endl;
        exit(-1);
    }
    string line;
    
    
    Table* lipidome = new Table(lipid_list_file);
    vector<double> intensities;
    vector<string> lipids;
    
    int pos_all = all_lipids.size();
    while (getline(infile, line)){
        if (line.length() == 0) continue;
        
        vector<string>* tokens = goslin::split_string(line, '\n', '"');
        line = strip(tokens->at(0), '"');
        delete tokens;
        
        
        if (line.find('\t') != string::npos){
            tokens = goslin::split_string(line, '\t', '"');
            line = strip(tokens->at(0), '"');
            intensities.push_back(atof(tokens->at(1).c_str()));
            delete tokens;
        }
        else {
            intensities.push_back(1);
        }
        lipids.push_back(line);
        lipidome->lipids.push_back(0);
        all_lipids.push_back(0);
        lipidome->species.push_back("");
        lipidome->classes.push_back("");
    }
    
    
    
    #pragma omp parallel for
    for (int i = 0; i < lipids.size(); ++i) { 
        string line = lipids.at(i);
        try {
            LipidAdduct* l = parser.parse_parallel(line);
            // deleting adduct since not necessary
            if (l->adduct != 0){
                delete l->adduct;
                l->adduct = 0;
            }
            
            all_lipids.at(pos_all + i) = l;
            lipidome->lipids.at(i) = l;
            lipidome->species.at(i) = l->get_lipid_string();
            lipidome->classes.at(i) = l->get_lipid_string(CLASS);
            for (auto fa : l->lipid->fa_list) cut_cycle(fa);
        }
        catch (exception &e) {
            cerr << "Error: lipid '" << line << "' cannot be parsed" << endl;
            exit(-1);
        }
    }
    
    VectorXd intens(intensities.size());
    for (int i = 0; i < intensities.size(); ++i) intens(i) = intensities.at(i);
    lipidome->intensities = intens;
    
    
    return lipidome;
}









ArrayXd LipidSpace::compute_PCA_variances(MatrixXd m){
    ArrayXd var = m.array().square().colwise().sum();
    return var / var.sum();
}








MatrixXd LipidSpace::compute_PCA(MatrixXd m){
    cout << "Running principal component analysis" << endl;
    // scale and transform the matrix
    int r = m.rows();
    m = m.rowwise() - (m.rowwise().mean()).transpose();
    VectorXd norm = (r / m.array().square().colwise().sum()).sqrt();
    for (int i = 0; i < r; ++i) m.block(0, i, r, 1) *= norm[i];

    int n = m.rows();    
    
    // calculate the covariance matrix
    MatrixXd centered = m.rowwise() - m.colwise().mean();
    MatrixXd cov = (centered.adjoint() * centered) / double(n - 1);

    

    // Construct matrix operation object using the wrapper class DenseSymMatProd
    DenseSymMatProd<double> op(cov);
 
    // Construct eigen solver object, requesting the largest three eigenvalues
    SymEigsSolver<DenseSymMatProd<double>> eigs(op, min(n - 1, cols_for_pca), min(n, cols_for_pca * 3));
 
    // Initialize and compute
    eigs.init();
    int nconv = eigs.compute(SortRule::LargestAlge);
 
    // Retrieve results
    if (eigs.info() != CompInfo::Successful){
        cerr << "Error: could not perform principal component analysis." << endl;
        exit(-1);
    }
    MatrixXd evecs = eigs.eigenvectors();
    
    return (evecs.transpose() * m.transpose()).transpose();
}








double LipidSpace::compute_hausdorff_distance(Table* l1, Table* l2){
    MatrixXd m1 = l1->m;
    MatrixXd m2 = l2->m;
    
    // add intensities to hausdorff matrix
    
    m1.conservativeResize(m1.rows(), cols_for_pca + 1);
    m1.col(cols_for_pca) = l1->intensities;
    
    m2.conservativeResize(m2.rows(), cols_for_pca + 1);
    m2.col(cols_for_pca) = l2->intensities;
    
    
    double hausdorff = 0;
    for (int i = 0; i < m1.rows(); ++i){
        VectorXd b = m1.row(i);
        MatrixXd d = m2.rowwise() - b.transpose();
        d = d.array().square();
        MatrixXd e = d.rowwise().sum();
        hausdorff = max(hausdorff, e.minCoeff());
    }
    for (int i = 0; i < m2.rows(); ++i){
        VectorXd b = m2.row(i);
        MatrixXd d = m1.rowwise() - b.transpose();
        d = d.array().square();
        MatrixXd e = d.rowwise().sum();
        hausdorff = max(hausdorff, e.minCoeff());
    }
    
    return sqrt(hausdorff);
}









MatrixXd LipidSpace::compute_hausdorff_matrix(vector<Table*>* lipidomes){
    int n = lipidomes->size();
    MatrixXd distance_matrix = MatrixXd::Zero(n, n);
    for (int i = 0; i < n - 1; ++i){
        for (int j = i + 1; j < n; ++j){
            distance_matrix(i, j) = compute_hausdorff_distance(lipidomes->at(i), lipidomes->at(j));
            distance_matrix(j, i) = distance_matrix(i, j);
        }
    }
    return distance_matrix;
}








void LipidSpace::plot_PCA(Table* table, string output_folder){
    string output_file_name = table->file_name;
    vector<string>* tokens = split_string(output_file_name, '/');
    output_file_name = tokens->back();
    delete tokens;
    
    if (output_file_name.find(".") != string::npos){
        tokens = split_string(output_file_name, '.');
        stringstream output_stream;
        for (int i = 0; i < tokens->size() - 1; ++i) output_stream << tokens->at(i);
        output_stream << ".pdf";
        output_file_name = output_stream.str();
        delete tokens;
    }
    else {
        output_file_name += ".pdf";
    }
    output_file_name = output_folder + "/" + output_file_name;
    
    
    map<string, vector<int>> indexes;
    for (int i = 0; i < table->classes.size(); ++i){
        string lipid_class = table->classes.at(i);
        if (uncontains(indexes, lipid_class)) indexes.insert({lipid_class, vector<int>()});
        indexes.at(lipid_class).push_back(i);
    }
    
    //ArrayXd variances = compute_PCA_variances(table->m);
    for (auto kv : indexes){
        vector<double> x;
        vector<double> y;
        for (auto i : kv.second){
            x.push_back(table->m(i, 0));
            y.push_back(table->m(i, 1));
        }
        stringstream label;
        label << kv.first << " (" << x.size() << ")";
        plt::scatter(x, y, 3, {{"label", label.str()}});
        plt::annotate(kv.first, 0, 0);
    }
    
    stringstream xlabel;
    xlabel.precision(1);
    //xlabel << fixed << "Principal component 1 (" << (variances(0) * 100) << " %)";
    xlabel << fixed << "Principal component 1";
    
    stringstream ylabel;
    ylabel.precision(1);
    //ylabel << fixed << "Principal component 2 (" << (variances(1) * 100) << " %)";
    ylabel << fixed << "Principal component 2";
    
    plt::xlabel(xlabel.str());
    plt::ylabel(ylabel.str());
    
    cout << "storing '" << output_file_name << "'" << endl;
    plt::xticks((vector<int>){});
    plt::yticks((vector<int>){});
    plt::legend({{"fontsize", "5"}});
    plt::save(output_file_name);
    plt::close();
}








void LipidSpace::report_hausdorff_matrix(vector<Table*>* lipidomes, MatrixXd distance_matrix, string output_folder){
    ofstream off(output_folder + "/hausdorff_distances.csv");
    vector<string> striped_names;
    int n = distance_matrix.rows();
    off << "ID";
    for (int i = 0; i < n; ++i){
        vector<string>* tokens = split_string(lipidomes->at(i)->file_name, '/', '"');
        off << "\t" << tokens->back();
        striped_names.push_back(tokens->back());
        delete tokens;
    } off << endl;
    
    for (int i = 0; i < n; ++i){
        off << striped_names.at(i);
        for (int j = 0; j < n; ++j){
            off << "\t" << distance_matrix(i, j);
        } off << endl;
    }
}










Table* LipidSpace::compute_global_distance_matrix(vector<Table*>* lipidomes){
    set<string> registered_lipids;
    Table* global_lipidome = new Table("global_lipidome");
    
    for (auto lipidome : *lipidomes){
        for (int i = 0; i < lipidome->species.size(); ++i){
            string lipid_species = lipidome->species.at(i);
            if (contains(registered_lipids, lipid_species)) continue;
            registered_lipids.insert(lipid_species);
            
            global_lipidome->species.push_back(lipid_species);
            global_lipidome->classes.push_back(lipidome->classes.at(i));
            global_lipidome->lipids.push_back(lipidome->lipids.at(i));
        }
    }
    
    
    // compute distances
    int n = global_lipidome->lipids.size();
    cout << "Computing pairwise distance matrix for " << n << " lipids" << endl;
    MatrixXd distance_matrix = MatrixXd::Zero(n, n);
    
    
    #pragma omp parallel for
    for (int ii = 0; ii < n * n; ++ii){
        int i = ii / n;
        int j = ii % n;
        
        if (i < j){
            int union_num, inter_num;
            lipid_similarity(global_lipidome->lipids.at(i), global_lipidome->lipids.at(j), union_num, inter_num);
            //double distance = (double)union_num / (double)inter_num - 1.;
            double distance = 1 - (double)inter_num / (double)union_num;
            distance_matrix(i, j) = distance;
            distance_matrix(j, i) = distance;
        }
    }
    global_lipidome->m = distance_matrix;
    
    return global_lipidome;
}









void LipidSpace::separate_matrixes(vector<Table*>* lipidomes, Table* global_lipidome){
    map<string, int> lipid_indexes;
    vector<int> all = {0, 1};
    for (int i = 0; i < global_lipidome->species.size(); ++i){
        lipid_indexes.insert({global_lipidome->species.at(i), i});
    }
    
    
    for (auto lipidome : *lipidomes){
        vector<int> indexes;
        for (auto lipid_species : lipidome->species) indexes.push_back(lipid_indexes.at(lipid_species));
        lipidome->m = global_lipidome->m(indexes, all);
    }
}








void LipidSpace::normalize_intensities(vector<Table*>* lipidomes, Table* global_lipidome){
    MatrixXd m = global_lipidome->m.block(0, 0, (int)global_lipidome->m.rows(), 1);
    double global_stdev = sqrt((m.array().square()).sum() / (double)global_lipidome->m.rows());
    
    for (auto lipidome : *lipidomes){
        VectorXd v = lipidome->intensities.array() - ((double)lipidome->intensities.sum() / (double)lipidome->intensities.rows());
        double stdev = sqrt((v.array().square()).sum() / (double)v.rows());
        if (stdev > 1e-16) lipidome->intensities = lipidome->intensities.array() / stdev * global_stdev;
    }
}







void LipidSpace::load_table(string table_file, vector<Table*>* lipidomes){
    cout << "reading table '" << table_file << "'" << endl;
    
    // load and parse lipids
    ifstream infile(table_file);
    if (!infile.good()){
        cerr << "Error: file '" << table_file << "' not found." << endl;
        exit(-1);
    }
    string line;
    

    set<string> NA_VALUES = {"NA", "nan", "N/A", "O", "", "n/a", "NaN"};
    vector<vector<int>> intensities;
    
    
    int line_cnt = 0;
    int num_cols = 0;
    while (getline(infile, line)){
        if (line_cnt++ == 0){
            vector<string>* tokens = split_string(line, ',', '"', true);
            num_cols = tokens->size();
            for (int i = 1; i < tokens->size(); ++i){
                lipidomes->push_back(new Table(tokens->at(i)));
                intensities.push_back(vector<int>());
            }
            delete tokens;
            continue;
        }
        if (line.length() == 0) continue;
        

                                                                          
        vector<string>* tokens = split_string(line, ',', '"', true);
        if (tokens->size() != num_cols) {
            cerr << "Error in line '" << line_cnt << "' number of cells does not match with number of column labels" << endl;
            exit(-1);
        }
        
        LipidAdduct* l = 0;
        try {
            l = parser.parse(tokens->at(0));
        }
        catch (exception &e) {
            cerr << "Error: lipid '" << tokens->at(0) << "' cannot be parsed" << endl;
            exit(-1);
        }
        
        if (l == 0) {
            cerr << "Error: lipid '" << tokens->at(0) << "' cannot be parsed" << endl;
            exit(-1);
        }
            
        // deleting adduct since not necessary
        if (l->adduct != 0){
            delete l->adduct;
            l->adduct = 0;
        }
        all_lipids.push_back(l);
        for (int i = 1; i < tokens->size(); ++i){
            string val = tokens->at(i);
            if (!contains(NA_VALUES, val)){
                Table* lipidome = lipidomes->at(i - 1);
                lipidome->lipids.push_back(l);
                lipidome->species.push_back(l->get_lipid_string());
                lipidome->classes.push_back(l->get_lipid_string(CLASS));
                intensities.at(i - 1).push_back(atof(val.c_str()));
            }
        }
        for (auto fa : l->lipid->fa_list) cut_cycle(fa);
        delete tokens;
    }
        
    for (int i = 0; i < lipidomes->size(); ++i){
        vector<int> &vi = intensities.at(i);
        VectorXd v(vi.size());
        for (int j = 0; j < vi.size(); ++j) v(j) = vi.at(j);
        lipidomes->at(i)->intensities = v;
    }
}




void LipidSpace::store_distance_table(Table* lipidome, string output_folder){
    string output_file = output_folder + "/distance_matrix.csv";
    
    ofstream table_stream(output_file);
    table_stream << "ID";
    for (auto lipid : lipidome->species) table_stream << "\t" << lipid;
    table_stream << endl;
    
    for (int i = 0; i < lipidome->species.size(); ++i){
        table_stream << lipidome->species.at(i);
        for (int j = 0; j < lipidome->species.size(); ++j){
            table_stream << "\t" << lipidome->m(i, j);
        }
        table_stream << endl;
    }
    
}





void print_help(){
    cerr << "usage: " << endl;
    cerr << "  > ./lipidspace output_folder table lipid_table[csv]" << endl;
    cerr << "  > ./lipidspace output_folder lists lipid_list[csv], ..." << endl;
    cerr << "modes either 'table' or 'lisds'." << endl;
}





int main(int argc, char** argv) {
    
    
    
    if (argc < 4) {
        print_help();
        exit(-1);
    }
        
    // getting input
    string output_folder = argv[1];
    string mode = argv[2];
    
    struct stat buffer;
    if (stat (output_folder.c_str(), &buffer) != 0){
        cerr << "Error: output folder '" << output_folder << "' does not exist." << endl;
        exit(-1);
    }
    
    
    if (mode != "table" && mode != "lists") {
        print_help();
        exit(-1);
    }
    
    LipidSpace lipid_space;
    vector<Table*> lipidomes;
    
    
    
    lipid_space.keep_sn_position = false;
    bool plot_pca = true; 
    bool plot_pca_lipidomes = false;
    bool storing_distance_table = true;
    
    
    // loadig each lipidome
    if (mode == "lists"){
        for (int i = 3; i < argc; ++i) lipidomes.push_back(lipid_space.load_list(argv[i]));
    }
    // loading lipid matrix
    else {
        lipid_space.load_table(argv[3], &lipidomes);
    }
    
    // compute PCA matrixes for the complete lipidome
    Table* global_lipidome = lipid_space.compute_global_distance_matrix(&lipidomes);
    
    
    // storing the distance matrix
    if (storing_distance_table){
        lipid_space.store_distance_table(global_lipidome, output_folder);
    }
    
    
    // perform the principal component analysis
    global_lipidome->m = lipid_space.compute_PCA(global_lipidome->m);
    
        
    // cutting the global PCA matrix back to a matrix for each lipidome
    lipid_space.separate_matrixes(&lipidomes, global_lipidome);

    
    // plotting all lipidome PCAs
    if (plot_pca){
        lipid_space.plot_PCA(global_lipidome, output_folder);
    }
    if (plot_pca_lipidomes){
        for (auto table : lipidomes){
            lipid_space.plot_PCA(table, output_folder);
        }
    }
    
    
    // normalize and incorporate intensities
    lipid_space.normalize_intensities(&lipidomes, global_lipidome);
    
    
    
    if (lipidomes.size() > 1){
        
        // computing the hausdorff distance matrix for all lipidomes
        MatrixXd distance_matrix = lipid_space.compute_hausdorff_matrix(&lipidomes);
        
        
        // storing hausdorff distance matrix into file
        lipid_space.report_hausdorff_matrix(&lipidomes, distance_matrix, output_folder);
        
        // ploting the dendrogram
        lipid_space.plot_dendrogram(&lipidomes, distance_matrix, output_folder);
    }
    
    
    // free all allocated data
    for (auto table : lipidomes) delete table;
    delete global_lipidome;
    
    
    if (plot_pca || plot_pca_lipidomes || lipidomes.size() > 1){
        Py_Finalize();
    }
    
    return 0;
}
