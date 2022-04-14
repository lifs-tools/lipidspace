#include "lipidspace/lipidspace.h"
 
 

double linkage(DendrogramNode* n1, DendrogramNode* n2, Matrix &m, Linkage linkage = CompleteLinkage){
    double v = 1e100 * (linkage == SingleLinkage);
    if (linkage == SingleLinkage || linkage == CompleteLinkage){
        for (auto index1 : n1->indexes){
            for (auto index2 : n2->indexes){
                if ((linkage == CompleteLinkage && v < m(index1, index2)) || (linkage == SingleLinkage && v > m(index1, index2))){
                    v = m(index1, index2);
                }
            }
        }
    }
    else {
        for (auto index1 : n1->indexes){
            for (auto index2 : n2->indexes){
                v += m(index1, index2);
            }
        }
        v /= (double)(n1->indexes.size() * n2->indexes.size());
    }
    return v;
}



bool sort_double_string_desc (pair<double, string> i, pair<double, string> j) { return (i.first > j.first); }

void LipidSpace::create_dendrogram(){
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    lipid_sortings.clear();
    
    int n = hausdorff_distances.rows;
    
    vector<DendrogramNode*> nodes;
    for (int i = 0; i < n; ++i) nodes.push_back(new DendrogramNode(i, &feature_values, selected_lipidomes[i]));
    
    while (nodes.size() > 1){
        double min_val = 1e9;
        int ii = 0, jj = 0;
        for (int i = 0; i < (int)nodes.size() - 1; ++i){
            for (int j = i + 1; j < (int)nodes.size(); ++j){
                double val = linkage(nodes.at(i), nodes.at(j), hausdorff_distances, GlobalData::linkage);
                if (min_val > val){
                    min_val = val;
                    ii = i;
                    jj = j;
                }
            }
        }
        
        DendrogramNode* node1 = nodes.at(ii);
        DendrogramNode* node2 = nodes.at(jj);
        nodes.erase(nodes.begin() + jj);
        nodes.erase(nodes.begin() + ii);
        nodes.push_back(new DendrogramNode(node1, node2, min_val));
    }
    
    double* ret = nodes.front()->execute(0, &dendrogram_points, &dendrogram_sorting);
    delete []ret;
    dendrogram_root = nodes.front();
    
    /*
    
    // compute importance of lipids using coefficient of variation and
    // Kolmogorv Smirnoff test on dendrogram branches
    map<string, int> species_to_index;
    Matrix matrix(global_lipidome->m.rows, selected_lipidomes.size());
    for (int i = 0; i < (int)global_lipidome->species.size(); ++i){
        species_to_index.insert({global_lipidome->species[i], i});
    }
    
    vector<pair<double, string>> CVs(global_lipidome->m.rows, pair<double, string>());
    vector<string> missing;
    for (int c = 0; c < (int)selected_lipidomes.size(); c++){
        auto lipidome = selected_lipidomes[c];
        for (int r = 0; r < (int)lipidome->species.size(); ++r){
            string species = lipidome->species[r];
            if (contains_val(species_to_index, species)){
                matrix(species_to_index[species], c) = lipidome->original_intensities(r);
                CVs[species_to_index[species]].second = species;
            }
            else {
                missing.push_back(species);
            }
        }
    }
    
    for (int r = 0; r < matrix.rows; ++r){
        double mean = 0, stdev = 0;
        for (int c = 0; c < matrix.cols; c++) mean += matrix(r, c);
        mean /= (double)matrix.cols;
        
        for (int c = 0; c < matrix.cols; c++) stdev += sq(mean - matrix(r, c));
        stdev = sqrt(stdev / (double)matrix.cols);
        CVs[r].first = stdev / mean;
    }
    
    sort(CVs.begin(), CVs.end(), sort_double_string_desc);
    lipid_sortings.insert({"Coefficient of Variation (highest)", vector<string>()});
    lipid_sortings.insert({"Coefficient of Variation (lowest)", vector<string>()});
    vector<string> &CVh = lipid_sortings["Coefficient of Variation (highest)"];
    vector<string> &CVl = lipid_sortings["Coefficient of Variation (lowest)"];
    for (auto cv : CVs) CVh.push_back(cv.second);
    for (auto miss : missing) CVh.push_back(miss);
    for (int i = (int)CVs.size() - 1; i >= 0; --i) CVl.push_back(CVh[i]);
    
    
    
    vector<pair<double, string>> top_two_groups;
    vector<double> p_values;
    vector<double> fold_change;
    double max_log_fc = 0;
    for (int r = 0; r < matrix.rows; ++r){
        Array sample1;
        Array sample2;
        for (int c1 : dendrogram_root->left_child->indexes){
            if (matrix(r, c1) > 0) sample1.push_back(matrix(r, c1));
        }
        for (int c2 : dendrogram_root->right_child->indexes){
            if (matrix(r, c2) > 0) sample2.push_back(matrix(r, c2));
        }
        
        if (sample1.size() < 2 || sample2.size() < 2){
            missing.push_back(global_lipidome->species[r]);
            continue;
        }
        
        top_two_groups.push_back({0, global_lipidome->species[r]});
        p_values.push_back(KS_pvalue(sample1, sample2));
        fold_change.push_back(fabs(log(sample2.mean() / sample1.mean())));
        max_log_fc = max(max_log_fc, fold_change.back());
    }
    
    BH_fdr(p_values);
    //for (int i = 0; i < (int)p_values.size(); ++i) top_two_groups[i].first = sqrt((1 - p_values[i]) * fold_change[i] / max_log_fc);
    for (int i = 0; i < (int)p_values.size(); ++i) top_two_groups[i].first = (1 - p_values[i]) * fold_change[i];
    
    sort(top_two_groups.begin(), top_two_groups.end(), sort_double_string_desc);
    
    lipid_sortings.insert({"Top 2 groups (highest)", vector<string>()});
    lipid_sortings.insert({"Top 2 groups (lowest)", vector<string>()});
    vector<string> &top_highest = lipid_sortings["Top 2 groups (highest)"];
    vector<string> &top_lowest = lipid_sortings["Top 2 groups (lowest)"];
    for (auto kv : top_two_groups) top_highest.push_back(kv.second);
    for (auto miss : missing) top_highest.push_back(miss);
    for (int i = (int)top_highest.size() - 1; i >= 0; --i) top_lowest.push_back(top_highest[i]);
    */
}




LipidSpace::LipidSpace() {
    keep_sn_position = true;
    ignore_unknown_lipids = false;
    ignore_doublette_lipids = false;
    unboundend_distance = false;
    without_quant = false;
    global_lipidome = new Table("global_lipidome");
    progress = 0;
    analysis_finished = false;
    dendrogram_root = 0;
    progress = 0;
        
    // load precomputed class distance matrix
    
    
    ifstream infile(QCoreApplication::applicationDirPath().toStdString() + "/data/classes-matrix.csv");
    if (!infile.good()){
        Logging::write_log("Error: file 'data/classes-matrix.csv' not found.");
        throw LipidException("Error: file 'data/classes-matrix.csv' not found. Please check the log message.");
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

const int LipidSpace::cols_for_pca_init = 7;
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
    if (dendrogram_root) delete dendrogram_root;
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
    
    
    int db1 = 0, db2 = 0;
    if (bond_types.size() == 1){ // both equal
        if (contains_val(bond_types, ESTER)){
            inter_num += 3; // two oxygens, one double bond
            union_num += 3; // two oxygens, one double bond
        }
        if (contains_val(bond_types, ETHER_PLASMANYL) || contains_val(bond_types, ETHER_PLASMENYL)){
            inter_num += 1; // one oxygen
            union_num += 1; // one oxygen
        }
    }
    
    else if (contains_val(bond_types, ESTER)){
        if (contains_val(bond_types, ETHER_PLASMANYL)){
            inter_num += 1; // one oxygen
            union_num += 3; // two oxygens, one double bond
            if (fa1->lipid_FA_bond_type == ETHER_PLASMANYL) db1 += 1;
            else db2 += 1;
        }
        
        else if (contains_val(bond_types, ETHER_PLASMENYL)){
            inter_num += 1; // one oxygen
            union_num += 3; // two oxygens, one double bond
        }
        
        else if (contains_val(bond_types, LCB_REGULAR) || contains_val(bond_types, LCB_EXCEPTION)){
            union_num += 2; // one oxygen, one double bond
        }
    }
    
    else if (contains_val(bond_types, LCB_EXCEPTION) || contains_val(bond_types, LCB_REGULAR)){
        if (contains_val(bond_types, ETHER_PLASMENYL)){
            union_num += 1;
        }
    }
        
    else if (contains_val(bond_types, ETHER_PLASMENYL) && contains_val(bond_types, ETHER_PLASMANYL)){
        inter_num += 1; // one oxygen
        union_num += 1; // one oxygen
        if (fa1->lipid_FA_bond_type == ETHER_PLASMANYL) db1 += 1;
        else db2 += 1;
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
    int common = 0;
    if (fa1->double_bonds->double_bond_positions.size() > 0 && fa2->double_bonds->double_bond_positions.size() > 0){
        for (auto kv : fa1->double_bonds->double_bond_positions){
            common += contains_val(fa2->double_bonds->double_bond_positions, kv.first) && fa2->double_bonds->double_bond_positions.at(kv.first) == kv.second;
        }
        inter_num += common;
        union_num += fa1->double_bonds->get_num() + fa2->double_bonds->get_num() - common;
        db1 += fa1->double_bonds->get_num();
        db2 += fa2->double_bonds->get_num();
    }
    else {
        db1 += fa1->double_bonds->get_num();
        db2 += fa2->double_bonds->get_num();
            
        // old strategy of taking the minimum as intersect
        // does not go figure with database observations
        //inter_num += mmin(db1, db2);
        //union_num += mmax(db1, db2);
        
        // When considering two random fatty acyl chains with
        // at least one double bond, the probability to have
        // no DB match at all is about 85%. Therefore, no
        // intersection, just union with only mismatches
        union_num += db1 + db2;
    }
    
    // add all single bonds
    inter_num += mmin(fa1->num_carbon - m1, fa2->num_carbon - m1) - (db1 + db2 - common);
    union_num += mmax(fa1->num_carbon - m2, fa2->num_carbon - m2) - (db1 + db2 - common);
    
    
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
    
    if (uncontains_val(class_matrix, key)){
        throw LipidSpaceException("Error: key '" + key + "' not in precomputed class matrix. Please check the log message.", LipidNotRegistered);
    }
    union_num = class_matrix.at(key)[0];
    inter_num = class_matrix.at(key)[1];
    
    if (lipid1->lipid->info->level <= CLASS || lipid1->lipid->info->level <= CLASS) return;
    
    vector<FattyAcid*>* orig_fa_list_1 = new vector<FattyAcid*>();
    if (lipid1->lipid->info->level >= MOLECULAR_SPECIES){
        for (auto fa : lipid1->lipid->fa_list){
            if (fa->num_carbon > 0)
                orig_fa_list_1->push_back(fa);
        }
    }
    else {
        orig_fa_list_1->push_back(lipid1->lipid->info);
    }
    vector<FattyAcid*>* orig_fa_list_2 = new vector<FattyAcid*>();
    if (lipid2->lipid->info->level >= MOLECULAR_SPECIES){
        for (auto fa : lipid2->lipid->fa_list){
            if (fa->num_carbon > 0)
                orig_fa_list_2->push_back(fa);
            
        }
    }
    else {
        orig_fa_list_2->push_back(lipid2->lipid->info);
    }
    
    if (orig_fa_list_1->size() < orig_fa_list_2->size()){
        vector<FattyAcid*>* tmp = orig_fa_list_1;
        orig_fa_list_1 = orig_fa_list_2;
        orig_fa_list_2 = tmp;
    }
    int len_fa1 = orig_fa_list_1->size();
    int len_fa2 = orig_fa_list_2->size();
    
    if (len_fa1 == 0 && len_fa2 == 0){
        delete orig_fa_list_1;
        delete orig_fa_list_2;
        return;
    }
    
    
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
    
    delete orig_fa_list_1;
    delete orig_fa_list_2;
}



void LipidSpace::reassembleSelection(){
    set<string> new_selection[4];
    
    // setup new set of entities
    for (auto lipidome : lipidomes){
        new_selection[3].insert(lipidome->cleaned_name);
        for (int i = 0; i < (int)lipidome->lipids.size(); ++i){
            new_selection[0].insert(lipidome->species.at(i));
            new_selection[1].insert(lipidome->classes.at(i));
            new_selection[2].insert(lipidome->categories.at(i));
        }
    }
    
    // copy all selections into a tmp map,
    // clear original selection,
    // copy back only remaining entities
    for (int i = 0; i < 4; ++i){
        map<string, bool> tmp_selection;
        for (auto kv : selection[i]) tmp_selection.insert({kv.first, kv.second});
        selection[i].clear();
        
        for (auto kv : tmp_selection){
            if (contains_val(new_selection[i], kv.first)){
                selection[i].insert({kv.first, kv.second});
            }
        }
    }
    
    
    
    
    
    // setup a complete list of all nominal features and their values
    map<string, set<string>> delete_nominal_features;
    for (auto kv : feature_values){
        if (kv.second.feature_type == NominalFeature){
            if (uncontains_val(delete_nominal_features, kv.first)) delete_nominal_features.insert({kv.first, set<string>()});
            for (auto kv_nom : kv.second.nominal_values){
                delete_nominal_features[kv.first].insert(kv_nom.first);
            }
        }
    }
    
    // remove all features and values that remain in the analysis
    for (auto lipidome : lipidomes){
        for (auto kv : lipidome->features){
            if (contains_val(delete_nominal_features, kv.first) && kv.second.feature_type == NominalFeature){
                delete_nominal_features[kv.first].erase(kv.second.nominal_value);
            }
        }
    }
    
    // delete remaining feature values or even whole features
    for (auto kv : delete_nominal_features){
        if (kv.second.size() == feature_values[kv.first].nominal_values.size()){ // delete whole feature
            feature_values.erase(kv.first);
        }
        else { // delete just the values
            for (auto feature_value : delete_nominal_features[kv.first]){
                feature_values[kv.first].nominal_values.erase(feature_value);
            }
        }
    }
    
    
    reassembled();
}




void LipidSpace::load_list(string lipid_list_file){
    
    // load and parse lipids
    ifstream infile(lipid_list_file);
    if (!infile.good()){
        LipidSpaceException("Error: file '" + lipid_list_file + "' not found.", FileUnreadable);
    }
    string line;
    
    
    Table* lipidome = new Table(lipid_list_file);
    selection[3].insert({lipidome->cleaned_name, true});
    vector<string> lipids;
    
    int pos_all = all_lipids.size();
    set<string> registered_lipids;
    while (getline(infile, line)){
        if (line.length() == 0) continue;
        
        vector<string>* tokens = goslin::split_string(line, '\n', '"');
        line = strip(tokens->at(0), '"');
        delete tokens;
        
        
        if (line.find('\t') != string::npos){
            tokens = goslin::split_string(line, '\t', '"');
            line = strip(tokens->at(0), '"');
            lipidome->original_intensities.push_back(atof(tokens->at(1).c_str()));
            delete tokens;
        }
        else {
            lipidome->original_intensities.push_back(1);
        }
        lipids.push_back(line);
        lipidome->lipids.push_back(0);
        all_lipids.push_back(0);
        lipidome->species.push_back("");
        lipidome->classes.push_back("");
        lipidome->categories.push_back("");
    }
    
    vector<int> remove;
    
    for (int i = 0; i < (int)lipids.size(); ++i) { 
        string line = lipids.at(i);
        try {
            LipidAdduct* l = parser.parse_parallel(line);
            // deleting adduct since not necessary
            if (l->adduct != 0){
                delete l->adduct;
                l->adduct = 0;
            }
            l->sort_fatty_acyl_chains();
            
            all_lipids.at(pos_all + i) = l;
            lipidome->lipids.at(i) = l;
            lipidome->species.at(i) = l->get_lipid_string();
            lipidome->classes.at(i) = l->get_lipid_string(CLASS);
            lipidome->categories.at(i) = l->get_lipid_string(CATEGORY);
            
            if (uncontains_val(registered_lipids, lipidome->species.at(i))){
                registered_lipids.insert(lipidome->species.at(i));
            }
            else {
                if (!ignore_doublette_lipids){
                    throw LipidSpaceException("Lipid '" + line + "' appears twice in the list.", LipidDoublette);
                }
                else {
                    Logging::write_log("Ignoring doublette lipid '" + line + "'");
                    remove.push_back(i);
                }
            }
        }
        catch (LipidException &e) {
            if (!ignore_unknown_lipids){
                throw LipidSpaceException(string(e.what()) + ": lipid '" + line + "' cannot be parsed.", LipidUnparsable);
            }
            else {
                Logging::write_log("Ignoring unidentifiable lipid '" + line + "'");
                remove.push_back(i);
            }
        }
    }
    
    if (remove.size() > 0){
        sort(remove.begin(), remove.end());
        for (int i = remove.size() - 1; i >= 0; --i){
            int index = remove.at(i);
            all_lipids.erase(all_lipids.begin() + pos_all + index);
            lipidome->lipids.erase(lipidome->lipids.begin() + index);
            lipidome->species.erase(lipidome->species.begin() + index);
            lipidome->classes.erase(lipidome->classes.begin() + index);
            lipidome->categories.erase(lipidome->categories.begin() + index);
            lipidome->original_intensities.erase(lipidome->original_intensities.begin() + index);
        }
    }
    
    for (int i = 0; i < (int)lipidome->species.size(); ++i){
        selection[0].insert({lipidome->species.at(i), true});
        selection[1].insert({lipidome->classes.at(i), true});
        selection[2].insert({lipidome->categories.at(i), true});
    }
    for (auto l : lipidome->lipids){
        for (auto fa : l->lipid->fa_list){
            cut_cycle(fa);
        }
    }
    
    lipidomes.push_back(lipidome);
    fileLoaded();
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







double LipidSpace::compute_hausdorff_distance(Matrix &m1, Matrix &m2){
    assert(m1.rows == m2.rows);
    double max_h = 0;
    
    for (int m1c = 0; m1c < m1.cols; ++m1c){
        double min_h = 1e100;
        double* m1col = m1.data() + (m1c * m1.rows);
        for (int m2c = 0; m2c < m2.cols; m2c++){
            __m256d dist = {0, 0, 0, 0};
            double* m2col = m2.data() + (m2c * m2.rows);
            for (int r = 0; r < m1.rows; r += 4){
                __m256d val1 = _mm256_loadu_pd(&m1col[r]);
                __m256d val2 = _mm256_loadu_pd(&m2col[r]);
                __m256d sub = _mm256_sub_pd(val1, val2);
                dist = _mm256_fmadd_pd(sub, sub, dist);
            }
            double distance = dist[0] + dist[1] + dist[2] + dist[3];
            min_h = min(min_h, distance);
            if (min_h < max_h || min_h == 0) break;
        }
        max_h = max(max_h, min_h);
    }
    
    
    for (int m2c = 0; m2c < m2.cols; m2c++){
        double min_h = 1e100;
        double* m2col = m2.data() + (m2c * m2.rows);
        for (int m1c = 0; m1c < m1.cols; ++m1c){
            __m256d dist = {0, 0, 0, 0};
            double* m1col = m1.data() + (m1c * m1.rows);
            for (int r = 0; r < m1.rows; r += 4){
                __m256d val1 = _mm256_loadu_pd(&m1col[r]);
                __m256d val2 = _mm256_loadu_pd(&m2col[r]);
                __m256d sub = _mm256_sub_pd(val1, val2);
                dist = _mm256_fmadd_pd(sub, sub, dist);
            }
            double distance = dist[0] + dist[1] + dist[2] + dist[3];
            min_h = min(min_h, distance);
            if (min_h < max_h || min_h == 0) break;
        }
        max_h = max(max_h, min_h);
    }
    
    return sqrt(max_h);
}





void LipidSpace::compute_hausdorff_matrix(){
    
    int n = selected_lipidomes.size();
    vector<Matrix*> matrixes;
    
    for (auto lipidome : selected_lipidomes){
        matrixes.push_back(new Matrix(lipidome->m));
        // add intensities to hausdorff matrix
        if (!without_quant){
            matrixes.back()->add_column(lipidome->PCA_intensities);
        }
        matrixes.back()->pad_cols_4(); // pad matrix columns with zeros to perform faster intrinsics
        matrixes.back()->transpose();
    }
    
    hausdorff_distances.reset(n, n);
    
    // precompute indexes for of symmetric matrix for faster
    // and equally distributed access 
    vector<pair<int, int>> pairs;
    for (int ii = 0; ii < sq(n); ++ii){
        int i = ii / n;
        int j = ii % n;
        if (i < j){
            pairs.push_back({i, j});
        }
    }
    
    #pragma omp parallel for
    for (int ii = 0; ii < (int)pairs.size(); ++ii){
        int i = pairs.at(ii).first;
        int j = pairs.at(ii).second;
        
        double hd = compute_hausdorff_distance(*matrixes.at(i), *matrixes.at(j));
        hausdorff_distances(i, j) = hd;
        hausdorff_distances(j, i) = hd;
    }
    

    for (auto matrix : matrixes) delete matrix;
}



void LipidSpace::store_results(string output_folder){
    report_hausdorff_matrix(output_folder);
    store_distance_table(output_folder);
}



void LipidSpace::report_hausdorff_matrix(string output_folder){
    ofstream off(output_folder + "/hausdorff_distances.csv");
    int n = hausdorff_distances.rows;
    off << "ID";
    for (int i = 0; i < n; ++i){
        off << "\t" << selected_lipidomes[i]->cleaned_name;
    } off << endl;
    
    for (int i = 0; i < n; ++i){
        off << selected_lipidomes[i]->cleaned_name;
        for (int j = 0; j < n; ++j){
            off << "\t" << hausdorff_distances(i, j);
        } off << endl;
    }
}




bool LipidSpace::compute_global_distance_matrix(){
    global_lipidome->species.clear();
    global_lipidome->classes.clear();
    global_lipidome->categories.clear();
    global_lipidome->lipids.clear();
    global_lipidome->intensities.clear();
    global_lipidome->PCA_intensities.clear();
    global_lipidome->m.reset(1, 1);
    

    
    set<string> registered_lipids;
    
    for (auto lipidome : lipidomes){
        // check if lipidome was (de)selected for analysis
        if (!selection[SAMPLE_ITEM][lipidome->cleaned_name]) continue;
        // check if lipidome is being excluded by deselected feature(s)
        bool filtered_out = false;
        for (auto kv : lipidome->features){
            if (kv.second.feature_type == NominalFeature){
                if (uncontains_val(feature_values, kv.first) || uncontains_val(feature_values[kv.first].nominal_values, kv.second.nominal_value) || !feature_values[kv.first].nominal_values[kv.second.nominal_value]) {
                    filtered_out = true;
                    break;
                }
            }
            else {
                if (uncontains_val(feature_values, kv.first)) continue;
                vector<double> &filter_values = feature_values[kv.first].numerical_filter.second;
                double numerical_value = kv.second.numerical_value;
                switch (feature_values[kv.first].numerical_filter.first){
                    
                    case LessFilter: // inverse logic, Less tells us, what to keep, greater we filter out
                        if (filter_values.size() < 1) continue;
                        if (filter_values[0] < numerical_value) filtered_out = true;
                        break;
                        
                    case GreaterFilter:
                        if (filter_values.size() < 1) continue;
                        if (filter_values[0] > numerical_value) filtered_out = true;
                        break;
                        
                    case EqualFilter:
                        if (filter_values.size() < 1) continue;
                        filtered_out = true; // < 1e-16 due to floating point rounding errors
                        for (double filter_value : filter_values) filtered_out &= !(fabs(numerical_value - filter_value) < 1e-16);
                        break;
                    
                    case WithinRange:
                        if (filter_values.size() < 2) continue;
                        if (numerical_value < filter_values[0] || filter_values[1] < numerical_value) filtered_out = true;
                        break;
                    
                    case OutsideRange:
                        if (filter_values.size() < 2) continue;
                        if (filter_values[0] <= numerical_value && numerical_value <= filter_values[1]) filtered_out = true;
                        break;
                        
                    default:
                        break;
                }
            }
        }
        if (filtered_out) continue;
            
        selected_lipidomes.push_back(lipidome);
            
        for (int i = 0; i < (int)lipidome->species.size(); ++i){
            string lipid_species = lipidome->species.at(i);
            string lipid_class = lipidome->classes.at(i);
            string lipid_category = lipidome->categories.at(i);
            
            // check if entities already exist in their corresponding sets,
            // check if entity is selected and add them if it is
            if (contains_val(registered_lipids, lipid_species)) continue;
            if (!selection[SPECIES_ITEM][lipid_species]) continue;
            if (!selection[CLASS_ITEM][lipid_class]) continue;
            if (!selection[CATEGORY_ITEM][lipid_category]) continue;
            registered_lipids.insert(lipid_species);
            
            // add lipid data to global lipidome
            global_lipidome->species.push_back(lipid_species);
            global_lipidome->classes.push_back(lipidome->classes.at(i));
            global_lipidome->categories.push_back(lipidome->categories.at(i));
            global_lipidome->lipids.push_back(lipidome->lipids.at(i));
        }
    }
    
    // set equal intensities, later important for ploting
    int n = global_lipidome->lipids.size();
    if (n < 3) return false;

    
    global_lipidome->intensities.resize(n);
    global_lipidome->PCA_intensities.resize(n);
    for (int i = 0; i < n; ++i) global_lipidome->intensities[i] = 1;
    for (int i = 0; i < n; ++i) global_lipidome->PCA_intensities[i] = 1;
    
    // compute distances
    Matrix& distance_matrix = global_lipidome->m;
    distance_matrix.reset(n, n);
    
    
    double total_num = (n * (n - 1)) >> 1;
    double next_tp = 1;
    
    if (progress){
        progress->max_progress = (n * (n - 1));
        progress->set_max(progress->max_progress);
    }    
    
    #pragma omp parallel for
    for (int ii = 0; ii < n * n; ++ii){
        int i = ii / n;
        int j = ii % n;
        
        if (i < j){
            
            bool go_on = true;
            if (progress){
                #pragma omp critical
                {
                    go_on = !progress->stop_progress;
                    if (++progress->current_progress / total_num * 100. >= next_tp){
                        progress->set(progress->current_progress);
                        next_tp += 1;
                    }
                }
            }
            if (go_on){
                int union_num = 0, inter_num = 0;
                lipid_similarity(global_lipidome->lipids.at(i), global_lipidome->lipids.at(j), union_num, inter_num);
                
                if (union_num < 0 || inter_num < 0){
                    throw LipidSpaceException("Error on maximum common subgraph for lipid '" + global_lipidome->lipids.at(i)->get_lipid_string() + "' and '" + global_lipidome->lipids.at(j)->get_lipid_string() + "', union: " + std::to_string(union_num) + ", inter: " + std::to_string(inter_num) + ". Please contact the developers!", UnspecificException);
                }
                double distance = unboundend_distance ?
                    ((double)union_num / (double)inter_num - 1.) // unboundend distance [0; inf[
                                :
                    (1. - (double)inter_num / (double)union_num); // bounded distance [0; 1]
                
                distance_matrix(i, j) = distance;
                distance_matrix(j, i) = distance;
            }
        }
    }
    return true;
}









void LipidSpace::separate_matrixes(){
    for (auto lipidome : lipidomes){
        lipidome->intensities.clear();
        lipidome->PCA_intensities.clear();
        lipidome->selection.clear();
    }
    global_lipidome->selection.clear();
    
    map<string, int> lipid_indexes;
    for (int i = 0; i < (int)global_lipidome->species.size(); ++i){
        string lipid_species = global_lipidome->species.at(i);
        lipid_indexes.insert({lipid_species, i});
        global_lipidome->selection.push_back(true);
    }
    
    vector<int> remove_lipidomes;
    int l = 0;
    for (auto lipidome : selected_lipidomes){
        Indexes indexes;
        for (int i = 0; i < (int)lipidome->species.size(); ++i){
            string lipid_species = lipidome->species[i];
            bool lipid_selection = selection[SPECIES_ITEM][lipid_species] && contains_val(lipid_indexes, lipid_species);
            lipidome->selection.push_back(lipid_selection);
            if (lipid_selection){
                indexes.push_back(lipid_indexes.at(lipid_species));
                lipidome->intensities.push_back(lipidome->original_intensities[i]);
                lipidome->PCA_intensities.push_back(lipidome->original_intensities[i]);
            }
        }
        if (indexes.size() > 0){
            lipidome->m.rewrite(global_lipidome->m, indexes);
        }
        else {
            remove_lipidomes.push_back(l);
        }
        l++;
    }
    for (int i = (int)remove_lipidomes.size() - 1; i >= 0; --i) selected_lipidomes.erase(selected_lipidomes.begin() + remove_lipidomes[i]);
}




inline double gauss(double x, double mue, double sigma){
    return exp(-0.5 * sq((x - mue) / sigma)) / sqrt(2 * M_PI);
}



void LipidSpace::normalize_intensities(){
    
    int n = global_lipidome->m.rows;
    double global_stdev = 0, global_mean = 0;
    for (int i = 0; i < n; ++i) global_mean += global_lipidome->m.m[i];
    global_mean /= (double)n;
    for (int i = 0; i < n; ++i) global_stdev += sq(global_lipidome->m.m[i] - global_mean);
    global_stdev = sqrt(global_stdev / (double)n);
    
    
    if (GlobalData::normalization == "absolute" || GlobalData::normalization == ""){
        Array data;
        for (auto lipidome : selected_lipidomes){
            for (auto val : lipidome->original_intensities) {
                if (val > 0) data.push_back(val);
            }
        }
        double values_mean = data.mean();
        double values_std = data.stdev();
        if (values_std < 1e-19) return;
        
        for (auto lipidome : selected_lipidomes){

            for (int i = 0; i < (int)lipidome->intensities.size(); ++i){
                lipidome->PCA_intensities[i] = global_stdev * (lipidome->PCA_intensities[i] - values_mean) / values_std;
                lipidome->intensities[i] = 100. * lipidome->intensities[i] / values_std;
            }
        }
    }
    else if (GlobalData::normalization == "relative") {
        for (auto lipidome : selected_lipidomes){
                
            Array data;
            for (auto val : lipidome->original_intensities) {
                if (val > 0) data.push_back(val);
            }
            double values_mean = data.mean();
            double values_std = data.stdev();
            if (values_std < 1e-19) continue;
            
            
            for (int i = 0; i < (int)lipidome->intensities.size(); ++i){
                lipidome->PCA_intensities[i] = global_stdev * (lipidome->PCA_intensities[i] - values_mean) / values_std;
                lipidome->intensities[i] = 100. * lipidome->intensities[i] / values_std;
            }
        }
    }
    
    else {
        string fv = GlobalData::normalization;
        if (uncontains_val(feature_values, fv)) return;
        FeatureSet &fs = feature_values.at(fv);
        for (auto kv : fs.nominal_values){
            Array data;
            for (auto lipidome : selected_lipidomes){
                if (uncontains_val(lipidome->features, fv) || lipidome->features.at(fv).nominal_value != kv.first) continue;
                for (auto val : lipidome->original_intensities) {
                    if (val > 0) data.push_back(val);
                }
            }
            
            double values_mean = data.mean();
            double values_std = data.stdev();
            if (values_std < 1e-19) continue;
                
            for (auto lipidome : selected_lipidomes){
                if (uncontains_val(lipidome->features, fv) || lipidome->features.at(fv).nominal_value != kv.first) continue;
                for (int i = 0; i < (int)lipidome->intensities.size(); ++i){
                    lipidome->PCA_intensities[i] = global_stdev * (lipidome->PCA_intensities[i] - values_mean) / values_std;
                    lipidome->intensities[i] = 100. * lipidome->intensities[i] / values_std;
                }
            }
        }
    }
    
    
    /*
    for (auto lipidome : selected_lipidomes){
        // compute the standard deviation of the first principal component,
        // mean of all PCs should be always 0 per definition
        int n = lipidome->m.rows;
        double global_stdev = 0, global_mean = 0;
        for (int i = 0; i < n; ++i) global_mean += lipidome->m.m[i];
        global_mean /= (double)n;
        for (int i = 0; i < n; ++i) global_stdev += sq(lipidome->m.m[i] - global_mean);
        global_stdev = sqrt(global_stdev / (double)n);
        
        
        // Use expectation maximization algorithm to identify outliers
        // for a more stable estimation of the standard deviation
        Array data;
        for (auto val : lipidome->intensities) {
            if (val > 0) data.push_back(val);
        }
        int l = data.size();
        double *X = new double[l];
        double *w = new double[l];
        double min_v = 1e90;
        double max_v = 0;
        for (int i = 0; i < l; ++i){
            X[i] = log(data.at(i));
            min_v = min(min_v, X[i]);
            max_v = max(max_v, X[i]);
        }
        double mue = log(data.median());
        double sigma = 1;
        double weight = 0.99;
        double bgm = 1. / (max_v - min_v); // background model
        
        double old_mue = 0, old_sigma = 0;
        for (int rep = 0; rep < 1000; ++rep){
            old_mue = mue;
            old_sigma = sigma;
            
            // expectation step
            double W = 0, W2 = 0;
            double gbg = bgm * (1. - weight);
            for (int i = 0; i < l; ++i){
                double g = weight * gauss(X[i], mue, sigma);
                w[i] = g / (g + bgm * (1. - weight));
                W += w[i];
                W2 += gbg / (g + gbg);
            }
            
            // maximization step
            // maximizing weight
            weight = W / (W + W2);
            
            // maximizing mean value mue
            mue = 0;
            for (int i = 0; i < l; ++i) mue += w[i] * X[i];
            mue /= W;
            
            // maximizing standard deviation sigma
            sigma = 0;
            for (int i = 0; i < l; ++i) sigma += w[i] * sq(X[i] - mue);
            sigma = sqrt(sigma / W);
            
            // check if values converge
            bool halt = false;
            halt |= fabs(mue) / fabs(max(mue, old_mue)) < 0.001;
            halt |= fabs(sigma) / fabs(max(sigma, old_sigma)) < 0.001;
            if (halt) break;
        }
        
        Array mod_intens;
        for (int i = 0; i < l; ++i) mod_intens.push_back(data[i] * w[i]);
        
        double mn = mod_intens.mean();
        double stdev = mod_intens.stdev();
        cout << lipidome->cleaned_name << " " << global_stdev << " " << stdev << endl;
        if (stdev > 0){
            for (int i = 0; i < l; ++i) lipidome->intensities[i] = global_stdev * (lipidome->intensities[i] - mn) / stdev;
        }
        
        delete []X;
        delete []w;
    }
    */
}


inline int LipidSpace::extract_number(string line, int line_number){
    unsigned long start_sample_num = line.find("[");
    unsigned long end_sample_num = line.find("]");
    if ((start_sample_num == std::string::npos) || (end_sample_num == std::string::npos) || (end_sample_num <= start_sample_num)){
        throw LipidSpaceException("Error in line " + std::to_string(line_number) + ", line is corrupted.", CorruptedFileFormat);
    }
    start_sample_num += 1;
    return atoi(line.substr(start_sample_num, end_sample_num - start_sample_num).c_str());
}


void LipidSpace::load_mzTabM(string mzTabM_file){
    Logging::write_log("Importing table '" + mzTabM_file + "' as pivot table.");
    
    ifstream infile(mzTabM_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + mzTabM_file + "' could not be found.", FileUnreadable);
    }
    vector<Table*> loaded_lipidomes;
    map<int, string> feature_names;
    
    vector<string> *tokens = 0;
    vector<string> *sample_data = 0;
    vector<string> *content_tokens = 0;
    try {
        // go through file
        string line;
        int line_num = 0;
        while (getline(infile, line)){
            ++line_num;
            if (line.length() == 0) continue;
            
            vector<string> *tokens = split_string(line, '\t', '"', true);
            for (int i = 0; i < (int)tokens->size(); ++i) tokens->at(i) = strip(tokens->at(i), '"');
            
            if (tokens->size() == 0){
                delete tokens;
                tokens = 0;
                continue;
            }
            
            // found a sample information line
            if (tokens->size() >= 3 && tokens->at(0) == "MTD" && tokens->at(1).substr(0, 6) == "sample"){
                if (tokens->at(1).size() == 6){
                    throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", sample number not provided. " + line, CorruptedFileFormat);
                }
                
                if (tokens->at(1).substr(0, 7) != "sample[") continue;
                
                vector<string> *sample_data = split_string(tokens->at(1), '-', '"', true);
                
                // searching for pattern: sample[123]
                if (sample_data->size() == 1){
                    string sample_name = tokens->at(2);
                    int sample_number = extract_number(sample_data->at(0), line_num);
                    
                    if (sample_number <= 0){
                        throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", sample number must be a positive number.", CorruptedFileFormat);
                    }
                    if (sample_number > 1048576){
                        throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", sample number is exceeding 1048576.", CorruptedFileFormat);
                    }
                    
                    
                    if ((int)loaded_lipidomes.size() < sample_number){
                        loaded_lipidomes.resize(sample_number, 0);
                    }
                    if (loaded_lipidomes.at(sample_number - 1)){
                        loaded_lipidomes[sample_number - 1]->file_name = sample_name;
                        loaded_lipidomes[sample_number - 1]->cleaned_name = sample_name;
                    }
                    else {
                        loaded_lipidomes[sample_number - 1] = new Table(sample_name);
                    }
                }
                
                // search for pattern: sample[123]-content[123]
                else {
                    string sample_name = tokens->at(2);
                    int sample_number = extract_number(sample_data->at(0), line_num);
                    
                    if (sample_number <= 0){
                        throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", sample number must be a positive number.", CorruptedFileFormat);
                    }
                    if (sample_number > 1048576){
                        throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", sample number is exceeding 1048576.", CorruptedFileFormat);
                    }
                    
                    if ((int)loaded_lipidomes.size() < sample_number){
                        loaded_lipidomes.resize(sample_number, 0);
                        loaded_lipidomes[sample_number - 1] = new Table("-");
                    }
                    
                    int content_number = extract_number(sample_data->at(1), line_num);
                    string content_name = sample_data->at(1).substr(0, sample_data->at(1).find("["));
                    
                    vector<string> *content_tokens = split_string(strip(strip(strip(tokens->at(2), ' '), '['), ']'), ',', '\0', true);
                    for (uint i = 0; i < content_tokens->size(); ++i) content_tokens->at(i) = strip(content_tokens->at(i), ' ');
                    
                    if (content_name == "custom"){
                        if (content_tokens->size() < 4 || content_tokens->at(2).size() == 0 || content_tokens->at(3).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = content_tokens->at(2);
                        string value = content_tokens->at(3);
                        if (uncontains_val(feature_names, content_number)){
                            feature_names.insert({content_number, key});
                        }
                        else if (feature_names[content_number] != key){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", different study variable key than already used, '" + feature_names[content_number] + "' vs. '" + key + "'", CorruptedFileFormat);
                        }
                        
                        // is custom value a string?
                        if (key.size() >= 3 && key[0] == '"' && key[key.size() - 1] == '"'){
                            loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, strip(value, '"'))});
                        }
                        else {
                            loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, atof(value.c_str()))});
                        }
                    }
                    else if (content_name == "species"){
                        if (content_tokens->size() < 3 || content_tokens->at(2).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = "Taxonomic species";
                        string value = content_tokens->at(2);
                        loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, strip(value, '"'))});
                    }
                    else if (content_name == "tissue"){
                        if (content_tokens->size() < 3 || content_tokens->at(2).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = "Tissue";
                        string value = content_tokens->at(2);
                        loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, strip(value, '"'))});
                    }
                    
                    delete content_tokens;
                    content_tokens = 0;
                }
                
                delete sample_data;
                sample_data = 0;
            }
            
            
            delete tokens;
            tokens = 0;
        }
        
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
        }
    }
    catch(LipidSpaceException &e){
        if (tokens) delete tokens;
        if (sample_data) delete sample_data;
        if (content_tokens) delete content_tokens;
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    catch (exception &e){
        if (tokens) delete tokens;
        if (sample_data) delete sample_data;
        if (content_tokens) delete content_tokens;
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    
    
    
    fileLoaded();
}



void LipidSpace::load_pivot_table(string pivot_table_file, vector<TableColumnType> *column_types){
    Logging::write_log("Importing table '" + pivot_table_file + "' as pivot table.");
    
    ifstream infile(pivot_table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + pivot_table_file + "' could not be found.", FileUnreadable);
    }
    vector<Table*> loaded_lipidomes;
    
    try {
        string line;
        vector<int> sample_columns;
        int lipid_species_column = -1;
        int quant_column = -1;
        set<string> index_key_pairs;
        vector<int> feature_columns_numerical;
        vector<int> feature_columns_nominal;
        
        for (int i = 0; i < (int)column_types->size(); ++i) {
            switch(column_types->at(i)){
                case FeatureColumnNominal: feature_columns_nominal.push_back(i); break;
                case FeatureColumnNumerical: feature_columns_numerical.push_back(i); break;
                case SampleColumn: sample_columns.push_back(i); break;
                case LipidColumn: lipid_species_column = i; break;
                case QuantColumn: quant_column = i; break;
                default: break;
            }
        }
        
        if (sample_columns.size() == 0 || lipid_species_column == -1 || quant_column == -1){
            throw LipidSpaceException("Error while loading pivot table '" + pivot_table_file + "': not all essential columns defined.", NoColumnFound);
        }
        
        set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
        int line_cnt = 0;
        map<string, LipidAdduct*> lipid_map;
        map<string, Table*> lipidome_map;
        vector<string> feature_names_nominal;
        vector<string> feature_names_numerical;
        
        while (getline(infile, line)){
            if (line.length() == 0) continue;
            
            vector<string>* tokens = split_string(line, ',', '"', true);
            for (int i = 0; i < (int)tokens->size(); ++i) tokens->at(i) = goslin::strip(tokens->at(i), '"');
            
            if (line_cnt++ == 0){
                for (auto fi : feature_columns_nominal){
                    string feature = tokens->at(fi);
                    if (feature_values.empty() || contains_val(feature_values, feature)){
                        feature_names_nominal.push_back(feature);
                    }
                    else {
                        throw LipidSpaceException("Tables with features have already been imported, however feature '" + feature + "' is not registered. Please remove this feature or reset LipidSpace.", FeatureNotRegistered);
                    }
                }
                for (auto fi : feature_columns_numerical){
                    string feature = tokens->at(fi);
                    if (feature_values.empty() || contains_val(feature_values, feature)) {
                        feature_names_numerical.push_back(feature);
                    }
                    else {
                        throw LipidSpaceException("Tables with features have already been imported, however feature '" + feature + "' is not registered. Please remove this feature or reset LipidSpace.", FeatureNotRegistered);
                    }
                }
                
                delete tokens;
                continue;
            }
            
            
            // check if quant information is valid
            string quant_val = tokens->at(quant_column);
            if (contains_val(NA_VALUES, quant_val)){
                delete tokens;
                continue;
            }
            
            // take or create sample / lipidome table
            Table* lipidome = 0;
            string sample_name = "";
            for (int i = 0; i < (int)sample_columns.size(); ++i){
                if (sample_name.length()) sample_name += "_";
                sample_name += tokens->at(sample_columns[i]);
            }
            string lipid_name = tokens->at(lipid_species_column);
            
            string index_key_pair = sample_name + " / " + lipid_name;
            if (uncontains_val(index_key_pairs, index_key_pair)){
                index_key_pairs.insert(index_key_pair);
            }
            else {
                if (ignore_doublette_lipids){
                    Logging::write_log("Ignoring lipid doublette '" + index_key_pair + "'");
                    continue;
                }
                else {
                    throw LipidSpaceException("Error while loading pivot table '" + pivot_table_file + "': sample and lipid pair '" + index_key_pair + "' occurs twice in table.", LipidDoublette);
                }
            }
            
            
            
            if (uncontains_val(lipidome_map, sample_name)){
                lipidome = new Table(sample_name);
                selection[3].insert({sample_name, true});
                lipidome_map.insert({sample_name, lipidome});
                loaded_lipidomes.push_back(lipidome);
            }
            else {
                lipidome = lipidome_map[sample_name];
            }
            
            // handle features
            for (int i = 0; i < (int)feature_columns_nominal.size(); ++i){
                lipidome->features.insert({feature_names_nominal[i], Feature(feature_names_nominal[i], tokens->at(feature_columns_nominal[i]))});
                if (uncontains_val(feature_values, feature_names_nominal[i])) {
                    feature_values.insert({feature_names_nominal[i], FeatureSet(feature_names_nominal[i], NominalFeature)});
                }
                feature_values[feature_names_nominal[i]].nominal_values.insert({tokens->at(feature_columns_nominal[i]), true});

            }
            for (int i = 0; i < (int)feature_columns_numerical.size(); ++i){
                double val = atof(tokens->at(feature_columns_numerical[i]).c_str());
                lipidome->features.insert({feature_names_numerical[i], Feature(feature_names_numerical[i], val)});
                if (uncontains_val(feature_values, feature_names_numerical[i])) {
                    feature_values.insert({feature_names_numerical[i], FeatureSet(feature_names_numerical[i], NumericalFeature)});
                }
                feature_values[feature_names_numerical[i]].numerical_values.insert(val);
            }
            
            
            
            // handle lipid
            LipidAdduct *l = 0;
            if (uncontains_val(lipid_map, lipid_name)){
                try {
                    l = parser.parse(lipid_name);
                        
                    // deleting adduct since not necessary
                    if (l->adduct != 0){
                        delete l->adduct;
                        l->adduct = 0;
                    }
                    l->sort_fatty_acyl_chains();
                    
                    all_lipids.push_back(l);
                }
                catch (exception &e) {
                    if (!ignore_unknown_lipids){
                        throw LipidSpaceException(string(e.what()) + ": lipid '" + lipid_name + "' cannot be parsed.", LipidUnparsable);
                    }
                    else {
                        Logging::write_log("Ignoring unidentifiable lipid '" + lipid_name + "'");
                        delete tokens;
                        continue;
                    }
                }
                lipid_map.insert({lipid_name, l});
            }
            else {
                l = lipid_map[lipid_name];
            }
            
            lipidome->lipids.push_back(l);
            lipidome->species.push_back(l->get_lipid_string());
            lipidome->classes.push_back(l->get_lipid_string(CLASS));
            lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
            selection[0].insert({lipidome->species.back(), true});
            selection[1].insert({lipidome->classes.back(), true});
            selection[2].insert({lipidome->categories.back(), true});
            double val = atof(quant_val.c_str());
            lipidome->original_intensities.push_back(val);
            
            delete tokens;
        }
        delete  column_types;
        
        for (auto l : all_lipids){
            for (auto fa : l->lipid->fa_list){
                cut_cycle(fa);
            }
        }
    
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
        }
    
    }
    catch(LipidSpaceException &e){
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    catch (exception &e){
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    
    
    
    fileLoaded();
}





void LipidSpace::load_column_table(string data_table_file, vector<TableColumnType> *column_types){
    Logging::write_log("Importing table '" + data_table_file + "' as lipid column table.");
    
    
    // load and parse lipid table, lipids and features per column, measurements per row
    ifstream infile(data_table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + data_table_file + "' could not be found.", FileUnreadable);
    }
    string line;
    
    bool has_sample_col = false;
    for (auto column_type : *column_types){
        has_sample_col |= (column_type == SampleColumn);
    }
    
    if (!has_sample_col){
        throw LipidSpaceException("No sample column was defined", NoColumnFound);
    }
    
    
    set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
    int line_cnt = 0;
    vector<string> features_names_numerical;
    vector<string> features_names_nominal;
    vector<LipidAdduct*> lipids;
    set<string> lipid_set;
    vector<Table*> loaded_lipidomes;
    
    try {
        while (getline(infile, line)){
            if (line.length() == 0) continue;
            
            
            // handle first / header line different to the others
            vector<string>* tokens = split_string(line, ',', '"', true);
            for (int i = 0; i < (int)tokens->size(); ++i) tokens->at(i) = goslin::strip(tokens->at(i), '"');
            if (column_types->size() != tokens->size()){
                throw LipidSpaceException("Inconsistant column size of header (" + std::to_string(column_types->size()) + ") and line " + std::to_string(line_cnt + 1) + " (" + std::to_string(tokens->size()) + ").", ColumnNumMismatch);
            }
            
            if (line_cnt++ == 0){
                // go through the column and handle them according to their column type
                for (int i = 0; i < (int)tokens->size(); ++i){
                    TableColumnType column_type = column_types->at(i);
                    switch(column_type){
                        case FeatureColumnNominal:
                            features_names_nominal.push_back(tokens->at(i));
                            break;
                            
                        case FeatureColumnNumerical:
                            features_names_numerical.push_back(tokens->at(i));
                            break;
                            
                        case LipidColumn:
                            {
                                bool ignore_lipid = false;
                                LipidAdduct* l = load_lipid(tokens->at(i), lipid_set, ignore_lipid);
                                lipids.push_back(l);
                                break;
                            }
                            
                        default:
                            break;
                    }
                }
                delete tokens;
                continue;
            }
            
            map<string, Feature> features;
            vector<LipidAdduct*> measurement_lipids;
            Array intensities;
            string measurement = "";
            int feature_counter_nominal = 0;
            int feature_counter_numerical = 0;
            int lipid_counter = 0;
            
            // handle all other rows
            for (int i = 0; i < (int)tokens->size(); ++i){
                
                switch(column_types->at(i)){
                    case SampleColumn:
                        measurement = tokens->at(i);
                        break;
                        
                    case LipidColumn:
                        if (lipids[lipid_counter]){
                            string val = tokens->at(i);
                            if (!contains_val(NA_VALUES, val)){
                                measurement_lipids.push_back(lipids[lipid_counter]);
                                intensities.push_back(atof(val.c_str()));
                            }
                        }
                        lipid_counter++;
                        break;
                        
                    case FeatureColumnNominal:
                        {
                            string feature = features_names_nominal[feature_counter_nominal];
                            string feature_val = tokens->at(i);
                            features.insert({feature, Feature(feature, feature_val)});
                            
                            
                            if (uncontains_val(feature_values, feature)) {
                                feature_values.insert({feature, FeatureSet(feature, NominalFeature)});
                            }
                            feature_values[feature].nominal_values.insert({feature_val, true});
                        }
                        feature_counter_nominal++;
                        break;
                        

                        
                    case FeatureColumnNumerical:
                        {
                            string feature = features_names_numerical[feature_counter_numerical];
                            double feature_val = atof(tokens->at(i).c_str());
                            //if (feature_val != 0) 
                                features.insert({feature, Feature(feature, feature_val)});
                            if (uncontains_val(feature_values, feature)) {
                                feature_values.insert({feature, FeatureSet(feature, NumericalFeature)});
                            }
                            //if (feature_val != 0)
                                feature_values[feature].numerical_values.insert(feature_val);
                        }
                        feature_counter_numerical++;
                        break;
                        
                    default:
                        break;
                }
            }
            
            selection[3].insert({measurement, true});
            loaded_lipidomes.push_back(new Table(measurement));
            Table *lipidome = loaded_lipidomes.back();
            for (auto kv : features){
                if (kv.second.feature_type == NominalFeature){
                    lipidome->features.insert({kv.first, Feature(kv.second.name, kv.second.nominal_value)});
                }
                else {
                    lipidome->features.insert({kv.first, Feature(kv.second.name, kv.second.numerical_value)});
                }
            }
                                    
            for (auto l : measurement_lipids){
                lipidome->lipids.push_back(l);
                lipidome->species.push_back(l->get_lipid_string());
                lipidome->classes.push_back(l->get_lipid_string(CLASS));
                lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
                selection[0].insert({lipidome->species.back(), true});
                selection[1].insert({lipidome->classes.back(), true});
                selection[2].insert({lipidome->categories.back(), true});
            }
            lipidome->original_intensities.reset(intensities);
            
            delete tokens;
        }
        delete column_types;
        
        for (auto l : all_lipids){
            for (auto fa : l->lipid->fa_list){
                cut_cycle(fa);
            }
        }
    
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
        }
    
    }
    catch(LipidSpaceException &e){
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    catch (exception &e){
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    
    
    fileLoaded();
}





void LipidSpace::load_row_table(string table_file, vector<TableColumnType> *column_types){
    // load and parse lipid table, lipids per row, measurements per column
    Logging::write_log("Reading table '" + table_file + "' lipid row table.");
    ifstream infile(table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + table_file + "' not found.");
        exit(-1);
    }
    string line;
    

    set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
    vector<Array> intensities;
    set<string> lipid_set;
    vector<Table*> loaded_lipidomes;
    
    
    try {
        // no specific column order is provided, we assume that first column contains
        // lipid species names and all remaining columns correspond to a sample containing
        // intensity / quant values
        if (column_types == 0){
            int line_cnt = 0;
            int num_cols = 0;
            while (getline(infile, line)){
                vector<string>* tokens = split_string(line, ',', '"', true);
                for (int i = 0; i < (int)tokens->size(); ++i) tokens->at(i) = goslin::strip(tokens->at(i), '"');
                
                // handle first / header line different to the others
                if (line_cnt++ == 0){
                    num_cols = tokens->size();
                    for (int i = 1; i < (int)tokens->size(); ++i){
                        selection[3].insert({tokens->at(i), true});
                        loaded_lipidomes.push_back(new Table(tokens->at(i)));
                        intensities.push_back(Array());
                    }
                    delete tokens;
                    continue;
                }
                if (line.length() == 0) continue;
                

                        
                // handle all other rows
                if ((int)tokens->size() != num_cols) {
                    throw LipidSpaceException("Error in line '" + std::to_string(line_cnt) + "' number of cells does not match with number of column labels", ColumnNumMismatch);
                }
                
                bool ignore_lipid = false;
                LipidAdduct* l = load_lipid(tokens->at(0), lipid_set, ignore_lipid);
                
                if (!ignore_lipid){
                
                    for (int i = 1; i < (int)tokens->size(); ++i){
                        string val = tokens->at(i);
                        if (!contains_val(NA_VALUES, val)){
                            Table* lipidome = loaded_lipidomes.at(i - 1);
                            lipidome->lipids.push_back(l);
                            lipidome->species.push_back(l->get_lipid_string());
                            lipidome->classes.push_back(l->get_lipid_string(CLASS));
                            lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
                            selection[0].insert({lipidome->species.back(), true});
                            selection[1].insert({lipidome->classes.back(), true});
                            selection[2].insert({lipidome->categories.back(), true});
                            intensities.at(i - 1).push_back(atof(val.c_str()));
                        }
                    }
                }
                
                delete tokens;
            }
        }
        
        else {
            int line_cnt = 0;
            bool has_lipid_col = false;
            for (auto column_type : *column_types){
                has_lipid_col |= (column_type == LipidColumn);
            }
            
            if (!has_lipid_col){
                throw LipidSpaceException("No lipid column was defined", NoColumnFound);
            }
                
            
            while (getline(infile, line)){
                vector<string>* tokens = split_string(line, ',', '"', true);
                for (int i = 0; i < (int)tokens->size(); ++i) tokens->at(i) = goslin::strip(tokens->at(i), '"');
                if (tokens->size() != column_types->size()) {
                    throw LipidSpaceException("Error in line '" + std::to_string(line_cnt  + 1) + "': number of cells does not match with specified number of columns", ColumnNumMismatch);
                    exit(-1);
                }
                
                // handle first / header line different to the others
                if (line_cnt++ == 0){
                    
                    map<string, int> doublettes;
                    for (int i = 0; i < (int)column_types->size(); ++i){
                        if (column_types->at(i) == SampleColumn){
                            string header = tokens->at(i);
                            if (uncontains_val(doublettes, header)){
                                doublettes.insert({header, 1});
                            }
                            else {
                                header += "." + std::to_string(++doublettes[header]);
                            }
                            
                            selection[3].insert({header, true});
                            loaded_lipidomes.push_back(new Table(header));
                            intensities.push_back(Array());
                        } 
                    }
                    delete tokens;
                    continue;
                }
                if (line.length() == 0) continue;
                
                
                LipidAdduct* l = 0;
                vector<string> quant_data;
                // handle all remaining rows
                bool ignore_lipid = false;
                for (int i = 0; i < (int)column_types->size(); ++i){
                    if (ignore_lipid) break;
                    switch(column_types->at(i)){
                        case SampleColumn:
                            quant_data.push_back(tokens->at(i));
                            break;
                            
                        case LipidColumn:
                            l = load_lipid(tokens->at(i), lipid_set, ignore_lipid);
                            break;
                            
                        default:
                            break;
                    }
                }
                if (!ignore_lipid){
                    
                    for (int i = 0; i < (int)quant_data.size(); ++i){
                        string val = quant_data[i];
                        if (!contains_val(NA_VALUES, val)){
                            Table* lipidome = loaded_lipidomes.at(i);
                            lipidome->lipids.push_back(l);
                            lipidome->species.push_back(l->get_lipid_string());
                            lipidome->classes.push_back(l->get_lipid_string(CLASS));
                            lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
                            selection[0].insert({lipidome->species.back(), true});
                            selection[1].insert({lipidome->classes.back(), true});
                            selection[2].insert({lipidome->categories.back(), true});
                            intensities.at(i).push_back(atof(val.c_str()));
                        }
                        
                    }
                }
                
                delete tokens;
            }
            
            delete column_types;
        }
        
        for (int i = 0; i < (int)loaded_lipidomes.size(); ++i){
            loaded_lipidomes.at(i)->original_intensities.reset(intensities.at(i));
        }
        
        for (auto l : all_lipids){
            for (auto fa : l->lipid->fa_list){
                cut_cycle(fa);
            }
        }
    
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
        }
    
    }
    catch(LipidSpaceException &e){
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    catch (exception &e){
        for (auto lipidome : loaded_lipidomes){
            delete lipidome;
        }
        throw e;
    }
    
    
    fileLoaded();
}


LipidAdduct* LipidSpace::load_lipid(string lipid_name, set<string> &lipid_set, bool &ignore_lipid){
    LipidAdduct* l = nullptr;
    try {
        l = parser.parse(lipid_name);
    }
    catch (exception &e) {
        if (!ignore_unknown_lipids){
            throw LipidSpaceException(string(e.what()) + ": lipid '" + lipid_name + "' cannot be parsed.", LipidUnparsable);
        }
        else {
            Logging::write_log("Ignoring unidentifiable lipid '" + lipid_name + "'");
            ignore_lipid = true;
        }
    }
    
    if (l == nullptr || l->get_extended_class() == UNDEFINED_LIPID) {
        if (!ignore_unknown_lipids){
            throw LipidSpaceException("Lipid '" + lipid_name + "' cannot be parsed.", LipidUnparsable);
        }
        else {
            Logging::write_log("Ignoring unidentifiable lipid '" + lipid_name + "'");
            ignore_lipid = true;
        }
    }
    if (l == nullptr) return l;
        
    // deleting adduct since not necessary
    if (l->adduct != nullptr){
        delete l->adduct;
        l->adduct = nullptr;
    }
    l->sort_fatty_acyl_chains();
    
    string lipid_unique_name = l->get_lipid_string();
    if (uncontains_val(lipid_set, lipid_unique_name)){
        lipid_set.insert(lipid_unique_name);
    }
    else {
        delete l;
        l = nullptr;
        if (ignore_doublette_lipids){
            ignore_lipid = true;
            Logging::write_log("Ignoring doublette lipid '" + lipid_name + "'");
        }
        else {
            throw LipidSpaceException("Error: lipid '" + lipid_name + "' appears twice in the table. " + lipid_unique_name, LipidDoublette);
        }
    }
    if (!ignore_lipid) all_lipids.push_back(l);
    return l;
}








void LipidSpace::store_distance_table(string output_folder, Table* lipidome){
    vector<string> &species = (lipidome != 0) ? lipidome->species : global_lipidome->species;
    Matrix &output_matrix = (lipidome != 0) ? lipidome->m : global_distances;
    string output_file = output_folder + "/distance_matrix.csv";
    
    stringstream table_stream;
    table_stream << "ID";
    for (auto lipid : species) table_stream << "\t" << lipid;
    table_stream << endl;
    for (int i = 0; i < (int)species.size(); ++i){
        table_stream << species.at(i);
        for (int j = 0; j < (int)species.size(); ++j){
            table_stream << "\t" << output_matrix(i, j);
        }
        table_stream << endl;
    }
    ofstream output_stream(output_file);
    output_stream << table_stream.str();
}

/*
std::thread LipidSpace::run_analysis_thread() {
    return std::thread([=] { run_analysis(); });
}
*/


bool mysort(pair<double, int> a, pair<double, int> b){ return a.first < b.first;}

void LipidSpace::run(){
    if (lipidomes.size() == 0){
        if (progress && !progress->stop_progress){
            progress->finish();
            return;
        }
    }
    Logging::write_log("Started analysis");
    
    auto start = high_resolution_clock::now();
    analysis_finished = false;
    if (dendrogram_root){
        delete dendrogram_root;
        dendrogram_root = 0;
    }
    selected_lipidomes.clear();
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    
    if (progress){
        progress->set(0);
    }
    
    
    // compute PCA matrixes for the complete lipidome
    if (!progress || !progress->stop_progress){
        if (!compute_global_distance_matrix()){
            if (progress){
                progress->setError(QString("Less than three lipids were taken for analysis. Analysis aborted. Select more lipids for analysis."));
            }
            return;
        }
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
        global_distances.rewrite(global_lipidome->m);
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
        if (selected_lipidomes.size() > 1){
            compute_hausdorff_matrix();
            if (progress){
                progress->set_step();
            }
        }
    }
    
    if (!progress || !progress->stop_progress){
        if (selected_lipidomes.size() > 1){
            create_dendrogram();
            if (progress){
                progress->set_step();
            }
        }
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Process: " << duration.count() << endl;
    
    if (progress && !progress->stop_progress){
        progress->finish();
        analysis_finished = true;
        Logging::write_log("Finished analysis with a union of " + std::to_string(global_lipidome->lipids.size()) + " lipids among " + std::to_string(lipidomes.size()) + " lipidome" + (lipidomes.size() > 1 ? "s" : "") + " in total.");
    }
}



void LipidSpace::reset_analysis(){
    analysis_finished = false;
    feature_values.clear();
    if (dendrogram_root){
        delete dendrogram_root;
        dendrogram_root = 0;
    }
    selected_lipidomes.clear();
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    
    for (int i = 0; i < 4; ++i) selection[i].clear();
    
    for (auto lipid : all_lipids) delete lipid;
    all_lipids.clear();
    
    for (auto lipidome : lipidomes) delete lipidome;
    lipidomes.clear();
    cols_for_pca = cols_for_pca_init;
    
    
    
    global_lipidome->species.clear();
    global_lipidome->classes.clear();
    global_lipidome->categories.clear();
    global_lipidome->lipids.clear();
    global_lipidome->intensities.clear();
    global_lipidome->m.reset(1, 1);
}



