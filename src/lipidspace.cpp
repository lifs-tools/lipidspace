#include <iostream>
#include "LipidSpace/matplotlibcpp.h"
#include <vector>
#include <string>
#include "cppgoslin/cppgoslin.h"
#include <math.h>
#include <algorithm>
#include <sys/stat.h>
#include <chrono>
#include <LipidSpace/Matrix.h>

 
using namespace std;
namespace plt = matplotlibcpp; 


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
    Mat m;
    
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
    
    
    LipidSpace();
    ~LipidSpace();
    Table* load_list(string lipid_list_file);
    void compute_PCA_variances(Mat &m, Array &a);
    void cut_cycle(FattyAcid* fa);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
    double compute_hausdorff_distance(Table* l1, Table* l2);
    void plot_PCA(Table* table, string output_folder);
    void compute_hausdorff_matrix(vector<Table*>* tables, Mat &m);
    void report_hausdorff_matrix(vector<Table*>* l, Mat &distance_matrix, string output_folder);
    Table* compute_global_distance_matrix(vector<Table*>* tables);
    void separate_matrixes(vector<Table*>* lipidomes, Table* global_lipidome);
    void normalize_intensities(vector<Table*>* lipidomes, Table* global_lipidome);
    void load_table(string table_file, vector<Table*>* lipidomes);
    void plot_dendrogram(vector<Table*>* lipidomes, Mat &m, string output_folder);
    void store_distance_table(Table* lipidome, string output_folder);
};



Node::Node(int index, string _name){
    indexes.insert(index);
    name = _name;
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

double* Node::plot(int cnt, vector<string>* sorted_ticks){
    if (left_child == 0){
        sorted_ticks->push_back(name);
        return new double[3]{(double)cnt, 0, (double)cnt + 1};
    }

    double* left_result = left_child->plot(cnt, sorted_ticks);
    double xl = left_result[0];
    double yl = left_result[1];
    cnt = left_result[2];
    delete []left_result;
    
    double* right_result = right_child->plot(cnt, sorted_ticks);
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








double linkage(Node* n1, Node* n2, Mat &m, Linkage linkage = COMPLETE){
    int mi1 = 0, mi2 = 0;
    double v = 1e9 * (linkage == SINGLE);
    for (auto index1 : n1->indexes){
        for (auto index2 : n2->indexes){
            if ((linkage == COMPLETE && v < m(index1, index2)) || (linkage == SINGLE && v > m(index1, index2))){
                v = m(index1, index2);
                mi1 = index1;
                mi2 = index2;
            }
        }
    }
    return v;
}









void LipidSpace::plot_dendrogram(vector<Table*>* lipidomes, Mat &m, string output_folder){
    string output_file = output_folder + "/" + "dendrogram.pdf";
    
    cout << "Storing dendrogram at '" << output_file << "'" << endl;
    vector<string> ticks;
    int n = m.rows;
    for (int i = 0; i < n; ++i){
        vector<string>* tokens = split_string(lipidomes->at(i)->file_name, '/', '"');
        ticks.push_back(tokens->back());
        delete tokens;
    }
    
    vector<Node*> nodes;
    for (int i = 0; i < n; ++i) nodes.push_back(new Node(i, ticks.at(i)));
    
    while (nodes.size() > 1){
        double min_val = 1e9;
        int ii = 0, jj = 0;
        for (int i = 0; i < nodes.size() - 1; ++i){
            for (int j = i + 1; j < nodes.size(); ++j){
                double val = linkage(nodes.at(i), nodes.at(j), m, COMPLETE);
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
    
    vector<string> sorted_ticks;
    double* ret = nodes.front()->plot(1, &sorted_ticks);
    delete []ret;
    delete nodes.front();
    
    vector<int> x;
    for (int i = 1; i <= m.rows; ++i) x.push_back(i);
    plt::xticks(x, sorted_ticks, {{"rotation", "45"}, {"horizontalalignment", "right"}, {"fontsize", "4"}});
    plt::yticks((vector<int>){});
    plt::title("Hierarchy of lipidomes");
    
    plt::save(output_file);
    plt::close();
}








LipidSpace::LipidSpace(){
    cols_for_pca = 7;
    keep_sn_position = false;
    ignore_unknown_lipids = false;
    unboundend_distance = false;
    without_quant = false;
    
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



const vector< vector< vector< vector<int> > > > LipidSpace::orders{
    {{{}}},
    {{{}}},
    {
        {{0, 0}, {1, 1}}, 
        {{0, 1}, {1, 0}}
    },
    {
        {{0, 0}, {1, 1}, {2, 2}},
        {{0, 1}, {1, 0}, {2, 2}},
        {{0, 0}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 2}, {2, 0}},
        {{0, 2}, {1, 0}, {2, 1}},
        {{0, 2}, {1, 1}, {2, 0}}
    },
    {
        {{0, 0}, {1, 1}, {2, 2}, {3, 3}},
        {{0, 0}, {1, 1}, {2, 3}, {2, 3}},
        {{0, 0}, {1, 2}, {2, 1}, {3, 3}},
        {{0, 0}, {1, 2}, {2, 3}, {3, 1}},
        {{0, 0}, {1, 3}, {2, 1}, {3, 2}},
        {{0, 0}, {1, 3}, {2, 2}, {3, 1}},
        
        {{0, 1}, {1, 0}, {2, 2}, {3, 3}},
        {{0, 1}, {1, 0}, {2, 3}, {3, 2}},
        {{0, 1}, {1, 2}, {2, 0}, {3, 3}},
        {{0, 1}, {1, 2}, {2, 3}, {3, 0}},
        {{0, 1}, {1, 3}, {2, 0}, {3, 2}},
        {{0, 1}, {1, 3}, {2, 2}, {3, 0}},
        
        {{0, 2}, {1, 0}, {2, 1}, {3, 3}},
        {{0, 2}, {1, 0}, {2, 3}, {3, 1}},
        {{0, 2}, {1, 1}, {2, 0}, {3, 3}},
        {{0, 2}, {1, 1}, {2, 3}, {3, 0}},
        {{0, 2}, {1, 3}, {2, 0}, {3, 1}},
        {{0, 2}, {1, 3}, {2, 1}, {3, 0}},
        
        {{0, 3}, {1, 0}, {2, 1}, {3, 2}},
        {{0, 3}, {1, 0}, {2, 2}, {3, 1}},
        {{0, 3}, {1, 1}, {2, 0}, {3, 2}},
        {{0, 3}, {1, 1}, {2, 2}, {3, 0}},
        {{0, 3}, {1, 2}, {2, 0}, {3, 1}},
        {{0, 3}, {1, 2}, {2, 1}, {3, 0}}
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
    
    else if (contains(bond_types, ESTER)){
        if (contains(bond_types, ETHER_PLASMENYL)){
            union_num += 3;
        }
        
        else if (contains(bond_types, ETHER_PLASMANYL)){
            union_num += 2;
        }
        
        else if (contains(bond_types, LCB_REGULAR) || contains(bond_types, LCB_EXCEPTION)){
            union_num += 2;
        }
    }
    
    else if (contains(bond_types, LCB_EXCEPTION) || contains(bond_types, LCB_REGULAR)){
        if (contains(bond_types, ETHER_PLASMENYL)){
            union_num += 1;
        }
    }
        
    else if (contains(bond_types, ETHER_PLASMENYL) && contains(bond_types, ETHER_PLASMANYL)){
        union_num += 1;
    }
        
    else { 
        throw LipidException("Cannot compute similarity between chains");
    }

    
    // compare lengths
    int m1 = contains(lcbs, fa1->lipid_FA_bond_type) * 2;
    int m2 = contains(lcbs, fa2->lipid_FA_bond_type) * 2;
    
    inter_num += mmin(fa1->num_carbon - m1, fa2->num_carbon - m1);
    union_num += mmax(fa1->num_carbon - m2, fa2->num_carbon - m2);
    
    
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
            
        inter_num += mmin(db1, db2);
        union_num += mmax(db1, db2);
    }
    
    // add all single bonds
    inter_num += mmin(fa1->num_carbon - m1 - db1, fa2->num_carbon - m1 - db2);
    union_num += mmax(fa1->num_carbon - m2 - db1, fa2->num_carbon - m2 - db2);
    
    if (fa1->functional_groups->size() == 0 && fa2->functional_groups->size() == 0) return;
    
    
    
    // compare functional groups
    for (auto kv : *(fa1->functional_groups)){
        string key = kv.first;
        if (key == "[X]") continue;
            
        // functional group occurs in both fatty acids
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
        // functional group occurs only in first fatty acid
        else {
            FunctionalGroup* func_group = KnownFunctionalGroups::get_functional_group(key);
            if (func_group){
                int elements = func_group->get_double_bonds();
                ElementTable* e = func_group->get_elements();
                for (auto ekv : *e){
                    if (ekv.first != ELEMENT_H) elements += ekv.second;
                }
                delete e;
                delete func_group;
                
                int num = 0;
                if (fa1->functional_groups->at(key).at(0)->position > -1){
                    num = fa1->functional_groups->at(key).size();
                }
                else {
                    for (auto f : fa1->functional_groups->at(key)) num += f->count;
                }
                union_num += elements * num;
            }
            else {
                for (auto fg : kv.second){
                    union_num += fg->get_double_bonds();
                    ElementTable* eee = fg->get_elements();
                    for (auto ekv : *eee){
                        if (ekv.first != ELEMENT_H) union_num += ekv.second;
                    }
                    delete eee;
                }
            }
        }
    }
    
    vector<string> keys_fa1;
    for (auto kv : *(fa1->functional_groups)) keys_fa1.push_back(kv.first);
    vector<string> keys_fa2;
    for (auto kv : *(fa2->functional_groups)) keys_fa2.push_back(kv.first);
    vector<string> diff;
    set_difference(keys_fa2.begin(), keys_fa2.end(), keys_fa1.begin(), keys_fa1.end(), inserter(diff, diff.begin()));
    
    
    
    // functional group occurs only in second fatty acid
    for (auto key : diff){
        if (key == "[X]") continue;
    
        FunctionalGroup* func_group = KnownFunctionalGroups::get_functional_group(key);
        if (func_group){
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
                for (auto fg : fa2->functional_groups->at(key)) num += fg->count;
            }
            union_num += elements * num;
        }
        else {
            for (auto fg : fa2->functional_groups->at(key)){
                union_num += fg->get_double_bonds();
                ElementTable* eee = fg->get_elements();
                for (auto ekv : *eee){
                    if (ekv.first != ELEMENT_H) union_num += ekv.second;
                }
                delete eee;
            } 
        }
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
    
    
    vector<FattyAcid*>* orig_fa_list_1 = &lipid1->lipid->fa_list;
    vector<FattyAcid*>* orig_fa_list_2 = &lipid2->lipid->fa_list;
    
    if (orig_fa_list_1->size() < orig_fa_list_2->size()){
        vector<FattyAcid*>* tmp = orig_fa_list_1;
        orig_fa_list_1 = orig_fa_list_2;
        orig_fa_list_2 = tmp;
    }
    int len_fa1 = orig_fa_list_1->size();
    int len_fa2 = orig_fa_list_2->size();
    
    if (len_fa1 == 0 && len_fa2 == 0) return;
    
    
    if (keep_sn_position || (len_fa1 <= 1 && len_fa2 <= 1)){
        int l = mmin(orig_fa_list_1->size(), orig_fa_list_2->size());
        for (int i = 0; i < l; ++i){
            fatty_acyl_similarity(orig_fa_list_1->at(i), orig_fa_list_2->at(i), union_num, inter_num);
        }
        
        if (orig_fa_list_1->size() > orig_fa_list_2->size()){
            for (int i = orig_fa_list_2->size(); i < orig_fa_list_1->size(); ++i){
                FattyAcid* fa = orig_fa_list_1->at(i);
                union_num += fa->get_double_bonds();
                ElementTable* e = fa->get_elements();
                for (auto kv : *e){
                    if (kv.first != ELEMENT_H) union_num += kv.second;
                }
                delete e;
            }
        }
    }
    
    else {
        int clen = len_fa1 * len_fa1;
        int** cache = new int*[clen];
        for (int i = 0; i < clen; ++i) cache[i] = 0;
        
        for (int i = 0; i < len_fa2; ++i){
            for (int j = 0; j < len_fa1; ++j){
                int uu = 0, ii = 0;
                fatty_acyl_similarity(orig_fa_list_1->at(j), orig_fa_list_2->at(i), uu, ii);
                cache[i * len_fa1 + j] = new int[2]{uu, ii};
            }
        }
        
        if (len_fa1 > len_fa2){
            for (int j = 0; j < len_fa1; ++j){
                
                int uu = orig_fa_list_1->at(j)->get_double_bonds();
                ElementTable* e = orig_fa_list_1->at(j)->get_elements();
                for (auto kv : *e){
                    if (kv.first != ELEMENT_H) uu += kv.second;
                }
                delete e;
                for (int i = len_fa2; i < len_fa1; ++i) cache[i * len_fa1 + j] = new int[2]{uu, 0};
            }
        }
        
        int max_u = 0, max_i = 0;
        double max_q = 0;
        
        for (int i = 0; i < orders.at(len_fa1).size(); ++i){
            int uu = 0, ii = 0;
            for (auto order : orders.at(len_fa1).at(i)){
                int* cell = cache[order.at(0) * len_fa1 + order.at(1)];
                uu += cell[0];
                ii += cell[1];
            }
            double q = (double)ii / (double)uu;
            if (max_q < q){
                max_q = q;
                max_u = uu;
                max_i = ii;
            }
        }
        
        for (int i = 0; i < clen; ++i) { delete []cache[i]; }
        delete []cache;
        
        union_num += max_u;
        inter_num += max_i;
    }
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
            lipidome->intensities.push_back(atof(tokens->at(1).c_str()));
            delete tokens;
        }
        else {
            lipidome->intensities.push_back(1);
        }
        lipids.push_back(line);
        lipidome->lipids.push_back(0);
        all_lipids.push_back(0);
        lipidome->species.push_back("");
        lipidome->classes.push_back("");
    }
    
    vector<int> remove;
    
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
            if (ignore_unknown_lipids){
                cerr << "Warning: ignoring lipid '" << line << "' in file '" << lipid_list_file << "'" << endl;
                remove.push_back(i);
            }
            else {
                cerr << "Error: lipid '" << line << "' cannot be parsed in file '" << lipid_list_file << "'" << endl;
                exit(-1);
            }
        }
    }
    
    if (ignore_unknown_lipids && remove.size() > 0){
        sort(remove.begin(), remove.end());
        for (int i = remove.size() - 1; i >= 0; --i){
            int index = remove.at(i);
            all_lipids.erase(all_lipids.begin() + pos_all + index);
            lipidome->lipids.erase(lipidome->lipids.begin() + index);
            lipidome->species.erase(lipidome->species.begin() + index);
            lipidome->classes.erase(lipidome->classes.begin() + index);
            lipidome->intensities.erase(lipidome->intensities.begin() + index);
        }
    }
    
    
    return lipidome;
}









void LipidSpace::compute_PCA_variances(Mat &m, Array &a){
    double total_var = 0;
    for (int c = 0; c < m.cols; c++){
        double var = 0;
        for(int r = 0; r < m.rows; ++r){
            var += sq(m(r, c));
        }
        a.push_back(var);
        total_var += var;
    }
    total_var = 1. / total_var;
    for (int c = 0; c < m.cols; c++) a[c] *= total_var;
}







double LipidSpace::compute_hausdorff_distance(Table* l1, Table* l2){
    // add intensities to hausdorff matrix
    Mat tm1, tm2;
    if (without_quant){
        tm1.rewrite_transpose(l1->m);
        tm2.rewrite_transpose(l2->m);
    }
    else {
        tm1.rewrite(l1->m);
        tm1.add_column(l1->intensities);
        tm1.transpose();
        tm2.rewrite(l2->m);
        tm2.add_column(l2->intensities);
        tm2.transpose();
    }
    assert(tm1.rows == tm2.rows);
    
    double max_h = 0;
    
    for (int tm2c = 0; tm2c < tm2.cols; tm2c++){
        double min_h = 1e9;
        double* tm2col = tm2.data() + (tm2c * tm2.rows);
        for (int tm1c = 0; tm1c < tm1.cols; ++tm1c){
            double dist = 0;
            double* tm1col = tm1.data() + (tm1c * tm1.rows);
            for (int r = 0; r < tm1.rows; r++){
                dist += sq(tm1col[r] - tm2col[r]);
            }
            min_h = mmin(min_h, dist);
        }
        max_h = mmax(max_h, min_h);
    }
    
    for (int tm1c = 0; tm1c < tm1.cols; ++tm1c){
        double min_h = 1e9;
        for (int tm2c = 0; tm2c < tm2.cols; tm2c++){
            double dist = 0;
            for (int r = 0; r < tm1.rows; r++){
                dist += sq(tm1(r, tm1c) - tm2(r, tm2c));
            }
            min_h = mmin(min_h, dist);
        }
        max_h = mmax(max_h, min_h);
    }
    
    return sqrt(max_h);
}









void LipidSpace::compute_hausdorff_matrix(vector<Table*>* lipidomes, Mat &distance_matrix){
    int n = lipidomes->size();
    distance_matrix.reset(n, n);
    for (int i = 0; i < n - 1; ++i){
        for (int j = i + 1; j < n; ++j){
            distance_matrix(i, j) = compute_hausdorff_distance(lipidomes->at(i), lipidomes->at(j));
            distance_matrix(j, i) = distance_matrix(i, j);
        }
    }
}



double pairwise_sum(Mat &m){
    assert(m.cols == 2);
    Mat tm(m, true);
    
    double dist_sum = 0;
    for (int tm2c = 0; tm2c < tm.cols; tm2c++){
        double* tm2col = tm.data() + (tm2c * tm.rows);
        for (int tm1c = 0; tm1c < tm.cols; ++tm1c){
            double* tm1col = tm.data() + (tm1c * tm.rows);
            double dist = sq(tm1col[0] - tm2col[0]);
            dist += sq(tm1col[1] - tm2col[1]);
            dist_sum += sqrt(dist);
        }
    }
    return dist_sum;
}



void automated_annotation(Array &xx, Array &yy, int l, Mat &label_points){
    Array label_xx(&xx, l);
    Array label_yy(&yy, l);
    Array orig_label_xx(&xx, l);
    Array orig_label_yy(&yy, l);
    
    double sigma_x = xx.stdev();
    double sigma_y = yy.stdev();
    
    Array all_xx(&label_xx);
    all_xx.add(xx);
    Array all_yy(&label_yy);
    all_yy.add(yy);
    Mat r;
    r.add_column(all_xx);
    r.add_column(all_yy);
    
     
    double ps = pairwise_sum(r) / sq(xx.size());
    double nf_x = 0.2 * sigma_x / ps; // normalization factor
    double nf_y = 0.2 * sigma_y / ps; // normalization factor
    
    
    #define pseq seq(2 * l, all_xx.size() - 1)
    #define lseq seq(0, 2 * l - 1)
    
    // do 30 iterations to find an equilibrium of pulling and pushing forces
    // for the label positions
    /*
    for (int i = 0; i < 30; ++i){
        
        for (int ii = 0; ii < l; ++ii){
            double l_xx = label_xx(ii);
            double l_yy = label_yy(ii);

            VectorXd distances = ((all_xx.array() - l_xx).square() + (all_yy.array() - l_yy).square()).sqrt();
            
            // apply pushing force
            VectorXd force_x = nf_x * (all_xx(pseq).array() - l_xx) / (distances(pseq).array().square() + 1e-16);
            VectorXd force_y = nf_y * (all_yy(pseq).array() - l_yy) / (distances(pseq).array().square() + 1e-16);
            VectorXd force_x_label = 50 * nf_x * (all_xx(lseq).array() - l_xx) / (distances(lseq).array().square() + 1e-16);
            VectorXd force_y_label = 30 * nf_y * (all_yy(lseq).array() - l_yy) / (distances(lseq).array().square() + 1e-16);
            
            l_xx -= (force_x.array().sum() + force_x_label.array().sum());
            l_yy -= (force_y.array().sum() + force_y_label.array().sum());

            // apply pulling force
            label_xx(ii) = orig_label_xx(ii) + (l_xx - orig_label_xx(ii)) * 0.6;
            label_yy(ii) = orig_label_yy(ii) + (l_yy - orig_label_yy(ii)) * 0.6;
            all_xx(ii) = label_xx(ii);
            all_yy(ii) = label_yy(ii);
        }
    }
    */
    
        
    label_points.reset(0, 0);
    label_points.add_column(label_xx);
    label_points.add_column(label_yy);
}





void LipidSpace::plot_PCA(Table* lipidome, string output_folder){
    string output_file_name = lipidome->file_name;
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
    for (int i = 0; i < lipidome->classes.size(); ++i){
        string lipid_class = lipidome->classes.at(i);
        if (uncontains(indexes, lipid_class)) indexes.insert({lipid_class, vector<int>()});
        indexes.at(lipid_class).push_back(i);
    }
    
    Array mean_x;
    Array mean_y;
    vector<string> labels;
    
    double min_x = 0, max_x = 0, min_y = 0, max_y = 0;
    
    Array intensities(lipidome->intensities);
    double mean = 0, stdev = 0;
    for (auto val : intensities) mean += val;
    mean /= (double)intensities.size();
    
    for (auto val : intensities) stdev += sq(val - mean);
    stdev = sqrt(stdev / (double)intensities.size());
    
    if (stdev > 1e-16){
        stdev = 5. / stdev;
        for (int i = 0; i < intensities.size(); ++i) {
            intensities[i] *= stdev;
        }
    }
    
    
    // plot the dots
    for (auto kv : indexes){
        vector<double> x;
        vector<double> y;
        vector<double> intens;
        double mx = 0, my = 0;
        for (auto i : kv.second){
            double vx = lipidome->m(i, 0), vy = lipidome->m(i, 1);
            x.push_back(vx);
            mx += vx;
            min_x = mmin(min_x, vx);
            max_x = max(max_x, vx);
            y.push_back(vy);
            my += vy;
            min_y = mmin(min_y, vy);
            max_y = max(max_y, vy);
            intens.push_back(intensities[i] > 1 ? log(intensities[i]) : 0.1);
        }
        mean_x.push_back(mx / (double)x.size());
        mean_y.push_back(my / (double)y.size());
        
        stringstream label;
        label << kv.first << " (" << x.size() << ")";
        labels.push_back(label.str());
        plt::scatter(x, y, intens);
    }
    
    for (int i = 0; i < lipidome->m.rows; ++i){
        mean_x.push_back(lipidome->m(i, 0));
        mean_y.push_back(lipidome->m(i, 1));
    }

    
    // plot the annotations
    Mat label_m;
    automated_annotation(mean_x, mean_y, labels.size(), label_m);
    
    for (int i = 0; i < labels.size(); ++i){
        plt::annotate(labels.at(i), mean_x.at(i), mean_y.at(i), label_m(i, 0), label_m(i, 1), {{"color", "#aaaaaa"}, {"fontsize", "6"}, {"weight", "bold"}, {"verticalalignment", "center"}, {"horizontalalignment", "center"}});
    }
    min_x = mmin(min_x, label_m.col_min(0));
    max_x = mmax(max_x, label_m.col_max(0));
    min_y = mmin(min_y, label_m.col_min(label_m.cols - 1));
    max_y = mmax(max_y, label_m.col_max(label_m.cols - 1));
    
    
    cout << "Running principal component analysis" << endl;
    Array pca_variances;
    compute_PCA_variances(lipidome->m, pca_variances);
    
    stringstream xlabel;
    xlabel.precision(1);
    xlabel << fixed << "Principal component 1 (" << (pca_variances[0] * 100) << " %)";
    //xlabel << fixed << "Principal component 1";
    
    stringstream ylabel;
    ylabel.precision(1);
    ylabel << fixed << "Principal component 2 (" << (pca_variances[1] * 100) << " %)";
    //ylabel << fixed << "Principal component 2";
    
    plt::xlabel(xlabel.str());
    plt::ylabel(ylabel.str());
    
    plt::xlim(min_x * 1.1, max_x * 1.1);
    plt::ylim(min_y * 1.1, max_y * 1.1);
    
    cout << "storing '" << output_file_name << "'" << endl;
    plt::xticks((vector<int>){});
    plt::yticks((vector<int>){});
    plt::save(output_file_name);
    plt::close();
}








void LipidSpace::report_hausdorff_matrix(vector<Table*>* lipidomes, Mat &distance_matrix, string output_folder){
    ofstream off(output_folder + "/hausdorff_distances.csv");
    vector<string> striped_names;
    int n = distance_matrix.rows;
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
    
    
    // set equal intensities, later important for ploting
    int n = global_lipidome->lipids.size();
    global_lipidome->intensities.resize(n);
    for (int i = 0; i < n; ++i)global_lipidome->intensities[i] = 3;
    
    
    // compute distances
    cout << "Computing pairwise distance matrix for " << n << " lipids" << endl;
    Mat& distance_matrix = global_lipidome->m;
    distance_matrix.reset(n, n);
    
    
    double total_num = (n * (n - 1)) >> 1;
    double tn = 0;
    double next_announce = 10;
    
    #pragma omp parallel for
    for (int ii = 0; ii < n * n; ++ii){
        int i = ii / n;
        int j = ii % n;
        
        if (i < j){
            
            #pragma omp critical
            {
                if (++tn / total_num * 100. >= next_announce){
                    cout << next_announce << "% ";
                    cout.flush();
                    next_announce += 10;
                }
            }
            
            int union_num, inter_num;
            lipid_similarity(global_lipidome->lipids.at(i), global_lipidome->lipids.at(j), union_num, inter_num);
            double distance = unboundend_distance ?
                (double)union_num / (double)inter_num - 1. // unboundend distance [0; inf[
                            :
                1 - (double)inter_num / (double)union_num; // bounded distance [0; 1]
            distance_matrix(i, j) = distance;
            distance_matrix(j, i) = distance;
        }
    }
    cout << endl;
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
        Indexes indexes;
        for (auto lipid_species : lipidome->species) indexes.push_back(lipid_indexes.at(lipid_species));
        lipidome->m.rewrite(global_lipidome->m, indexes);
    }
}








void LipidSpace::normalize_intensities(vector<Table*>* lipidomes, Table* global_lipidome){
    Mat &m = global_lipidome->m;
    
    double global_stdev = 0;
    for (int i = 0; i < m.rows; ++i) global_stdev += sq(m.m[i]);
    global_stdev = sqrt(global_stdev / (double)m.rows);
    
    
    for (auto lipidome : *lipidomes){
        
        Array &intensities = lipidome->intensities;
        double mean = 0, stdev = 0;
        for (auto val : intensities) mean += val;
        mean /= (double)intensities.size();
        
        for (auto val : intensities) stdev += sq(val - mean);
        stdev = sqrt(stdev / (double)intensities.size());
        
        if (stdev > 1e-16){
            stdev = global_stdev / stdev;
            for (int i = 0; i < intensities.size(); ++i) {
                intensities[i] *= stdev;
            }
        }
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
    vector<Array> intensities;
    
    
    int line_cnt = 0;
    int num_cols = 0;
    while (getline(infile, line)){
        if (line_cnt++ == 0){
            vector<string>* tokens = split_string(line, ',', '"', true);
            num_cols = tokens->size();
            for (int i = 1; i < tokens->size(); ++i){
                lipidomes->push_back(new Table(tokens->at(i)));
                intensities.push_back(Array());
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
            if (ignore_unknown_lipids){
                cerr << "Warning: ignoring lipid '" << tokens->at(0) << "' in file '" << table_file << "'" << endl;
                continue;
            }
            else {
                cerr << "Error: lipid '" << tokens->at(0) << "' cannot be parsed in file '" << table_file << "'" << endl;
                exit(-1);
            }
        }
        
        if (l == 0) {
            if (ignore_unknown_lipids){
                cerr << "Warning: ignoring lipid '" << tokens->at(0) << "' in file '" << table_file << "'" << endl;
                continue;
            }
            else {
                cerr << "Error: lipid '" << tokens->at(0) << "' cannot be parsed in file '" << table_file << "'" << endl;
                exit(-1);
            }
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
        lipidomes->at(i)->intensities.reset(intensities.at(i));
    }
}




void LipidSpace::store_distance_table(Table* lipidome, string output_folder){
    string output_file = output_folder + "/distance_matrix.csv";
    
    stringstream table_stream;
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
    ofstream output_stream(output_file);
    output_stream << table_stream.str();
    
}





void print_help(){
    cerr << "usage: " << endl;
    cerr << "  > ./lipidspace [options] output_folder table lipid_table[csv]" << endl;
    cerr << "  > ./lipidspace [options] output_folder lists lipid_list[csv], ..." << endl;
    cerr << "modes either 'table' or 'lists'." << endl << endl;
    cerr << "options:" << endl;
    cerr << " -h\t\tshow this help message" << endl;
    cerr << " -sn\t\tignore sn-positions, compare all pairwise fatty acyl chains" << endl;
    cerr << " -i\t\tignore unknown lipids (default: exit with error)" << endl;
    cerr << " -q\t\tignore quantitative data if present (default: guess what, consider them)" << endl;
    cerr << " -d\t\tstore distance tables" << endl;
    cerr << " -p\t\tplot only figure for global principal component analysis" << endl;
    cerr << " -u\t\tunbounded distance to inf (default: 0 <= distance <= 1)" << endl;
}



int main(int argc, char** argv) {
    // parameters to change
    bool keep_sn_position = true;
    bool ignore_unknown_lipids = false;
    bool plot_pca = true; 
    bool plot_pca_lipidomes = true;
    bool storing_distance_table = false;
    bool unboundend_distance = false;
    bool without_quant = false;
    
    if (argc < 4) {
        print_help();
        exit(-1);
    }
    
    map<string, int> options = {{"-sn", 0}, {"-i", 1}, {"-d", 2}, {"-p", 3}, {"-h", 4}, {"--help", 4}, {"-u", 5}, {"-q", 6}};
    
    
    int num_opt = 0;
    while (true){
        if (uncontains(options, string(argv[1 + num_opt]))) break;
        int opt = options.at(argv[1 + num_opt++]);
        
        switch (opt){
            case 0: keep_sn_position = false; break;
            case 1: ignore_unknown_lipids = true; break;
            case 2: storing_distance_table = true; break;
            case 3: plot_pca_lipidomes = false; break;
            case 4: print_help(); return 0;
            case 5: unboundend_distance = true; break;
            case 6: without_quant = true; break;
        }
    }
    
        
    // getting input
    string output_folder = argv[1 + num_opt];
    string mode = argv[2 + num_opt];
    
    // check if output folder exists
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
    
    
    lipid_space.keep_sn_position = keep_sn_position;
    lipid_space.ignore_unknown_lipids = ignore_unknown_lipids;
    lipid_space.unboundend_distance = unboundend_distance;
    lipid_space.without_quant = without_quant;
    
    
    // loadig each lipidome
    if (mode == "lists"){
        for (int i = 3 + num_opt; i < argc; ++i) lipidomes.push_back(lipid_space.load_list(argv[i]));
    }
    // loading lipid matrix
    else {
        lipid_space.load_table(argv[3 + num_opt], &lipidomes);
    }
    
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    // compute PCA matrixes for the complete lipidome
    Table* global_lipidome = lipid_space.compute_global_distance_matrix(&lipidomes);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Time difference = " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]" << endl;
    cout << "Time difference = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() << "[us]" << endl << endl;
    
    
    // storing the distance matrix
    if (storing_distance_table){
        lipid_space.store_distance_table(global_lipidome, output_folder);
    }
    
    
    begin = chrono::steady_clock::now();
    // perform the principal component analysis
    Mat tmp;
    global_lipidome->m.PCA(tmp, lipid_space.cols_for_pca);
    global_lipidome->m.rewrite(tmp);
    end = chrono::steady_clock::now();
    cout << "Time difference = " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]" << endl;
    cout << "Time difference = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() << "[us]" << endl << endl;
    
        
    // cutting the global PCA matrix back to a matrix for each lipidome
    lipid_space.separate_matrixes(&lipidomes, global_lipidome);

    
    // plotting all lipidome PCAs
    if (plot_pca && lipidomes.size() > 1){
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
        
        begin = chrono::steady_clock::now();
        // computing the hausdorff distance matrix for all lipidomes
        Mat distance_matrix;
        lipid_space.compute_hausdorff_matrix(&lipidomes, distance_matrix);
        
        
        // storing hausdorff distance matrix into file
        if (storing_distance_table)
            lipid_space.report_hausdorff_matrix(&lipidomes, distance_matrix, output_folder);
        
        // ploting the dendrogram
        lipid_space.plot_dendrogram(&lipidomes, distance_matrix, output_folder);
        end = chrono::steady_clock::now();
        cout << "Time difference = " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]" << endl;
        cout << "Time difference = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() << "[us]" << endl << endl;
    }
    
    
    // free all allocated data
    for (auto table : lipidomes) delete table;
    delete global_lipidome;
    
    
    if (plot_pca || plot_pca_lipidomes || lipidomes.size() > 1){
        Py_Finalize();
    }
    
    return 0;
}
