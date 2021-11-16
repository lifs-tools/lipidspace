#include "lipidspace/lipidspace.h"
 
 

double linkage(Node* n1, Node* n2, Matrix &m, Linkage linkage = COMPLETE){
    double v = 1e9 * (linkage == SINGLE);
    for (auto index1 : n1->indexes){
        for (auto index2 : n2->indexes){
            if ((linkage == COMPLETE && v < m(index1, index2)) || (linkage == SINGLE && v > m(index1, index2))){
                v = m(index1, index2);
            }
        }
    }
    return v;
}


void LipidSpace::create_dendrogram(){
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    
    int n = hausdorff_distances.rows;
    
    vector<Node*> nodes;
    for (int i = 0; i < n; ++i) nodes.push_back(new Node(i));
    
    while (nodes.size() > 1){
        double min_val = 1e9;
        int ii = 0, jj = 0;
        for (int i = 0; i < (int)nodes.size() - 1; ++i){
            for (int j = i + 1; j < (int)nodes.size(); ++j){
                double val = linkage(nodes.at(i), nodes.at(j), hausdorff_distances, COMPLETE);
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
    
    double* ret = nodes.front()->execute(0, &dendrogram_points, &dendrogram_sorting);
    delete []ret;
    delete nodes.front();
}






LipidSpace::LipidSpace() {
    keep_sn_position = true;
    ignore_unknown_lipids = false;
    unboundend_distance = false;
    without_quant = false;
    global_lipidome = new Table("global_lipidome");
    progress = 0;
    analysis_finished = false;
    
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

int LipidSpace::cols_for_pca = 7;


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
    delete global_lipidome;
    for (auto table : lipidomes) delete table;
}






void LipidSpace::cut_cycle(FattyAcid* fa){
    if (uncontains_val_p(fa->functional_groups, "cy")) return;

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
                    if (contains_val(keep_fg, func_group)){
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
        if (contains_val(bond_types, ESTER)){
            union_num += 2; // one oxygen plus one double bond
            inter_num += 2;
        }
        if (contains_val(bond_types, ETHER_PLASMENYL)){
            union_num += 1; // one double bond
            inter_num += 1;
        }
    }
    
    else if (contains_val(bond_types, ESTER)){
        if (contains_val(bond_types, ETHER_PLASMENYL)){
            union_num += 3;
        }
        
        else if (contains_val(bond_types, ETHER_PLASMANYL)){
            union_num += 2;
        }
        
        else if (contains_val(bond_types, LCB_REGULAR) || contains_val(bond_types, LCB_EXCEPTION)){
            union_num += 2;
        }
    }
    
    else if (contains_val(bond_types, LCB_EXCEPTION) || contains_val(bond_types, LCB_REGULAR)){
        if (contains_val(bond_types, ETHER_PLASMENYL)){
            union_num += 1;
        }
    }
        
    else if (contains_val(bond_types, ETHER_PLASMENYL) && contains_val(bond_types, ETHER_PLASMANYL)){
        union_num += 1;
    }
        
    else { 
        throw LipidException("Cannot compute similarity between chains");
    }

    
    // compare lengths
    int m1 = contains_val(lcbs, fa1->lipid_FA_bond_type) * 2;
    int m2 = contains_val(lcbs, fa2->lipid_FA_bond_type) * 2;
    
    inter_num += mmin(fa1->num_carbon - m1, fa2->num_carbon - m1);
    union_num += mmax(fa1->num_carbon - m2, fa2->num_carbon - m2);
    
    
    // compare double bonds
    int db1 = 0, db2 = 0;
    if (fa1->double_bonds->double_bond_positions.size() > 0 && fa2->double_bonds->double_bond_positions.size() > 0){
        int common = 0;
        for (auto kv : fa1->double_bonds->double_bond_positions){
            common += contains_val(fa2->double_bonds->double_bond_positions, kv.first) && fa2->double_bonds->double_bond_positions.at(kv.first) == kv.second;
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
        if (contains_val_p(fa2->functional_groups, key)){ // functional group occurs in both fatty acids
            
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
    if (!contains_val(class_matrix, key)){
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
            for (int i = (int)orig_fa_list_2->size(); i < (int)orig_fa_list_1->size(); ++i){
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
        
        for (int i = 0; i < (int)orders.at(len_fa1).size(); ++i){
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
    //cout << "reading list '" << lipid_list_file << "'" << endl;
    
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
    
    //#pragma omp parallel for
    for (int i = 0; i < (int)lipids.size(); ++i) { 
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
            if (!ignore_unknown_lipids){
                throw LipidException(string(e.what()) + ": lipid '" + line + "' cannot be parsed.");
            }
            else {
                remove.push_back(i);
            }
            /*
            if (ignore_unknown_lipids){
                cerr << "Warning: ignoring lipid '" << line << "' in file '" << lipid_list_file << "'" << endl;
                remove.push_back(i);
            }
            else {
                cerr << "Error: lipid '" << line << "' cannot be parsed in file '" << lipid_list_file << "'" << endl;
                exit(-1);
            }
            */
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









void LipidSpace::compute_PCA_variances(Matrix &m, Array &a){
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
    Matrix tm1(l1->m);
    Matrix tm2(l2->m);
    if (!without_quant){
        tm1.add_column(l1->intensities);
        tm2.add_column(l2->intensities);
    }
    tm1.pad_cols_4(); // pad matrix columns with zeros to perform faster intrinsics
    tm2.pad_cols_4();
    tm1.transpose();
    tm2.transpose();
    assert(tm1.rows == tm2.rows);
    
    double max_h = 0;
    
    for (int tm2c = 0; tm2c < tm2.cols; tm2c++){
        double min_h = 1e9;
        double* tm2col = tm2.data() + (tm2c * tm2.rows);
        for (int tm1c = 0; tm1c < tm1.cols; ++tm1c){
            __m256d dist = {0, 0, 0, 0};
            double* tm1col = tm1.data() + (tm1c * tm1.rows);
            for (int r = 0; r < tm1.rows; r += 4){
                __m256d val1 = _mm256_loadu_pd(&tm1col[r]);
                __m256d val2 = _mm256_loadu_pd(&tm2col[r]);
                __m256d sub = _mm256_sub_pd(val1, val2);
                dist = _mm256_fmadd_pd(sub, sub, dist);
            }
            double distance = dist[0] + dist[1] + dist[2] + dist[3];
            min_h = mmin(min_h, distance);
        }
        max_h = mmax(max_h, min_h);
    }
    
    for (int tm1c = 0; tm1c < tm1.cols; ++tm1c){
        double min_h = 1e9;
        double* tm1col = tm1.data() + (tm1c * tm1.rows);
        for (int tm2c = 0; tm2c < tm2.cols; tm2c++){
            __m256d dist = {0, 0, 0, 0};
            double* tm2col = tm2.data() + (tm2c * tm2.rows);
            for (int r = 0; r < tm1.rows; r += 4){
                __m256d val1 = _mm256_loadu_pd(&tm1col[r]);
                __m256d val2 = _mm256_loadu_pd(&tm2col[r]);
                __m256d sub = _mm256_sub_pd(val1, val2);
                dist = _mm256_fmadd_pd(sub, sub, dist);
            }
            double distance = dist[0] + dist[1] + dist[2] + dist[3];
            min_h = mmin(min_h, distance);
        }
        max_h = mmax(max_h, min_h);
    }
    
    return sqrt(max_h);
}









void LipidSpace::compute_hausdorff_matrix(){
    
    int n = lipidomes.size();
    hausdorff_distances.reset(n, n);
            
    #pragma omp parallel for
    for (int ii = 0; ii < sq(n); ++ii){ 
        int i = ii / n;
        int j = ii % n;
        if (i < j){
            hausdorff_distances(i, j) = compute_hausdorff_distance(lipidomes.at(i), lipidomes.at(j));
            hausdorff_distances(j, i) = hausdorff_distances(i, j);
        }
    }
}






void LipidSpace::report_hausdorff_matrix(string output_folder){
    ofstream off(output_folder + "/hausdorff_distances.csv");
    vector<string> striped_names;
    int n = hausdorff_distances.rows;
    off << "ID";
    for (int i = 0; i < n; ++i){
        vector<string>* tokens = split_string(lipidomes.at(i)->file_name, '/', '"');
        off << "\t" << tokens->back();
        striped_names.push_back(tokens->back());
        delete tokens;
    } off << endl;
    
    for (int i = 0; i < n; ++i){
        off << striped_names.at(i);
        for (int j = 0; j < n; ++j){
            off << "\t" << hausdorff_distances(i, j);
        } off << endl;
    }
}










void LipidSpace::compute_global_distance_matrix(){
    global_lipidome->species.clear();
    global_lipidome->classes.clear();
    global_lipidome->lipids.clear();
    global_lipidome->intensities.clear();
    global_lipidome->m.reset(1, 1);
    
    
    set<string> registered_lipids;
    
    for (auto lipidome : lipidomes){
        for (int i = 0; i < (int)lipidome->species.size(); ++i){
            string lipid_species = lipidome->species.at(i);
            if (contains_val(registered_lipids, lipid_species)) continue;
            registered_lipids.insert(lipid_species);
            global_lipidome->species.push_back(lipid_species);
            global_lipidome->classes.push_back(lipidome->classes.at(i));
            global_lipidome->lipids.push_back(lipidome->lipids.at(i));
        }
    }
    
    
    // set equal intensities, later important for ploting
    int n = global_lipidome->lipids.size();
    global_lipidome->intensities.resize(n);
    for (int i = 0; i < n; ++i) global_lipidome->intensities[i] = 1;
    
    // compute distances
    Matrix& distance_matrix = global_lipidome->m;
    distance_matrix.reset(n, n);
    
    
    double total_num = (n * (n - 1)) >> 1;
    double next_tp = 1;
    
    if (progress){
        int n = global_lipidome->lipids.size();
        progress->max_progress = (n * (n - 1));
        progress->set_max(progress->max_progress);
    }    
    
    
    #pragma omp parallel for
    for (int ii = 0; ii < n * n; ++ii){
        int i = ii / n;
        int j = ii % n;
        
        if (i < j){
            
            bool go_on = true;
            #pragma omp critical
            {
                if (progress){
                    go_on = !progress->stop_progress;
                    if (++progress->current_progress / total_num * 100. >= next_tp){
                        progress->set(progress->current_progress);
                        next_tp += 1;
                    }
                }
            }
            if (go_on){
                int union_num, inter_num;
                lipid_similarity(global_lipidome->lipids.at(i), global_lipidome->lipids.at(j), union_num, inter_num);
                double distance = unboundend_distance ?
                    ((double)union_num / (double)inter_num - 1.) // unboundend distance [0; inf[
                                :
                    (1. - (double)inter_num / (double)union_num); // bounded distance [0; 1]
                distance_matrix(i, j) = distance;
                distance_matrix(j, i) = distance;
            }
        }
    }
}









void LipidSpace::separate_matrixes(){
    map<string, int> lipid_indexes;
    for (int i = 0; i < (int)global_lipidome->species.size(); ++i){
        lipid_indexes.insert({global_lipidome->species.at(i), i});
    }
    
    for (auto lipidome : lipidomes){
        Indexes indexes;
        for (auto lipid_species : lipidome->species) indexes.push_back(lipid_indexes.at(lipid_species));
        lipidome->m.rewrite(global_lipidome->m, indexes);
    }
}








void LipidSpace::normalize_intensities(){
    Matrix &m = global_lipidome->m;
    
    double global_stdev = 0;
    for (int i = 0; i < m.rows; ++i) global_stdev += sq(m.m[i]);
    global_stdev = sqrt(global_stdev / (double)m.rows);
    
    
    for (auto lipidome : lipidomes){
        
        Array &intensities = lipidome->intensities;
        double mean = 0, stdev = 0;
        for (auto val : intensities) mean += val;
        mean /= (double)intensities.size();
        
        for (auto val : intensities) stdev += sq(val - mean);
        stdev = sqrt(stdev / (double)intensities.size());
        
        if (stdev > 1e-16){
            stdev = global_stdev / stdev;
            for (int i = 0; i < (int)intensities.size(); ++i) {
                intensities[i] *= stdev;
            }
        }
    }
}







void LipidSpace::load_table(string table_file){
    //cout << "reading table '" << table_file << "'" << endl;
    
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
            for (int i = 1; i < (int)tokens->size(); ++i){
                lipidomes.push_back(new Table(tokens->at(i)));
                intensities.push_back(Array());
            }
            delete tokens;
            continue;
        }
        if (line.length() == 0) continue;
        

                                                                          
        vector<string>* tokens = split_string(line, ',', '"', true);
        if ((int)tokens->size() != num_cols) {
            cerr << "Error in line '" << line_cnt << "' number of cells does not match with number of column labels" << endl;
            exit(-1);
        }
        
        LipidAdduct* l = 0;
        try {
            l = parser.parse(tokens->at(0));
        }
        catch (exception &e) {
            if (!ignore_unknown_lipids){
                throw LipidException(string(e.what()) + ": lipid '" + tokens->at(0) + "' cannot be parsed.");
            }
            else {
               continue;
            }
        }
        
        if (l == 0) {
            if (!ignore_unknown_lipids){
                throw LipidException("Lipid '" + tokens->at(0) + "' cannot be parsed.");
            }
            else {
               continue;
            }
        }
            
        // deleting adduct since not necessary
        if (l->adduct != 0){
            delete l->adduct;
            l->adduct = 0;
        }
        all_lipids.push_back(l);
        for (int i = 1; i < (int)tokens->size(); ++i){
            string val = tokens->at(i);
            if (!contains_val(NA_VALUES, val)){
                Table* lipidome = lipidomes.at(i - 1);
                lipidome->lipids.push_back(l);
                lipidome->species.push_back(l->get_lipid_string());
                lipidome->classes.push_back(l->get_lipid_string(CLASS));
                intensities.at(i - 1).push_back(atof(val.c_str()));
            }
        }
        for (auto fa : l->lipid->fa_list) cut_cycle(fa);
        delete tokens;
    }
        
    for (int i = 0; i < (int)lipidomes.size(); ++i){
        lipidomes.at(i)->intensities.reset(intensities.at(i));
    }
}




void LipidSpace::store_distance_table(Table* lipidome, string output_folder){
    string output_file = output_folder + "/distance_matrix.csv";
    
    stringstream table_stream;
    table_stream << "ID";
    for (auto lipid : lipidome->species) table_stream << "\t" << lipid;
    table_stream << endl;
    
    for (int i = 0; i < (int)lipidome->species.size(); ++i){
        table_stream << lipidome->species.at(i);
        for (int j = 0; j < (int)lipidome->species.size(); ++j){
            table_stream << "\t" << lipidome->m(i, j);
        }
        table_stream << endl;
    }
    ofstream output_stream(output_file);
    output_stream << table_stream.str();
    
}


std::thread LipidSpace::run_analysis_thread(Progress *_progress) {
    return std::thread([=] { run_analysis(_progress); });
}


bool mysort(double* a, double* b){ return a[0] < b[0];}

void LipidSpace::run_analysis(Progress *_progress){
    if (lipidomes.size() == 0) return;
    
    
    analysis_finished = false;
    
    if (_progress){
        progress = _progress;
        progress->set(0);
    }
    
    
    // compute PCA matrixes for the complete lipidome
    if (!progress || !progress->stop_progress){
        compute_global_distance_matrix();
    }
    
    
    cols_for_pca = min(cols_for_pca, (int)global_lipidome->lipids.size() - 1);
    
    // perform the principal component analysis
    if (!progress || !progress->stop_progress){
        // set the step size for the next analyses
        if (progress){
            progress->prepare_steps(7);
            progress->connect(&global_lipidome->m, SIGNAL(set_step()), progress, SLOT(set_step()));
        }
        Matrix pca;
        global_lipidome->m.PCA(pca, cols_for_pca);
        global_lipidome->m.rewrite(pca);
        if (progress){
            progress->disconnect(&global_lipidome->m, SIGNAL(set_step()), 0, 0);
        }
    }
        
    
    
    
    // cutting the global PCA matrix back to a matrix for each lipidome
    if (!progress || !progress->stop_progress){
        separate_matrixes();
        normalize_intensities();
        if (progress){
            progress->set_step();
        }
    }
    
    if (!progress || !progress->stop_progress){
        compute_hausdorff_matrix();
        if (progress){
            progress->set_step();
        }
    }
    
    if (!progress || !progress->stop_progress){
        create_dendrogram();
        if (progress){
            progress->set_step();
        }
    }
    
    if (progress){
        progress->finish();
    }
    analysis_finished = true;
    
    
    
    
    
    /*
    // experimental part
    Matrix aa(global_lipidome->m.rows, lipidomes.size());
    map<string, int> species_to_index;
    for (int i = 0; i < (int)global_lipidome->species.size(); ++i){
        species_to_index.insert({global_lipidome->species[i], i});
    }
    
    for (int c = 0; c < (int)lipidomes.size(); c++){
        auto lipidome = lipidomes[c];
        for (int r = 0; r < (int)lipidome->species.size(); ++r){
            aa(species_to_index[lipidome->species[r]], c) = lipidome->intensities(r);
        }
    }
    {
        Matrix p;
        //aa.transpose();
        aa.PCA(p, 2);
        aa.rewrite(p);
    }
    
    vector<double*> lst;
    for (int r = 0; r < aa.rows; ++r){
        lst.push_back(new double[2]{sq(aa(r, 0)) + sq(aa(r, 1)), (double)r});
    }
    sort(lst.begin(), lst.end(), mysort);
    for (auto row : lst){
        cout << row[0] << " " << global_lipidome->species[(int)row[1]] << endl;
        //cout << row[0] << " " << lipidomes[(int)row[1]]->file_name << endl;
    }
    */
}



void LipidSpace::reset_analysis(){
    analysis_finished = false;
    
    for (auto lipid : all_lipids) delete lipid;
    all_lipids.clear();
    
    for (auto lipidome : lipidomes) delete lipidome;
    lipidomes.clear();
    
    global_lipidome->species.clear();
    global_lipidome->classes.clear();
    global_lipidome->lipids.clear();
    global_lipidome->intensities.clear();
    global_lipidome->m.reset(1, 1);
}



