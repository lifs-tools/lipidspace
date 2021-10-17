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
 
using namespace std;
using namespace Eigen;



class Table {
public:
    vector<string> species;
    vector<string> classes;
    MatrixXd m;
    
    Table(int len) : m(len, len){}
};


class LipidSpace {
public:
    LipidParser parser;
    map<string, int*> class_matrix;
    
    LipidSpace();
    Table* create_Table(string lipid_list_file);
    ArrayXd compute_PCA_variances(MatrixXd m);
    void cut_cycle(FattyAcid* fa);
    MatrixXd compute_PCA(MatrixXd m);
    void lipid_similarity(LipidAdduct* l1, LipidAdduct* l2, int& union_num, int& inter_num);
    void fatty_acyl_similarity(FattyAcid* f1, FattyAcid* f2, int& union_num, int& inter_num);
};



LipidSpace::LipidSpace(){
    // load precomputed class distance matrix
    ifstream infile("data/classes-matrix.csv");
    if (!infile.good()){
        cout << "Error: file 'data/classes-matrix.csv' not found." << endl;
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
        cout << "Error: key '" << key << "' not in precomputed class matrix" << endl;
        exit(-1);
    }
    union_num = class_matrix.at(key)[0];
    inter_num = class_matrix.at(key)[1];
    int l = min(lipid1->lipid->fa_list.size(), lipid2->lipid->fa_list.size());
    for (int i = 0; i < l; ++i){
        fatty_acyl_similarity(lipid1->lipid->fa_list.at(i), lipid2->lipid->fa_list.at(i), union_num, inter_num);
    }
      
      
    vector<FattyAcid*>* fa_list_1 = &lipid1->lipid->fa_list;
    vector<FattyAcid*>* fa_list_2 = &lipid2->lipid->fa_list;
    if (fa_list_1->size() < fa_list_2->size()){
        fa_list_1 = &lipid2->lipid->fa_list;
        fa_list_2 = &lipid1->lipid->fa_list;
    }
        
    if (fa_list_1->size() > fa_list_2->size()){
        for (int i = fa_list_2->size(); i < fa_list_1->size(); ++i){
            FattyAcid* fa = fa_list_1->at(i);
            union_num += fa->get_double_bonds();
            ElementTable* e = fa->get_elements();
            for (auto kv : *e){
                if (kv.first != ELEMENT_H) union_num += kv.second;
            }
            delete e;
        }
    }
}


Table* LipidSpace::create_Table(string lipid_list_file){
    cout << "reading '" << lipid_list_file << "'" << endl;
    
    // load and parse lipids
    ifstream infile(lipid_list_file);
    if (!infile.good()){
        cout << "Error: file '" << lipid_list_file << "' not found." << endl;
        exit(-1);
    }
    string line;
    
    
    vector<LipidAdduct*> lipids;
    while (getline(infile, line)){
        if (line.length() == 0) continue;
        
        vector<string>* tokens = goslin::split_string(line, '\n', '"');
        line = strip(tokens->at(0), '"');
        delete tokens;
        
        try {
            LipidAdduct* l = parser.parse(line);
            for (auto fa : l->lipid->fa_list) cut_cycle(fa);
            lipids.push_back(l);
        }
        catch (exception &e) {
            cout << "Error: lipid '" << line << "' cannot be parsed" << endl;
        }
    }
    
    int n = lipids.size();
    Table* table = new Table(n);
    
    // compute distances
    MatrixXd distance_matrix(n, n);
    for (int i = 0; i < n - 1; ++i){
        table->species.push_back(lipids.at(i)->get_lipid_string());
        table->classes.push_back(lipids.at(i)->get_lipid_string(CLASS));
        distance_matrix(i, i) = 0;
        for (int j = i + 1; j < n; ++j){
            int union_num, inter_num;
            lipid_similarity(lipids.at(i), lipids.at(j), union_num, inter_num);
            double distance = 1. / ((double)inter_num / (double)union_num) - 1.;
            distance_matrix(i, j) = distance;
            distance_matrix(j, i) = distance;
        }
    }
    table->m = compute_PCA(distance_matrix);
    
    for (auto lipid : lipids) delete lipid;
    return table;
}





ArrayXd LipidSpace::compute_PCA_variances(MatrixXd m){
    ArrayXd var = m.array().square().colwise().sum();
    return var / var.sum();
}





MatrixXd LipidSpace::compute_PCA(MatrixXd m){
    // scale and transform the matrix
    int r = m.rows();
    m = m.rowwise() - (m.rowwise().mean()).transpose();
    VectorXd norm = (r / m.array().square().colwise().sum()).sqrt();
    for (int i = 0; i < r; ++i) m.block(0, i, r, 1) *= norm[i];
    
    // calculate the covariance matrix
    MatrixXd centered = m.rowwise() - m.colwise().mean();
    MatrixXd cov = (centered.adjoint() * centered) / double(m.rows() - 1);
    
    // calculate eigenvectors of the covariance matrix
    SelfAdjointEigenSolver<MatrixXd> eigensolver(cov);
    
    // reverse the order of columns
    MatrixXd evecs = eigensolver.eigenvectors();
    MatrixXd rev = evecs.rowwise().reverse();
    
    // carry out the transformation on the data using eigenvectors
    return (rev.transpose() * m.transpose()).transpose();
}


int main(int argc, char** argv) {
    bool plot_pca = true;
    bool store_Tables = true;
    
    if (argc < 3) {
        cout << "usage: python3 " << argv[0] << " output_folder lipid_list[csv], ..." << endl;
        exit(-1);
    }
        
    string output_folder = argv[1];
    vector<Table*> tables;
    LipidSpace lipid_space;
    
    for (int i = 2; i < argc; ++i) tables.push_back(lipid_space.create_Table(argv[i]));
    
    cout << tables.size() << endl;
    
    for (auto table : tables) delete table;

    return 0;
}
