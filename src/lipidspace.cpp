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
    
    
    
    // determining importance of lipids based on goodness of separating
    // study variables applying simple 1D linear regression 
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    Matrix global_matrix;
    
    
    // setting up lipid to column in matrix map
    for (auto lipidome : selected_lipidomes){
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            LipidAdduct* lipid = lipidome->lipids[i];
            if (uncontains_val(lipid_map, lipid)){
                lipid_map.insert({lipid, lipid_map.size()});
                lipid_name_map.insert({lipidome->species[i], lipid_name_map.size()});
            }
        }
    }

    // set up matrix for multiple linear regression
    global_matrix.reset(selected_lipidomes.size(), lipid_map.size());
    for (uint r = 0; r < selected_lipidomes.size(); ++r){
        Lipidome* lipidome = selected_lipidomes[r];
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            global_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->original_intensities[i];
        }
    }
    
    // computing mean values for all lipids
    vector<double> mx_values;
    for (int c = 0; c < global_matrix.cols; c++){
        double m = 0;
        double n = 0;
        double* row = &(global_matrix.m[c * global_matrix.rows]);
        for (int r = 0; r < global_matrix.rows; ++r){
            if (row[r] <= 1e-15) continue;
            m += row[r];
            n += 1;
        }
        mx_values.push_back(m / n);
    }
    
    
    
    // computing the coefficient of variation for each lipid
    double max_cv = 0.;
    vector<pair<double, string>> CVs;
    for (auto kv : lipid_name_map){
        int c = kv.second;
        Array values;
        double* row = &(global_matrix.m[c * global_matrix.rows]);
        for (int r = 0; r < global_matrix.rows; ++r){
            if (row[r] > 1e-15) values.push_back(row[r]);
            
        }
        CVs.push_back({values.stdev() / values.mean(), kv.first});
        max_cv = max(max_cv, CVs.back().first);
    }
    // normalizing all CV values by setting highest value to 1
    if (max_cv > 0){
        for (auto &val : CVs) val.first /= max_cv;
    }
    
    sort(CVs.begin(), CVs.end(), sort_double_string_desc);
    lipid_sortings.insert({"Coefficient of Variation (Desc)", vector<pair<string, double>>()});
    vector<pair<string, double>> &CVh = lipid_sortings["Coefficient of Variation (Desc)"];
    for (auto cv : CVs) CVh.push_back({cv.second, cv.first});
    
    
    
    // going through all study variables
    for (auto kv : feature_values){
        string target_variable = kv.first;
        
        Array target_values;
        vector<pair<double, string>> regression_result;
        map<string, double> nominal_target_values;
        int nom_counter = 0;
        bool is_nominal = feature_values[target_variable].feature_type == NominalFeature;
        
        if (uncontains_val(feature_values, target_variable)){
            return;
        }
        
        // setup array for target variable values, if nominal then each with incrementing number
        if (is_nominal){
            for (auto lipidome : selected_lipidomes){
                string nominal_value = lipidome->features[target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                }
                target_values.push_back(nominal_target_values[nominal_value]);
            }
        }
        else {
            for (auto lipidome : selected_lipidomes){
                target_values.push_back(lipidome->features[target_variable].numerical_value);
            }
        }
        
        // going through all lipids
        for (auto kv : lipid_name_map){
            int c = kv.second;
            if (is_nominal){
                vector<Array> arrays(nom_counter);
                
                for (int r = 0; r < global_matrix.rows; ++r){
                    if (global_matrix(r, c) <= 1e-15) continue;
                    arrays[target_values[r]].push_back(global_matrix(r, c));
                }
                
                double pos_max = 0, d = 0, score = 1;
                for (uint ii = 0; ii < arrays.size() - 1; ++ii){
                    for (uint jj = ii + 1; jj < arrays.size(); ++jj){
                        ks_separation_value(arrays[ii], arrays[jj], d, pos_max);
                        score *= d;
                    }
                }
                double ll = 2. / ((double)(arrays.size() - 1) * (double)arrays.size());
                score = pow(score, ll);
                
                regression_result.push_back({score, kv.first});
                
            }
            else {
                double mx = mx_values[c];
                double my = 0, ny = 0;
                // computing the study variable mean based on missing values of lipids
                for (int r = 0; r < global_matrix.rows; ++r){
                    if (global_matrix(r, c) <= 1e-15) continue;
                    my += target_values[r];
                    ny += 1;
                }
                my /= ny;
                
                // estimate slope and intercept factors for linear regression
                double slope_num = 0, slope_denom = 0;
                double* row = &(global_matrix.m[c * global_matrix.rows]);
                for (int r = 0; r < global_matrix.rows; ++r){
                    if (global_matrix(r, c) <= 1e-15) continue;
                    slope_num += (row[r] - mx) * (target_values[r] - my);
                    slope_denom += sq(row[r] - mx);
                }
                double slope = slope_num / slope_denom;
                double intercept = my - slope * mx;
                
                // checking with R^2 performance indicator goodness of the fit
                double SQR = 0, SQT = 0;
                for (int r = 0; r < global_matrix.rows; ++r){
                    if (global_matrix(r, c) <= 1e-15) continue;
                    SQR += sq(target_values[r] - (slope * row[r] + intercept));
                    SQT += sq(target_values[r] - my);
                }
                double R2 = 1. - SQR / SQT;
                regression_result.push_back({R2, kv.first});
            }
            
        }
        
        // sorting all lipids according to the performance indicator
        sort(regression_result.begin(), regression_result.end(), sort_double_string_desc);
        lipid_sortings.insert({target_variable + " regression (Desc)", vector<pair<string, double>>()});
        vector<pair<string, double>> &regression_vector = lipid_sortings[target_variable + " regression (Desc)"];
        for (auto kv_reg : regression_result) regression_vector.push_back({kv_reg.second, kv_reg.first});
        
    }
}




LipidSpace::LipidSpace() {
    keep_sn_position = true;
    ignore_unknown_lipids = false;
    ignore_doublette_lipids = false;
    unboundend_distance = false;
    without_quant = false;
    global_lipidome = new Lipidome("global_lipidome", "");
    progress = 0;
    analysis_finished = false;
    dendrogram_root = 0;
    progress = 0;
    process_id = 0;
    target_variable = "";
    feature_values.insert({FILE_FEATURE_NAME, FeatureSet(FILE_FEATURE_NAME, NominalFeature)});
    lipid_name_translations.resize(2);
        
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
        registered_lipid_classes.insert(tokens->at(0));
        registered_lipid_classes.insert(tokens->at(1));
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
    reset_analysis();
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




bool LipidSpace::is_double(const std::string& s){
    std::istringstream iss(s);
    double d;
    return iss >> d >> std::ws && iss.eof();
}



void LipidSpace::fatty_acyl_similarity(FattyAcid* fa1, FattyAcid* fa2, int& union_num, int& inter_num){
    set<LipidFaBondType> lcbs = {LCB_REGULAR, LCB_EXCEPTION};
    set<LipidFaBondType> bond_types = {fa1->lipid_FA_bond_type, fa2->lipid_FA_bond_type};
    //int uu = union_num, ii = inter_num;
    
    int db1 = 0, db2 = 0;
    if (bond_types.size() == 1){ // both equal
        if (contains_val(bond_types, ESTER)){
            inter_num += 2; // one oxygen, one double bond
            union_num += 2; // one oxygen, one double bond
        }
    }
    
    else if (contains_val(bond_types, ESTER)){
        if (contains_val(bond_types, ETHER_PLASMANYL) || contains_val(bond_types, LCB_REGULAR) || contains_val(bond_types, LCB_EXCEPTION)){
            union_num += 2; // one oxygen, one double bond
        }
        else if (contains_val(bond_types, ETHER_PLASMENYL)){
            union_num += 3; // one oxygen, two double bonds
        }
    }
    
    else if (contains_val(bond_types, LCB_EXCEPTION) || contains_val(bond_types, LCB_REGULAR)){
        if (contains_val(bond_types, ETHER_PLASMENYL)){
            union_num += 1; // one double bond
        }
        else if (contains_val(bond_types, ETHER_PLASMANYL)){
            
        }
    }
        
    else if (contains_val(bond_types, ETHER_PLASMENYL) && contains_val(bond_types, ETHER_PLASMANYL)){
        union_num += 1; // one double bond
    }
        
    else { 
        throw LipidException("Cannot compute similarity between chains");
    }
    
    
    // prepare double bonds for counting
    bool fa1db_new = contains_val(lcbs, fa1->lipid_FA_bond_type) && fa1->double_bonds->double_bond_positions.size() > 0;
    bool fa2db_new = contains_val(lcbs, fa2->lipid_FA_bond_type) && fa2->double_bonds->double_bond_positions.size() > 0;
    
    map<int, string> *fa1db = 0;
    map<int, string> *fa2db = 0;
    if (fa1db_new){
        fa1db = new map<int, string>();
        for (auto kv : fa1->double_bonds->double_bond_positions){
            if (kv.first > 4){
                db1 += 1;
                fa1db->insert({kv.first - 4, kv.second});
            }
        }
    }
    else {
        fa1db = &fa1->double_bonds->double_bond_positions;
    }
    if (fa2db_new){
        fa2db = new map<int, string>();
        for (auto kv : fa2->double_bonds->double_bond_positions){
            if (kv.first > 4){
                db2 += 1;
                fa2db->insert({kv.first - 4, kv.second});
            }
        }
    }
    else {
        fa2db = &fa2->double_bonds->double_bond_positions;
    }
    
    // compare lengths
    int m1 = contains_val(lcbs, fa1->lipid_FA_bond_type) * 4;
    int m2 = contains_val(lcbs, fa2->lipid_FA_bond_type) * 4;
    
    inter_num += max(0, min(fa1->num_carbon - m1, fa2->num_carbon - m2));
    union_num += max(0, max(fa1->num_carbon - m1, fa2->num_carbon - m2));
    
    
    // compare double bonds
    int common_db = 0;
    if (fa1db->size() > 0 && fa2db->size() > 0){
        for (auto kv : *fa1db){
            common_db += contains_val_p(fa2db, kv.first) && fa2db->at(kv.first) == kv.second;
        }
        inter_num += common_db;
        union_num += fa1db->size() + fa2db->size() - common_db;
        db1 += fa1db->size();
        db2 += fa2db->size();
    }
    else {
        db1 += fa1db->size() > 0 ? fa1db->size() : fa1->double_bonds->get_num();
        db2 += fa2db->size() > 0 ? fa2db->size() : fa2->double_bonds->get_num();
            
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
    inter_num += max(0, min(fa1->num_carbon - m1, fa2->num_carbon - m2) - (db1 + db2 - common_db));
    union_num += max(0, max(fa1->num_carbon - m1, fa2->num_carbon - m2) - (db1 + db2 - common_db));
    
    
    if (fa1->functional_groups->size() == 0 && fa2->functional_groups->size() == 0){
        if (fa1db_new) delete fa1db;
        if (fa2db_new) delete fa2db;
        //cout << fa1->name << " vs. " << fa2->name << ": " << (inter_num - ii) << " / " << (union_num - uu) << endl;
        return;
    }
    
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
                int naab = fa1->functional_groups->at(key).at(0)->num_atoms_and_bonds;
                
                vector<int> unions;
                set_union(keys_fa1.begin(), keys_fa1.end(), keys_fa2.begin(), keys_fa2.end(), back_inserter(unions));
                vector<int> inters;
                set_intersection(keys_fa1.begin(), keys_fa1.end(), keys_fa2.begin(), keys_fa2.end(), back_inserter(unions));
                
                inter_num += inters.size() * naab;
                union_num += unions.size() * naab;
            }
        }
        // functional group occurs only in first fatty acid
        else {
            FunctionalGroup* func_group = KnownFunctionalGroups::get_functional_group(key);
            if (func_group){
                int naab = func_group->num_atoms_and_bonds;
                
                int num = 0;
                if (fa1->functional_groups->at(key).at(0)->position > -1){
                    num = fa1->functional_groups->at(key).size();
                }
                else {
                    for (auto f : fa1->functional_groups->at(key)) num += f->count;
                }
                if (fa1->lipid_FA_bond_type == LCB_EXCEPTION && func_group->name == "O") num -= 1;
                union_num += naab * num;
                delete func_group;
            }
            else {
                for (auto fg : kv.second){
                    union_num += fg->num_atoms_and_bonds;
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
            int naab = func_group->num_atoms_and_bonds;
            
            int num = 0;
            if (fa2->functional_groups->at(key).at(0)->position > -1){
                num = fa2->functional_groups->at(key).size();
            }
            else {
                for (auto fg : fa2->functional_groups->at(key)) num += fg->count;
            }
            if (fa2->lipid_FA_bond_type == LCB_EXCEPTION && func_group->name == "O") num -= 1;
            union_num += naab * num;
            delete func_group;
        }
        else {
            for (auto fg : fa2->functional_groups->at(key)){
                union_num += fg->num_atoms_and_bonds;
            } 
        }
    }
    if (fa1db_new) delete fa1db;
    if (fa2db_new) delete fa2db;
    //cout << fa1->name << " vs. " << fa2->name << ": " << (inter_num - ii) << " / " << (union_num - uu) << endl;
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
            FattyAcid *dummy = new FattyAcid("dummyFA", 0, 0, 0, ETHER_PLASMANYL);
            for (int i = (int)orig_fa_list_2->size(); i < (int)orig_fa_list_1->size(); ++i){
                
                fatty_acyl_similarity(orig_fa_list_1->at(i), dummy, union_num, inter_num);
                /*
                FattyAcid* fa = orig_fa_list_1->at(i);
                union_num += fa->get_double_bonds();
                ElementTable* e = fa->get_elements();
                for (auto kv : *e){
                    if (kv.first != ELEMENT_H) union_num += kv.second;
                }
                delete e;
                */
            }
            delete dummy;
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
    
    // delete unnecessary lipids
    set<LipidAdduct*> lipids_to_delete;
    for (auto kv : all_lipids) lipids_to_delete.insert(kv.second);
    all_lipids.clear();
    for (auto lipidome : lipidomes){
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            if (uncontains_val(all_lipids, lipidome->species.at(i))){
                all_lipids.insert({lipidome->species.at(i), lipidome->lipids.at(i)});
            }
            if (contains_val(lipids_to_delete, lipidome->lipids.at(i))){
                lipids_to_delete.erase(lipidome->lipids.at(i));
            }
        }
    }
    for (auto lipid : lipids_to_delete) delete lipid;
    
    set<string> all_lipid_names;
    for (auto lipidome : lipidomes){
        for (auto lipid_name : lipidome->species){
            all_lipid_names.insert(lipid_name);
        }
    }
    
    // clean up translation maps
    vector<string> lipid_names_to_delete;
    for (auto &lipid_name_kv : lipid_name_translations[TRANSLATED_NAME]){
        if (uncontains_val(all_lipid_names, lipid_name_kv.first)) lipid_names_to_delete.push_back(lipid_name_kv.first);
    }
    
    for (auto lipid_name : lipid_names_to_delete){
        lipid_name_translations[TRANSLATED_NAME].erase(lipid_name);
        lipid_name_translations[IMPORT_NAME].erase(lipid_name);
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
    map<string, LipidAdduct*> lipid_set;
    
    Lipidome* lipidome = new Lipidome(lipid_list_file, lipid_list_file, true);
    try {
        while (getline(infile, line)){
            if (line.length() == 0) continue;
            vector<string>* tokens = goslin::split_string(line, '\t', '"', true);
            for (uint i = 0; i < tokens->size(); ++i) tokens->at(i) = strip(tokens->at(i), '"');
            double intensity = 1;
            string lipid_name = tokens->at(0);
            if (tokens->size() > 1) intensity = atof(tokens->at(1).c_str());
            delete tokens;
            
            LipidAdduct* l = load_lipid(lipid_name, lipid_set);
            
            if (l){
                lipidome->lipids.push_back(l);
                lipidome->species.push_back(l->get_lipid_string());
                lipidome->classes.push_back(l->get_lipid_string(CLASS));
                lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
                lipidome->original_intensities.push_back(intensity);
            }
        }
        
        set<string> lipidome_names;
        lipidome_names.insert(lipidome->cleaned_name);
        for (auto lipidome : lipidomes){
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains sample name '" + lm + "' which is already registered in LipidSpace.", CorruptedFileFormat);
            }
        }
        
        if (feature_values.size() > 1){
            throw LipidSpaceException("Study variables have been loaded. Lists do not supported any study variable import routine. Please reset LipidSpace.", FeatureNotRegistered);
        }
        feature_values[FILE_FEATURE_NAME].nominal_values.insert({lipidome->features[FILE_FEATURE_NAME].nominal_value, true});
        
        
        lipidomes.push_back(lipidome);
        selection[3].insert({lipidome->cleaned_name, true});
            
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            selection[0].insert({lipidome->species.at(i), true});
            selection[1].insert({lipidome->classes.at(i), true});
            selection[2].insert({lipidome->categories.at(i), true});
        }
        for (auto kv : lipid_set){
            if (uncontains_val(all_lipids, kv.first)) all_lipids.insert({kv.first, kv.second});
        }
    }
    catch(LipidSpaceException &e){
        delete lipidome;
        throw e;
    }
    catch(exception &e){
        delete lipidome;
        throw e;
    }
    
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
            if (min_h <= max_h || min_h == 0) break;
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
            if (min_h <= max_h || min_h == 0) break;
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




int LipidSpace::compute_global_distance_matrix(){
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
    
    // Lanczos algorithm computes one less principal components than
    // lipids provided, we need at least 2 dimensions, therefore
    // at least three lipids
    if (n < 3) return n;

    
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
    return n;
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
    double global_stdev = 0;
    if (global_lipidome->lipids.size() >= 3){
        double global_mean = 0;
        for (int i = 0; i < n; ++i) global_mean += global_lipidome->m.m[i];
        global_mean /= (double)n;
        for (int i = 0; i < n; ++i) global_stdev += sq(global_lipidome->m.m[i] - global_mean);
        global_stdev = sqrt(global_stdev / (double)n);
    }
    else {
        global_stdev = 1;
    }
    
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
    size_t start_sample_num = line.find("[");
    size_t end_sample_num = line.find("]");
    if ((start_sample_num == std::string::npos) || (end_sample_num == std::string::npos) || (end_sample_num <= start_sample_num)){
        throw LipidSpaceException("Error in line " + std::to_string(line_number) + ", line is corrupted.", CorruptedFileFormat);
    }
    start_sample_num += 1;
    return atoi(line.substr(start_sample_num, end_sample_num - start_sample_num).c_str());
}


void LipidSpace::load_mzTabM(string mzTabM_file){
    Logging::write_log("Importing table '" + mzTabM_file + "' as mzTabM table.");
    set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
    
    ifstream infile(mzTabM_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + mzTabM_file + "' could not be found.", FileUnreadable);
    }
    vector<Lipidome*> loaded_lipidomes;
    map<int, string> feature_names;
    
    vector<Feature> headers;
    map<string, LipidAdduct*> lipid_set;
    
    vector<string> *tokens = 0;
    vector<string> *sample_data = 0;
    vector<string> *content_tokens = 0;
    map<string, FeatureType> feature_types;
    
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
                        loaded_lipidomes[sample_number - 1] = new Lipidome(sample_name, mzTabM_file);
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
                        loaded_lipidomes[sample_number - 1] = new Lipidome("-", mzTabM_file);
                    }
                    
                    int content_number = extract_number(sample_data->at(1), line_num);
                    string content_name = sample_data->at(1).substr(0, sample_data->at(1).find("["));
                    
                    vector<string> *content_tokens = split_string(strip(strip(strip(tokens->at(2), ' '), '['), ']'), ',', '\0', true);
                    for (uint i = 0; i < content_tokens->size(); ++i) content_tokens->at(i) = strip(content_tokens->at(i), ' ');
                    
                    if (content_name == "custom"){
                        if (content_tokens->size() < 4 || content_tokens->at(2).size() == 0 || content_tokens->at(3).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = strip(content_tokens->at(2), '"');
                        string value = strip(content_tokens->at(3), '"');
                        if (uncontains_val(feature_names, content_number)){
                            feature_names.insert({content_number, key});
                        }
                        else if (feature_names[content_number] != key){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", different study variable key than already used, '" + feature_names[content_number] + "' vs. '" + key + "'", CorruptedFileFormat);
                        }
                        
                        // is custom value a number?
                        FeatureType ft = NominalFeature;
                        if (is_double(value)){
                            loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, atof(value.c_str()))});
                            ft = NumericalFeature;
                        }
                        else {
                            loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, value)});
                        }
                        
                        if (uncontains_val(feature_types, key)){
                            feature_types.insert({key, ft});
                        }
                        else {
                            if (feature_types[key] != ft){
                                throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", key of study variable '" + key + "' is already registered with different data type (string, number).", CorruptedFileFormat);
                            }
                        }
                    }
                    else if (content_name == "species"){
                        if (content_tokens->size() < 3 || content_tokens->at(2).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = "Taxonomic species";
                        if (uncontains_val(feature_names, -1)){
                            feature_names.insert({-1, key});
                        }
                        string value = content_tokens->at(2);
                        loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, strip(value, '"'))});
                    }
                    else if (content_name == "tissue"){
                        if (content_tokens->size() < 3 || content_tokens->at(2).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = "Tissue";
                        if (uncontains_val(feature_names, -2)){
                            feature_names.insert({-2, key});
                        }
                        string value = content_tokens->at(2);
                        loaded_lipidomes[sample_number - 1]->features.insert({key, Feature(key, strip(value, '"'))});
                    }
                    
                    delete content_tokens;
                    content_tokens = 0;
                }
                
                delete sample_data;
                sample_data = 0;
            }
            
            if (tokens->size() >= 1 && tokens->at(0) == "SMH"){
                if (!headers.empty()){
                    throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", header already defined.", CorruptedFileFormat);
                }
                    
                    
                for (uint i = 0; i < tokens->size(); ++i){
                    string header = tokens->at(i);
                    if (header.size() > 16 && header.substr(0, 16) == "abundance_assay["){
                        int sample_number = extract_number(header, line_num) - 1;
                        if (sample_number < 0 || sample_number >= (int)loaded_lipidomes.size()){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", assay number '" + std::to_string(sample_number) + "' not registered.", CorruptedFileFormat);
                        }
                        headers.push_back(Feature("[s]", sample_number));
                    }
                    else {
                        headers.push_back(Feature(header, ""));
                    }
                }
            }
            
            if (tokens->size() >= 1 && tokens->at(0) == "SML"){
                if (tokens->size() > headers.size()){
                    throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", more elements (" + std::to_string(tokens->size()) + ") than defined in header (" + std::to_string(headers.size()) + ").", CorruptedFileFormat);
                }
                
                LipidAdduct *l = 0;
                
                for (uint i = 0; i < tokens->size(); ++i){
                    Feature &f = headers[i];
                    string value = tokens->at(i);
                    
                    // found abundance
                    if (f.name == "[s]"){
                        if (!l && !ignore_unknown_lipids && !ignore_doublette_lipids){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", lipid name (with header 'chemical_name') must occur before any abundance.", CorruptedFileFormat);
                        }
                        
                        if (l && !contains_val(NA_VALUES, value)){ // valid abundance
                            Lipidome* lipidome = loaded_lipidomes.at((int)f.numerical_value);
                            lipidome->lipids.push_back(l);
                            lipidome->species.push_back(l->get_lipid_string());
                            lipidome->classes.push_back(l->get_lipid_string(CLASS));
                            lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
                            lipidome->original_intensities.push_back(atof(value.c_str()));
                        }
                        
                    }
                    else if (f.name == "chemical_name"){
                        l = load_lipid(tokens->at(i), lipid_set);
                    }
                }
            }
                       
            
            delete tokens;
            tokens = 0;
        }
        set<string> lipidome_names;
        for (auto lipidome : loaded_lipidomes){
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains multiple time the sample name '" + lm + "'.", CorruptedFileFormat);
            }
        }
        for (auto lipidome : lipidomes){
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains sample name '" + lm + "' which is already registered in LipidSpace.", CorruptedFileFormat);
            }
        }
        
        // checking consistancy of features
        set<string> registered_features;
        registered_features.insert(FILE_FEATURE_NAME);
        for (auto kv : feature_names) registered_features.insert(kv.second);
        for (auto lipidome : loaded_lipidomes){
            if (lipidome->cleaned_name == "-"){
                throw LipidSpaceException("Error, sample has no sample name.", CorruptedFileFormat);
            }
            
            for (auto feature : registered_features){
                if (uncontains_val(lipidome->features, feature)){
                    throw LipidSpaceException("Error, study variable '" + feature + "' not defined for sample '" + lipidome->cleaned_name + "'.", CorruptedFileFormat);
                }
            }
        }
        
        if (feature_values.size() > 1){
            for (auto feature : registered_features){
                if (uncontains_val(feature_values, feature)){
                    throw LipidSpaceException("Error, study variable '" + feature + "' is not registed already.", FeatureNotRegistered);
                }
            }
            
            for (auto kv : feature_values){
                if (uncontains_val(registered_features, kv.first)){
                    throw LipidSpaceException("Error, study variable '" + kv.first + "' is not present in imported file.", FeatureNotRegistered);
                }
            }
            
            // add new values into existing features
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->features){
                    FeatureSet &fs = feature_values[kv.first];
                    if (kv.second.feature_type == NumericalFeature){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        // register features
        else {
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->features){
                    if (uncontains_val(feature_values, kv.first)){
                        feature_values.insert({kv.first, FeatureSet(kv.first, kv.second.feature_type)});
                    }
                    FeatureSet &fs = feature_values[kv.first];
                    if (kv.second.feature_type == NumericalFeature){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
            selection[3].insert({lipidome->cleaned_name, true});
            
            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                selection[0].insert({lipidome->species.at(i), true});
                selection[1].insert({lipidome->classes.at(i), true});
                selection[2].insert({lipidome->categories.at(i), true});
            }
        }
        for (auto kv : lipid_set){
            if (uncontains_val(all_lipids, kv.first)) all_lipids.insert({kv.first, kv.second});
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



void LipidSpace::load_flat_table(string flat_table_file, vector<TableColumnType> *column_types, string sheet){
    Logging::write_log("Importing table '" + flat_table_file + "' as flat table.");
    
    // load and parse lipid table, lipids and features per column, measurements per row
    ifstream infile(flat_table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + flat_table_file + "' could not be found.", FileUnreadable);
    }
    
    vector<Lipidome*> loaded_lipidomes;
    set<string> registered_features;
    
    try {
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
            throw LipidSpaceException("Error while loading flat table '" + flat_table_file + "': not all essential columns defined.", NoColumnFound);
        }
        
        set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
        map<string, LipidAdduct*> lipid_map;
        map<string, Lipidome*> lipidome_map;
        vector<string> feature_names_nominal;
        vector<string> feature_names_numerical;
        map<string, LipidAdduct*> load_lipids;
        
        
        FileTableHandler fth(flat_table_file, sheet);
        
        for (auto fi : feature_columns_nominal){
            string feature = fth.headers.at(fi);
            feature_names_nominal.push_back(feature);
            registered_features.insert(feature);
        }
        for (auto fi : feature_columns_numerical){
            string feature = fth.headers.at(fi);
            feature_names_numerical.push_back(feature);
            registered_features.insert(feature);
        }
        
        for (auto tokens : fth.rows){
            
            // check if quant information is valid
            string quant_val = tokens.at(quant_column);
            if (contains_val(NA_VALUES, quant_val)){
                continue;
            }
            
            // take or create sample / lipidome table
            Lipidome* lipidome = 0;
            string sample_name = "";
            for (int i = 0; i < (int)sample_columns.size(); ++i){
                if (sample_name.length()) sample_name += "_";
                sample_name += tokens.at(sample_columns[i]);
            }
            string lipid_table_name = tokens.at(lipid_species_column);
            
            string index_key_pair = sample_name + " / " + lipid_table_name;
            if (uncontains_val(index_key_pairs, index_key_pair)){
                index_key_pairs.insert(index_key_pair);
            }
            else {
                if (ignore_doublette_lipids){
                    Logging::write_log("Ignoring lipid doublette '" + index_key_pair + "'");
                    continue;
                }
                else {
                    throw LipidSpaceException("Error while loading flat table '" + flat_table_file + "': sample and lipid pair '" + index_key_pair + "' occurs twice in table.", LipidDoublette);
                }
            }
            
            
            
            if (uncontains_val(lipidome_map, sample_name)){
                lipidome = new Lipidome(sample_name, flat_table_file);
                lipidome_map.insert({sample_name, lipidome});
                loaded_lipidomes.push_back(lipidome);
            }
            else {
                lipidome = lipidome_map[sample_name];
            }
            
            // handle features
            for (int i = 0; i < (int)feature_columns_nominal.size(); ++i){
                lipidome->features.insert({feature_names_nominal[i], Feature(feature_names_nominal[i], tokens.at(feature_columns_nominal[i]))});
                
            }
            for (int i = 0; i < (int)feature_columns_numerical.size(); ++i){
                double val = atof(tokens.at(feature_columns_numerical[i]).c_str());
                lipidome->features.insert({feature_names_numerical[i], Feature(feature_names_numerical[i], val)});
            }
            
            // handle lipid
            LipidAdduct *l = 0;
            string translated_name = "";
            if (uncontains_val(load_lipids, lipid_table_name)){
                try {
                    l = parser.parse(lipid_table_name);
                    
                    if (uncontains_val(registered_lipid_classes, l->get_extended_class())){
                        if (!ignore_unknown_lipids){
                            throw LipidSpaceException("Lipid structure not registered: lipid '" + lipid_table_name + "' cannot be parsed.", LipidNotRegistered);
                        }
                        else {
                            Logging::write_log("Ignoring unregistered lipid '" + lipid_table_name + "'");
                            continue;
                        }
                    }
                    translated_name = l->get_lipid_string();
                        
                    // deleting adduct since not necessary
                    if (l->adduct != 0){
                        delete l->adduct;
                        l->adduct = 0;
                    }
                    l->sort_fatty_acyl_chains();
                    for (auto fa : l->lipid->fa_list) cut_cycle(fa);
                }
                catch (exception &e) {
                    if (!ignore_unknown_lipids){
                        throw LipidSpaceException(string(e.what()) + ": lipid '" + lipid_table_name + "' cannot be parsed.", LipidUnparsable);
                    }
                    else {
                        Logging::write_log("Ignoring unidentifiable lipid '" + lipid_table_name + "'");
                        continue;
                    }
                }
                string lipid_name = l->get_lipid_string();
                if (contains_val(all_lipids, lipid_name)){
                    delete l;
                    l = all_lipids[lipid_name];
                }
                else if (contains_val(lipid_map, lipid_name)){
                    delete l;
                    l = lipid_map[lipid_name];
                }
                else {
                    lipid_map.insert({lipid_name, l});
                }
                load_lipids.insert({lipid_table_name, l});
            }
            else {
                l = load_lipids[lipid_table_name];
            }
                
            
            string lipid_string = l->get_lipid_string();
            if (uncontains_val(lipid_name_translations[TRANSLATED_NAME], lipid_string))
                lipid_name_translations[TRANSLATED_NAME].insert({lipid_string, translated_name});
            if (uncontains_val(lipid_name_translations[IMPORT_NAME], lipid_string))
                lipid_name_translations[IMPORT_NAME].insert({lipid_string, lipid_table_name});
                
            lipidome->lipids.push_back(l);
            lipidome->species.push_back(lipid_string);
            lipidome->classes.push_back(l->get_lipid_string(CLASS));
            lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
            double val = atof(quant_val.c_str());
            lipidome->original_intensities.push_back(val);
            
        }
        set<string> lipidome_names;
        for (auto lipidome : loaded_lipidomes){
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains multiple time the sample name '" + lm + "'.", CorruptedFileFormat);
            }
        }
        for (auto lipidome : lipidomes){
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains sample name '" + lm + "' which is already registered in LipidSpace.", CorruptedFileFormat);
            }
        }
        
        // checking consistancy of features
        registered_features.insert(FILE_FEATURE_NAME);
        for (auto lipidome : loaded_lipidomes){
            if (lipidome->cleaned_name == "-"){
                throw LipidSpaceException("Error, sample has no sample name.", CorruptedFileFormat);
            }
            
            for (auto feature : registered_features){
                if (uncontains_val(lipidome->features, feature)){
                    throw LipidSpaceException("Error, study variable '" + feature + "' not defined for sample '" + lipidome->cleaned_name + "'.", CorruptedFileFormat);
                }
            }
        }
        
        if (feature_values.size() > 1){
            for (auto feature : registered_features){
                if (uncontains_val(feature_values, feature)){
                    throw LipidSpaceException("Error, study variable '" + feature + "' is not registed already.", FeatureNotRegistered);
                }
            }
            
            for (auto kv : feature_values){
                if (uncontains_val(registered_features, kv.first)){
                    throw LipidSpaceException("Error, study variable '" + kv.first + "' is not present in imported file.", FeatureNotRegistered);
                }
            }
            
            // add new values into existing features
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->features){
                    FeatureSet &fs = feature_values[kv.first];
                    if (kv.second.feature_type == NumericalFeature){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        // register features
        else {
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->features){
                    if (uncontains_val(feature_values, kv.first)){
                        feature_values.insert({kv.first, FeatureSet(kv.first, kv.second.feature_type)});
                    }
                    FeatureSet &fs = feature_values[kv.first];
                    if (kv.second.feature_type == NumericalFeature){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        
    
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
            selection[3].insert({lipidome->cleaned_name, true});
            
            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                selection[0].insert({lipidome->species.at(i), true});
                selection[1].insert({lipidome->classes.at(i), true});
                selection[2].insert({lipidome->categories.at(i), true});
            }
        }
            
        for (auto kv : lipid_map){
            if (uncontains_val(all_lipids, kv.first)) all_lipids.insert({kv.first, kv.second});
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
    delete column_types;
    fileLoaded();
}





void LipidSpace::load_column_table(string data_table_file, vector<TableColumnType> *column_types, string sheet){
    Logging::write_log("Importing table '" + data_table_file + "' as lipid column table.");
    
    // load and parse lipid table, lipids and features per column, measurements per row
    ifstream infile(data_table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + data_table_file + "' could not be found.", FileUnreadable);
    }
    
    bool has_sample_col = false;
    for (auto column_type : *column_types){
        has_sample_col |= (column_type == SampleColumn);
    }
    
    if (!has_sample_col){
        throw LipidSpaceException("No sample column was defined", NoColumnFound);
    }
    
    
    set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
    vector<string> features_names_numerical;
    vector<string> features_names_nominal;
    vector<LipidAdduct*> lipids;
    map<string, LipidAdduct*> lipid_set;
    vector<Lipidome*> loaded_lipidomes;
    
    FileTableHandler fth(data_table_file, sheet);
    
    try {
        // go through the column and handle them according to their column type
        for (int i = 0; i < (int)fth.headers.size(); ++i){
            TableColumnType column_type = column_types->at(i);
            switch(column_type){
                case FeatureColumnNominal:
                    features_names_nominal.push_back(fth.headers.at(i));
                    break;
                    
                case FeatureColumnNumerical:
                    features_names_numerical.push_back(fth.headers.at(i));
                    break;
                    
                case LipidColumn:
                    {
                        LipidAdduct* l = load_lipid(fth.headers.at(i), lipid_set);
                        lipids.push_back(l);
                        break;
                    }
                    
                default:
                    break;
            }
        }
        
        
        for (auto tokens : fth.rows){
            map<string, Feature> features;
            vector<LipidAdduct*> measurement_lipids;
            Array intensities;
            string measurement = "";
            int feature_counter_nominal = 0;
            int feature_counter_numerical = 0;
            int lipid_counter = 0;
            
            // handle all other rows
            for (int i = 0; i < (int)tokens.size(); ++i){
                
                switch(column_types->at(i)){
                    case SampleColumn:
                        measurement = tokens.at(i);
                        break;
                        
                    case LipidColumn:
                        if (lipids[lipid_counter]){
                            string val = tokens.at(i);
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
                            string feature_val = tokens.at(i);
                            features.insert({feature, Feature(feature, feature_val)});
                        }
                        feature_counter_nominal++;
                        break;
                        

                        
                    case FeatureColumnNumerical:
                        {
                            string feature = features_names_numerical[feature_counter_numerical];
                            double feature_val = atof(tokens.at(i).c_str());
                            features.insert({feature, Feature(feature, feature_val)});
                        }
                        feature_counter_numerical++;
                        break;
                        
                    default:
                        break;
                }
            }
            
            loaded_lipidomes.push_back(new Lipidome(measurement, data_table_file));
            Lipidome *lipidome = loaded_lipidomes.back();
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
            }
            lipidome->original_intensities.reset(intensities);
        }
        
        // checking consistancy of features
        set<string> registered_features;
        registered_features.insert(FILE_FEATURE_NAME);
        for (auto feature : features_names_numerical) registered_features.insert(feature);
        for (auto feature : features_names_nominal) registered_features.insert(feature);
        for (auto lipidome : loaded_lipidomes){
            if (lipidome->cleaned_name == "-"){
                throw LipidSpaceException("Error, sample has no sample name.", CorruptedFileFormat);
            }
            
            for (auto feature : registered_features){
                if (uncontains_val(lipidome->features, feature)){
                    throw LipidSpaceException("Error, study variable '" + feature + "' not defined for sample '" + lipidome->cleaned_name + "'.", CorruptedFileFormat);
                }
            }
        }
        
        if (feature_values.size() > 1){
            for (auto feature : registered_features){
                if (uncontains_val(feature_values, feature)){
                    throw LipidSpaceException("Error, study variable '" + feature + "' is not registed already.", FeatureNotRegistered);
                }
            }
            
            for (auto kv : feature_values){
                if (uncontains_val(registered_features, kv.first)){
                    throw LipidSpaceException("Error, study variable '" + kv.first + "' is not present in imported file.", FeatureNotRegistered);
                }
            }
            
            // add new values into existing features
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->features){
                    FeatureSet &fs = feature_values[kv.first];
                    if (kv.second.feature_type == NumericalFeature){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        // register features
        else {
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->features){
                    if (uncontains_val(feature_values, kv.first)){
                        feature_values.insert({kv.first, FeatureSet(kv.first, kv.second.feature_type)});
                    }
                    FeatureSet &fs = feature_values[kv.first];
                    if (kv.second.feature_type == NumericalFeature){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
    
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            lipidomes.push_back(lipidome);
            selection[3].insert({lipidome->cleaned_name, true});
            
            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                selection[0].insert({lipidome->species.at(i), true});
                selection[1].insert({lipidome->classes.at(i), true});
                selection[2].insert({lipidome->categories.at(i), true});
            }
        }
        for (auto kv : lipid_set){
            if (uncontains_val(all_lipids, kv.first)) all_lipids.insert({kv.first, kv.second});
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
    
    delete column_types;
    fileLoaded();
}





void LipidSpace::load_row_table(string table_file, vector<TableColumnType> *column_types, string sheet){
    // load and parse lipid table, lipids per row, measurements per column
    Logging::write_log("Reading table '" + table_file + "' lipid row table.");
    ifstream infile(table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + table_file + "' not found.");
        exit(-1);
    }
    
    set<string> NA_VALUES = {"NA", "nan", "N/A", "0", "", "n/a", "NaN"};
    vector<Array> intensities;
    map<string, LipidAdduct*> lipid_set;
    vector<Lipidome*> loaded_lipidomes;
    
    try {
        
        
        FileTableHandler fth(table_file, sheet);
        
        // no specific column order is provided, we assume that first column contains
        // lipid species names and all remaining columns correspond to a sample containing
        // intensity / quant values
        if (column_types == 0){
            
            for (auto header : fth.headers){
                loaded_lipidomes.push_back(new Lipidome(header, table_file));
                intensities.push_back(Array());
            }
            
            for (auto tokens : fth.rows){
                LipidAdduct* l = load_lipid(tokens.at(0), lipid_set);
                
                if (l){
                
                    for (int i = 1; i < (int)tokens.size(); ++i){
                        string val = tokens.at(i);
                        if (!contains_val(NA_VALUES, val)){
                            Lipidome* lipidome = loaded_lipidomes.at(i - 1);
                            lipidome->lipids.push_back(l);
                            lipidome->species.push_back(l->get_lipid_string());
                            lipidome->classes.push_back(l->get_lipid_string(CLASS));
                            lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
                            intensities.at(i - 1).push_back(atof(val.c_str()));
                        }
                    }
                }
            }
        }
        
        else {
            bool has_lipid_col = false;
            for (auto column_type : *column_types){
                has_lipid_col |= (column_type == LipidColumn);
            }
            
            if (!has_lipid_col){
                throw LipidSpaceException("No lipid column was defined", NoColumnFound);
            }
            
            
            map<string, int> doublettes;
            for (int i = 0; i < (int)column_types->size(); ++i){
                if (column_types->at(i) == SampleColumn){
                    string header = fth.headers.at(i);
                    if (uncontains_val(doublettes, header)){
                        doublettes.insert({header, 1});
                    }
                    else {
                        header += "." + std::to_string(++doublettes[header]);
                    }
                    
                    selection[3].insert({header, true});
                    loaded_lipidomes.push_back(new Lipidome(header, table_file));
                    intensities.push_back(Array());
                } 
            }
                
            for (auto tokens : fth.rows){
                
                LipidAdduct* l = 0;
                vector<string> quant_data;
                // handle all remaining rows
                for (int i = 0; i < (int)column_types->size(); ++i){
                    switch(column_types->at(i)){
                        case SampleColumn:
                            quant_data.push_back(tokens.at(i));
                            break;
                            
                        case LipidColumn:
                            l = load_lipid(tokens.at(i), lipid_set);
                            break;
                            
                        default:
                            break;
                    }
                }
                if (l){
                    for (int i = 0; i < (int)quant_data.size(); ++i){
                        string val = quant_data[i];
                        if (!contains_val(NA_VALUES, val)){
                            Lipidome* lipidome = loaded_lipidomes.at(i);
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
            }
            
            delete column_types;
        }
        if (feature_values.size() > 1){
            throw LipidSpaceException("Error, study variables are already registered, table does not contain any variables and thus cannot be imported.", FeatureNotRegistered);
        }
        
        
        set<string> lipidome_names;
        for (auto lipidome : loaded_lipidomes){
            
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains multiple time the sample name '" + lm + "'.", CorruptedFileFormat);
            }
        }
        for (auto lipidome : lipidomes){
            string lm = lipidome->cleaned_name;
            if (uncontains_val(lipidome_names, lm)){
                lipidome_names.insert(lm);
            }
            else {
                throw LipidSpaceException("Error, table contains sample name '" + lm + "' which is already registered in LipidSpace.", CorruptedFileFormat);
            }
        }
        
        for (int i = 0; i < (int)loaded_lipidomes.size(); ++i){
            loaded_lipidomes.at(i)->original_intensities.reset(intensities.at(i));
        }
    
        // when all lipidomes are loaded successfully
        // they will be added to the global lipidome set
        for (auto lipidome : loaded_lipidomes){
            feature_values[FILE_FEATURE_NAME].nominal_values.insert({lipidome->features[FILE_FEATURE_NAME].nominal_value, true});
            
            lipidomes.push_back(lipidome);
            selection[3].insert({lipidome->cleaned_name, true});
            
            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                selection[0].insert({lipidome->species.at(i), true});
                selection[1].insert({lipidome->classes.at(i), true});
                selection[2].insert({lipidome->categories.at(i), true});
            }
        }
        for (auto kv : lipid_set){
            if (uncontains_val(all_lipids, kv.first)) all_lipids.insert({kv.first, kv.second});
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


LipidAdduct* LipidSpace::load_lipid(string lipid_name, map<string, LipidAdduct*> &lipid_set){
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
        }
    }
    
    if (l == nullptr || l->get_extended_class() == UNDEFINED_LIPID) {
        if (l){
            delete l;
            l = nullptr;
        }
        if (!ignore_unknown_lipids){
            throw LipidSpaceException("Lipid '" + lipid_name + "' cannot be parsed.", LipidUnparsable);
        }
        else {
            Logging::write_log("Ignoring unidentifiable lipid '" + lipid_name + "'");
        }
    }
    else if (uncontains_val(registered_lipid_classes, l->get_extended_class())) {
        if (l){
            delete l;
            l = nullptr;
        }
        if (!ignore_unknown_lipids){
            throw LipidSpaceException("Lipid '" + lipid_name + "' molecular structure is not registed in LipidSpace database.", LipidUnparsable);
        }
        else {
            Logging::write_log("Ignoring unidentifiable lipid '" + lipid_name + "'");
        }
    }
        
    if (l == nullptr) return l;
    
        
    // deleting adduct since not necessary
    if (l->adduct != nullptr){
        delete l->adduct;
        l->adduct = nullptr;
    }
    
    string translated_name = l->get_lipid_string();
    
    l->sort_fatty_acyl_chains();
    for (auto fa : l->lipid->fa_list) cut_cycle(fa);
    
    string lipid_string = l->get_lipid_string();
    if (uncontains_val(lipid_name_translations[TRANSLATED_NAME], lipid_string))
        lipid_name_translations[TRANSLATED_NAME].insert({lipid_string, translated_name});
    if (uncontains_val(lipid_name_translations[IMPORT_NAME], lipid_string))
        lipid_name_translations[IMPORT_NAME].insert({lipid_string, lipid_name});
    
    
    string lipid_unique_name = l->get_lipid_string();
    bool from_repository = false;
    if (contains_val(all_lipids, lipid_unique_name)){
        delete l;
        from_repository = true;
        l = all_lipids[lipid_unique_name];
    }
    
    if (uncontains_val(lipid_set, lipid_unique_name)){
        lipid_set.insert({lipid_unique_name, l});
    }
    else {
        if (!from_repository) delete l;
        l = nullptr;
        if (ignore_doublette_lipids){
            Logging::write_log("Ignoring doublette lipid '" + lipid_name + "'");
        }
        else {
            throw LipidSpaceException("Error: lipid '" + lipid_name + "' appears twice in the table.", LipidDoublette);
        }
    }
    return l;
}








void LipidSpace::store_distance_table(string output_folder, Lipidome* lipidome){
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


bool mysort(pair<double, int> a, pair<double, int> b){ return a.first < b.first;}

void LipidSpace::run(){
    if (process_id == 1){
        if (lipidomes.size() == 0){
            if (progress && !progress->stop_progress){
                progress->finish();
            }
            return;
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
        
        
        int num_for_PCA = 0;
        // compute PCA matrixes for the complete lipidome
        if (!progress || !progress->stop_progress){
            num_for_PCA = compute_global_distance_matrix();
            if (num_for_PCA == 0){
                if (progress){
                    progress->setError(QString("No lipids were taken for analysis. Analysis aborted. Select more lipids for analysis. Maybe consider the log messages."));
                    
                }
                return;
            }
        }
        
        if (num_for_PCA >= 3){
            cols_for_pca = min(cols_for_pca, (int)global_lipidome->lipids.size() - 1);
            
            // perform the principal component analysis
            if (!progress || !progress->stop_progress){
                // set the step size for the next analyses
                if (progress){
                    progress->prepare_steps(8);
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
        }
        else {
            global_lipidome->m.reset(selected_lipidomes.size(), global_lipidome->lipids.size());
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
            Logging::write_log("Finished analysis with a union of " + std::to_string(global_lipidome->lipids.size()) + " lipids among " + std::to_string(lipidomes.size()) + " lipidome" + (selected_lipidomes.size() > 1 ? "s" : "") + " in total.");
        }
    }
    
    else if (process_id == 2 && target_variable != ""){
        
        
        
        Array target_values;
        map<string, double> nominal_target_values;
        int nom_counter = 0;
        map<LipidAdduct*, int> lipid_map;
        map<string, int> lipid_name_map;
        Matrix global_matrix;
        vector< Gene* >genes;
        bool is_nominal = feature_values[target_variable].feature_type == NominalFeature;
        
        if (uncontains_val(feature_values, target_variable)){
            if (progress && !progress->stop_progress){
                progress->finish();
            }
            return;
        }
        Logging::write_log("Started feature analysis");
        
        if (progress){
            progress->set(0);
        }
        // perform the principal component analysis
        if (!progress || !progress->stop_progress){
            // set the step size for the next analyses
            
            // setup array for target variable values, if nominal then each with incrementing number
            if (is_nominal){
                for (auto lipidome : selected_lipidomes){
                    string nominal_value = lipidome->features[target_variable].nominal_value;
                    if (uncontains_val(nominal_target_values, nominal_value)){
                        nominal_target_values.insert({nominal_value, nom_counter++});
                    }
                    target_values.push_back(nominal_target_values[nominal_value]);
                }
            }
            else {
                for (auto lipidome : selected_lipidomes){
                    target_values.push_back(lipidome->features[target_variable].numerical_value);
                }
            }
            
            // setting up lipid to column in matrix map
            for (auto lipidome : selected_lipidomes){
                for (uint i = 0; i < lipidome->lipids.size(); ++i){
                    LipidAdduct* lipid = lipidome->lipids[i];
                    if (uncontains_val(lipid_map, lipid)){
                        lipid_map.insert({lipid, lipid_map.size()});
                        lipid_name_map.insert({lipidome->species[i], lipid_name_map.size()});
                    }
                }
            }
            
            if (is_nominal && nom_counter <= 1){
                if (progress && !progress->stop_progress){
                    progress->finish();
                }
                return;
            }
        
            // set up matrix for multiple linear regression
            global_matrix.reset(selected_lipidomes.size(), lipid_map.size());
            for (uint r = 0; r < selected_lipidomes.size(); ++r){
                Lipidome* lipidome = selected_lipidomes[r];
                for (uint i = 0; i < lipidome->lipids.size(); ++i){
                    global_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->original_intensities[i];
                }
            }
        }
        
        if (is_nominal){
            global_matrix.scale();
        }
        else {
            Array constants;
            constants.resize(global_matrix.rows, 1);
            global_matrix.add_column(constants);
        }
        
        
        // determining the upper number of features to consider
        int n = global_matrix.cols - !is_nominal; // -1 because we added a column of constant values 1, 1, 1, 1, ...
        int n_features = n;
        if (n > 1) n = min(n_features - 1, (int)sqrt(n) * 2);
        
        // perform the principal component analysis
        if (!progress || !progress->stop_progress){
            // set the step size for the next analyses
            if (progress){
                int m = n_features - n;
                progress->prepare_steps((((n_features + 1) * n_features) >> 1) - (((m + 1) * m) >> 1));
                progress->connect(&global_lipidome->m, SIGNAL(set_step()), progress, SLOT(set_step()));
            }
        
            genes.resize(n + 1, 0);
            genes[0] = new Gene(n_features + 1);
            if (!is_nominal) genes[0]->gene_code[n_features] = true; // add constant value column
            genes[0]->score = -1e100;
        
        // implementation of sequential forward feature selection
        
            if (progress){
                progress->set_step();
            }
        }
        
        for(int i = 1; i <= n && (!progress || !progress->stop_progress); ++i){
            Gene* best = 0;
            int pos = 0;
            double best_score = 1e100;
            Gene* last = genes[i - 1];
            while (pos < n_features - (i - 1) && (!progress || !progress->stop_progress)){
                if (!last->gene_code[pos]){
                    Gene* new_gene = new Gene(last);
                    new_gene->gene_code[pos] = true;
                
                    Indexes feature_indexes;
                    new_gene->get_indexes(feature_indexes);
                    Matrix sub_features;
                    sub_features.rewrite(global_matrix, {}, feature_indexes);
                    
                    if (is_nominal){
                        Array summed_values;
                        summed_values.resize(sub_features.rows);
                        
                        for (int c = 0; c < sub_features.cols; c++){
                        double *col = &(sub_features.m[c * sub_features.rows]);
                            for (int r = 0; r < sub_features.rows; ++r){
                                summed_values[r] += col[r];
                            }
                        }
                        
                        vector<Array> arrays(nom_counter);
                        for (int r = 0; r < sub_features.rows; ++r) arrays[target_values[r]].push_back(summed_values[r]);
                        
                        
                        double pos_max = 0, d = 0;
                        new_gene->score = 1;
                        for (uint ii = 0; ii < arrays.size() - 1; ++ii){
                            for (uint jj = ii + 1; jj < arrays.size(); ++jj){
                                ks_separation_value(arrays[ii], arrays[jj], d, pos_max);
                                new_gene->score *= d;
                            }
                        }
                        double ll = 2. / ((double)(arrays.size() - 1) * (double)arrays.size());
                        new_gene->score = 1 - pow(new_gene->score, ll);
                    }
                    else {
                        Array coefficiants;
                        coefficiants.compute_coefficiants(sub_features, target_values);    // estimating coefficiants
                        new_gene->score = compute_aic(sub_features, coefficiants, target_values);  // computing aic
                    }
                    
                    if (!best || best_score > new_gene->score){
                        best = new_gene;
                        best_score = new_gene->score;
                    }
                    else {
                        delete new_gene;
                    }
                    
                }
                ++pos;
                if (progress){
                    progress->set_step();
                }
            }
                
            genes[i] = best;
            
        }        
        
        if (!progress || !progress->stop_progress){
            // find the feature subset with the lowest best_score
            double best_score = 1e100;
            int best_pos = 0;
            for (uint i = 1; i < genes.size(); ++i){
                if (best_score > genes[i]->score){
                    best_score = genes[i]->score;
                    best_pos = i;
                }
            }
            
            // do the selection
            for (auto &kv : selection[0]){
                kv.second = contains_val(lipid_name_map, kv.first) ? genes[best_pos]->gene_code[lipid_name_map[kv.first]] : false;
            }
            
            // cleanup
            for (auto gene : genes){
                if (gene) delete gene;
            }
            
            if (progress && !progress->stop_progress){
                progress->finish();
            }
            emit reassembled();
        }
    }
    
    process_id = 0;
}



void LipidSpace::reset_analysis(){
    analysis_finished = false;
    feature_values.clear();
    feature_values.insert({"File", FeatureSet("File", NominalFeature)});
    if (dendrogram_root){
        delete dendrogram_root;
        dendrogram_root = 0;
    }
    selected_lipidomes.clear();
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    
    for (int i = 0; i < 4; ++i) selection[i].clear();
    
    for (auto kv : all_lipids) delete kv.second;
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




