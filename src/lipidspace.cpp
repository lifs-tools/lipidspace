#include "lipidspace/lipidspace.h"


void LipidSpace::create_dendrogram(){
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    lipid_sortings.clear();

    set<DendrogramNode*> nodes;
    for (int i = 0; i < hausdorff_distances.rows; ++i) nodes.insert(new DendrogramNode(i, &study_variable_values, selected_lipidomes[i]));
    for (auto node : nodes) node->update_distances(nodes, hausdorff_distances);

    while (nodes.size() > 1){
        DendrogramNode* min_node = 0;
        double min_depth = INFINITY;

        for (auto node : nodes){
            if (min_node == 0 || min_node->min_distance.first > node->min_distance.first || (min_node->min_distance.first == node->min_distance.first && (min_depth == INFINITY || (1 + __max(node->depth, node->min_distance.second->depth)) < min_depth))){
                min_node = node;
                min_depth = 1 + __max(node->depth, node->min_distance.second->depth);
            }
        }

        DendrogramNode* pair_node = min_node->min_distance.second;
        nodes.erase(min_node);
        nodes.erase(pair_node);
        DendrogramNode* new_node = new DendrogramNode(min_node, pair_node, min_node->min_distance.first);
        nodes.insert(new_node);
        new_node->update_distances(nodes, hausdorff_distances);

        for (auto node : nodes){
            if (node->min_distance.second && (node->min_distance.second == min_node || node->min_distance.second == pair_node)){
                node->update_distances(nodes, hausdorff_distances);
            }
        }
    }

    dendrogram_root = *nodes.begin();
    double* ret = dendrogram_root->execute(0, &dendrogram_points, &dendrogram_sorting);
    delete []ret;



    // determining importance of lipids based on goodness of separating
    // study variables applying simple 1D linear regression
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    Matrix global_matrix;

    vector<Lipidome*> lipidomes_for_feature_selection;
    for (auto lipidome :  lipidomes){
        bool select_lipidome = true;
        string lipidome_name = lipidome->cleaned_name;

        for (auto kv : lipidome->study_variables){
            string variable_name = kv.first;
            StudyVariable &study_variable = kv.second;
            if (study_variable.study_variable_type == NominalStudyVariable){
                if (!study_variable_values[variable_name].nominal_values[study_variable.nominal_value]){
                    select_lipidome = false;
                    break;
                }
            }
            else {
                double numerical_value = study_variable.numerical_value;
                auto filter = study_variable_values[variable_name].numerical_filter.first;
                auto filter_values = study_variable_values[variable_name].numerical_filter.second;
                switch (filter){

                    case LessFilter: // inverse logic, Less tells us, what to keep, greater we filter out
                        if (filter_values.size() < 1) continue;
                        if (filter_values[0] < numerical_value) select_lipidome = false;
                        break;

                    case GreaterFilter:
                        if (filter_values.size() < 1) continue;
                        if (filter_values[0] > numerical_value) select_lipidome = false;
                        break;

                    case EqualFilter:
                        {
                            if (filter_values.size() < 1) continue;
                            bool filtered_out = true; // < 1e-16 due to floating point rounding errors
                            for (double filter_value : filter_values) filtered_out &= !(fabs(numerical_value - filter_value) < 1e-16);
                            select_lipidome = !filtered_out;
                        }
                        break;

                    case WithinRange:
                        if (filter_values.size() < 2) continue;
                        if (numerical_value < filter_values[0] || filter_values[1] < numerical_value) select_lipidome = false;
                        break;

                    case OutsideRange:
                        if (filter_values.size() < 2) continue;
                        if (filter_values[0] <= numerical_value && numerical_value <= filter_values[1]) select_lipidome = false;
                        break;

                    default:
                        break;
                }
            }
        }


        if (select_lipidome && contains_val(selection[3], lipidome_name) && selection[3][lipidome_name]){
            lipidomes_for_feature_selection.push_back(lipidome);
        }
    }

    // setting up lipid to column in matrix map
    for (auto lipidome : lipidomes_for_feature_selection){
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            LipidAdduct* lipid = lipidome->lipids[i];
            if (uncontains_val(lipid_map, lipid)){
                lipid_map.insert({lipid, lipid_map.size()});
                lipid_name_map.insert({lipidome->species[i], lipid_name_map.size()});
            }
        }
    }

    // set up matrix for multiple linear regression
    global_matrix.reset(lipidomes_for_feature_selection.size(), lipid_map.size());
    for (uint r = 0; r < lipidomes_for_feature_selection.size(); ++r){
        Lipidome* lipidome = lipidomes_for_feature_selection[r];
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            global_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->normalized_intensities[i];
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
    SortVector<double, string> CVs;
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
    CVs.sort_desc();
    lipid_sortings.insert({"Coefficient of Variation (Desc)", SortVector<string, double>()});
    vector<pair<string, double>> &CVh = lipid_sortings["Coefficient of Variation (Desc)"];
    for (auto cv : CVs) CVh.push_back({cv.second, cv.first});



    // going through all study variables
    for (auto kv : study_variable_values){
        string target_variable = kv.first;

        Array target_values;
        SortVector<double, string> regression_result;
        map<string, double> nominal_target_values;
        int nom_counter = 0;
        bool is_nominal = study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

        if (uncontains_val(study_variable_values, target_variable)){
            return;
        }

        // setup array for target variable values, if nominal then each with incrementing number
        if (is_nominal){
            for (auto lipidome : lipidomes_for_feature_selection){
                string nominal_value = lipidome->study_variables[target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                }
                target_values.push_back(nominal_target_values[nominal_value]);
            }
        }
        else {
            for (auto lipidome : lipidomes_for_feature_selection){
                target_values.push_back(lipidome->study_variables[target_variable].numerical_value);
            }
        }

        // going through all lipids
        for (auto kv : lipid_name_map){
            int c = kv.second;
            if (is_nominal){
                vector<Array> arrays(nom_counter);
                double num_lipid_in_lipidomes = 0;

                for (int r = 0; r < global_matrix.rows; ++r){
                    if (global_matrix(r, c) <= 1e-15) continue;
                    num_lipid_in_lipidomes += 1;
                    arrays[target_values[r]].push_back(global_matrix(r, c));
                }
                double acc = compute_accuracy(arrays);
                acc = __abs(1. / (double)nom_counter - acc) + 1. / (double)nom_counter;
                double score = acc * num_lipid_in_lipidomes / (double)lipidomes_for_feature_selection.size();
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
        regression_result.sort_desc();
        string target_regression_classification = target_variable + (is_nominal ? " classification" : " regression") + " (Desc)";
        lipid_sortings.insert({target_regression_classification, SortVector<string, double>()});
        SortVector<string, double> &regression_vector = lipid_sortings[target_regression_classification];
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
    process_id = 0;
    target_variable = "";
    study_variable_values.insert({FILE_STUDY_VARIABLE_NAME, StudyVariableSet(FILE_STUDY_VARIABLE_NAME, NominalStudyVariable)});
    lipid_name_translations.resize(3);



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

        registered_lipid_classes.insert(tokens->at(0));
        registered_lipid_classes.insert(tokens->at(1));
        string key = tokens->at(0) + "/" + tokens->at(1);
        string key2 = tokens->at(1) + "/" + tokens->at(0);
        class_matrix.insert({key, pair<int, int>{abs(atoi(tokens->at(2).c_str())), abs(atoi(tokens->at(3).c_str()))}});
        class_matrix.insert({key2, pair<int, int>{abs(atoi(tokens->at(2).c_str())), abs(atoi(tokens->at(3).c_str()))}});
        delete tokens;
    }
}


LipidSpace::LipidSpace(LipidSpace *ls){
    progress = ls->progress;
    dendrogram_root = 0;

    for (auto kv : ls->class_matrix) class_matrix.insert({kv.first, pair<int, int>{kv.second.first, kv.second.second}});
    for (auto kv : ls->all_lipids) all_lipids.insert({kv.first, new LipidAdduct(kv.second)});
    keep_sn_position = ls->keep_sn_position;
    ignore_unknown_lipids = ls->ignore_unknown_lipids;
    ignore_doublette_lipids = ls->ignore_doublette_lipids;
    unboundend_distance = ls->unboundend_distance;
    without_quant = ls->without_quant;
    map<Lipidome*, Lipidome*> lipidome_map;
    for (auto lipidome : ls->lipidomes){
        Lipidome *new_lipidome = new Lipidome(lipidome);
        for (auto species : new_lipidome->species) new_lipidome->lipids.push_back(all_lipids[species]);
        lipidomes.push_back(new_lipidome);
        lipidome_map.insert({lipidome, new_lipidome});
    }
    for (auto translation : ls->lipid_name_translations){
        lipid_name_translations.push_back(map<string, string>());
        for (auto kv : translation) lipid_name_translations.back().insert({kv.first, kv.second});
    }
    global_lipidome = (ls->global_lipidome != 0) ? new Lipidome(ls->global_lipidome) : 0;
    hausdorff_distances.rewrite(ls->hausdorff_distances);
    analysis_finished = false;
    for (auto kv : ls->study_variable_values) study_variable_values.insert({kv.first, StudyVariableSet(&kv.second)});
    for (int i = 0; i < 4; ++i){
        for (auto kv : ls->selection[i]) selection[i].insert({kv.first, kv.second});
    }
    for (auto lipidome : ls->selected_lipidomes) selected_lipidomes.push_back(lipidome_map[lipidome]);
    global_distances.rewrite(ls->global_distances);
    process_id = ls->process_id;
    target_variable = ls->target_variable;
    for (auto value : ls->registered_lipid_classes) registered_lipid_classes.insert(value);
}





const int LipidSpace::cols_for_pca_init = 7;
int LipidSpace::cols_for_pca = 7;


const vector< vector< vector< pair<int, int> > > > LipidSpace::orders {
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
    delete global_lipidome;
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
}








void LipidSpace::lipid_similarity(LipidAdduct* lipid1, LipidAdduct* lipid2, int& union_num, int& inter_num){

    string key = lipid1->get_extended_class() + "/" + lipid2->get_extended_class();

    if (uncontains_val(class_matrix, key)){
        throw LipidSpaceException("Error: key '" + key + "' not in precomputed class matrix. Please check the log message.", LipidNotRegistered);
    }
    union_num = class_matrix[key].first;
    inter_num = class_matrix[key].second;

    if (lipid1->lipid->info->level <= CLASS || lipid2->lipid->info->level <= CLASS) return;

    vector<FattyAcid*>* orig_fa_list_1 = new vector<FattyAcid*>();
    vector<FattyAcid*>* orig_fa_list_2 = new vector<FattyAcid*>();


    // if two lipids have different level, take information on species level
    if ((lipid1->lipid->info->level == SPECIES && lipid2->lipid->info->level > SPECIES) | (lipid1->lipid->info->level > SPECIES && lipid2->lipid->info->level == SPECIES)){
        orig_fa_list_1->push_back(lipid1->lipid->info);
        orig_fa_list_2->push_back(lipid2->lipid->info);
    }
    else {
        if (lipid1->lipid->info->level >= MOLECULAR_SPECIES){
            for (auto fa : lipid1->lipid->fa_list){
                if (fa->num_carbon > 0)
                    orig_fa_list_1->push_back(fa);
            }
        }
        else {
            orig_fa_list_1->push_back(lipid1->lipid->info);
        }

        if (lipid2->lipid->info->level >= MOLECULAR_SPECIES){
            for (auto fa : lipid2->lipid->fa_list){
                if (fa->num_carbon > 0)
                    orig_fa_list_2->push_back(fa);

            }
        }
        else {
            orig_fa_list_2->push_back(lipid2->lipid->info);
        }
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
                int* cell = cache[order.first * len_fa1 + order.second];
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


    // setup a complete list of all nominal study variables and their values
    map<string, set<string>> delete_nominal_study_variables;
    for (auto kv : study_variable_values){
        if (kv.second.study_variable_type == NominalStudyVariable){
            if (uncontains_val(delete_nominal_study_variables, kv.first)) delete_nominal_study_variables.insert({kv.first, set<string>()});
            for (auto kv_nom : kv.second.nominal_values){
                delete_nominal_study_variables[kv.first].insert(kv_nom.first);
            }
        }
    }

    // remove all study variables and values that remain in the analysis
    for (auto lipidome : lipidomes){
        for (auto kv : lipidome->study_variables){
            if (contains_val(delete_nominal_study_variables, kv.first) && kv.second.study_variable_type == NominalStudyVariable){
                delete_nominal_study_variables[kv.first].erase(kv.second.nominal_value);
            }
        }
    }

    // delete remaining study_variable values or even whole study_variables
    for (auto kv : delete_nominal_study_variables){
        if (kv.second.size() == study_variable_values[kv.first].nominal_values.size()){ // delete whole study_variable
            study_variable_values.erase(kv.first);
        }
        else { // delete just the values
            for (auto study_variable_value : delete_nominal_study_variables[kv.first]){
                study_variable_values[kv.first].nominal_values.erase(study_variable_value);
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
        if (contains_val(lipid_name_translations[NORMALIZED_NAME], lipid_name_translations[TRANSLATED_NAME][lipid_name])){
            lipid_name_translations[NORMALIZED_NAME].erase(lipid_name_translations[TRANSLATED_NAME][lipid_name]);
        }
        if (contains_val(lipid_name_translations[NORMALIZED_NAME], lipid_name_translations[IMPORT_NAME][lipid_name])){
            lipid_name_translations[NORMALIZED_NAME].erase(lipid_name_translations[IMPORT_NAME][lipid_name]);
        }

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

    Lipidome* lipidome = new Lipidome(lipid_list_file, lipid_list_file, "", true);
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

        if (study_variable_values.size() > 1){
            throw LipidSpaceException("Study variables have been loaded. Lists do not supported any study variable import routine. Please reset LipidSpace.", StudyVariableNotRegistered);
        }
        study_variable_values[FILE_STUDY_VARIABLE_NAME].nominal_values.insert({lipidome->study_variables[FILE_STUDY_VARIABLE_NAME].nominal_value, true});


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





// An efficient computational algorithm for Hausdorff distance based on points-ruling-out and systematic random sampling
// Jegoon Ryu, Sei-ichiro Kamata
// DOI: 10.1016/j.patcog.2021.107857
double LipidSpace::compute_hausdorff_distance(Matrix &m1, Matrix &m2){
    double cmax = 0;
    const int rows = m1.rows;

    Array U(m1.cols, INFINITY);
    Array V(m2.cols, INFINITY);


    int delta = max(1, m2.cols >> 3);
    int osp = rand() % delta;
    double isp = m1.cols >> 1;
    for (int x = osp; x < m2.cols; x += delta){
        double cmin = INFINITY;
        double dist1 = INFINITY;
        double dist2 = INFINITY;
        const double* m2col = m2.data() + (x * rows);

        for (int y = isp - 1, z = isp; y >= 0 || z < m1.cols; y--, z++){
            if (y >= 0){
                const double* m1col = m1.data() + (y * rows);
                dist1 = compute_l2_norm(m1col, m2col, rows);
                if (dist1 < U[y]) U[y] = dist1;
            }

            if (z < m2.cols) {
                const double* m1col = m1.data() + (z * rows);
                dist2 = compute_l2_norm(m1col, m2col, rows);
                if (dist2 < U[z]) U[z] = dist2;
            }
            double dist = __min(dist1, dist2);
            if (dist < V[x]) V[x] = dist;
            cmin = __min(cmin, dist);

            if (cmin <= cmax){
                isp = dist1 < dist2 ? y : z;
                break;
            }
        }
        if (cmin > cmax) cmax = cmin;
    }


    Indexes Ai;
    for (uint i = 0; i < U.size(); ++i){
        if (U[i] > cmax) Ai.push_back(i);
    }

    isp = m2.cols >> 1;
    for (int x : Ai){
        double cmin = INFINITY;
        double dist1 = INFINITY;
        double dist2 = INFINITY;

        const double* m1col = m1.data() + (x * rows);
        for (int y = isp - 1, z = isp; y >= 0 || z < m2.cols; y--, z++){
            if (y >= 0){
                const double* m2col = m2.data() + (y * rows);
                dist1 = compute_l2_norm(m1col, m2col, rows);
                if (dist1 < V[y]) V[y] = dist1;
            }

            if (z < m2.cols) {
                const double* m2col = m2.data() + (z * rows);
                dist2 = compute_l2_norm(m1col, m2col, rows);
                if (dist2 < V[z]) V[z] = dist2;
            }

            cmin = __min(__min(dist1, dist2), cmin);
            if (cmin <= cmax || cmin == 0){
                isp = dist1 < dist2 ? y : z;
                break;
            }
        }
        cmax = __max(cmax, cmin);
    }


    Indexes Bi;
    for (uint i = 0; i < V.size(); ++i){
        if (V[i] > cmax) Bi.push_back(i);
    }

    isp = m1.cols >> 1;
    for (int x = 0; x < m2.cols; ++x){
        double cmin = INFINITY;
        double dist1 = INFINITY;
        double dist2 = INFINITY;

        const double* m2col = m2.data() + (x * rows);
        for (int y = isp - 1, z = isp; y >= 0 || z < m1.cols; y--, z++){
            if (y >= 0){
                const double* m1col = m1.data() + (y * rows);
                dist1 = compute_l2_norm(m1col, m2col, rows);
            }

            if (z < m2.cols) {
                const double* m1col = m1.data() + (z * rows);
                dist2 = compute_l2_norm(m1col, m2col, rows);
            }

            cmin = __min(__min(dist1, dist2), cmin);
            if (cmin <= cmax || cmin == 0){
                isp = dist1 < dist2 ? y : z;
                break;
            }
        }
        cmax = __max(cmax, cmin);
    }

    return cmax;
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
    int N = (n * (n - 1)) >> 1;
    ulong *pairs = new ulong[N];
    int k = 0;
    for (ulong i = 0; i < (ulong)n - 1; ++i){
        ulong ii = i << 32;
        for (ulong j = i + 1; j < (ulong)n; ++j){
            pairs[k++] = ii | j;
        }
    }


    #pragma omp parallel for
    for (int ii = 0; ii < N; ++ii){
        int i = (int)(pairs[ii] >> 32);
        int j = (int)(pairs[ii] & MASK32);

        double hd = sqrt(compute_hausdorff_distance(*matrixes[i], *matrixes[j]));
        hausdorff_distances(i, j) = hd;
        hausdorff_distances(j, i) = hd;
    }

    delete []pairs;
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
    global_lipidome->selected_lipid_indexes.clear();
    global_lipidome->normalized_intensities.clear();
    global_lipidome->visualization_intensities.clear();
    global_lipidome->PCA_intensities.clear();
    global_lipidome->m.reset(1, 1);

    for (auto lipidome : study_lipidomes) delete lipidome;
    study_lipidomes.clear();



    set<string> registered_lipids;

    for (auto lipidome : lipidomes){
        // check if lipidome was (de)selected for analysis
        if (!selection[SAMPLE_ITEM][lipidome->cleaned_name]) continue;

        // check if lipidome is being excluded by deselected study variable(s)
        bool filtered_out = false;
        for (auto kv : lipidome->study_variables){
            if (kv.second.study_variable_type == NominalStudyVariable){
                if (uncontains_val(study_variable_values, kv.first) || uncontains_val(study_variable_values[kv.first].nominal_values, kv.second.nominal_value) || !study_variable_values[kv.first].nominal_values[kv.second.nominal_value]) {
                    filtered_out = true;
                    break;
                }
            }
            else {
                if (uncontains_val(study_variable_values, kv.first)) continue;
                vector<double> &filter_values = study_variable_values[kv.first].numerical_filter.second;
                double numerical_value = kv.second.numerical_value;
                switch (study_variable_values[kv.first].numerical_filter.first){

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


    global_lipidome->original_intensities.resize(n, 1);
    global_lipidome->visualization_intensities.resize(n, 1);
    global_lipidome->normalized_intensities.resize(n, 1);
    global_lipidome->PCA_intensities.resize(n, 1);

    // compute distances
    Matrix& distance_matrix = global_lipidome->m;
    distance_matrix.reset(n, n);


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
        lipidome->visualization_intensities.clear();
        lipidome->normalized_intensities.clear();
        lipidome->PCA_intensities.clear();
        lipidome->selected_lipid_indexes.clear();
    }
    global_lipidome->selected_lipid_indexes.clear();



    map<string, int> lipid_indexes;
    map<string, Array> global_lipid_intensities;
    for (int i = 0; i < (int)global_lipidome->species.size(); ++i){
        string lipid_species = global_lipidome->species.at(i);
        lipid_indexes.insert({lipid_species, i});
        global_lipid_intensities.insert({lipid_species, Array()});
        global_lipidome->selected_lipid_indexes.push_back(i);
    }



    vector<int> remove_lipidomes;
    int l = 0;
    for (auto lipidome : selected_lipidomes){
        Indexes selected_lipid_indexes;
        for (int i = 0; i < (int)lipidome->species.size(); ++i){
            string lipid_species = lipidome->species[i];
            lipidome->normalized_intensities.push_back(lipidome->original_intensities[i]);
            if (!(selection[SPECIES_ITEM][lipid_species] && contains_val(lipid_indexes, lipid_species))) continue;
            selected_lipid_indexes.push_back(lipid_indexes.at(lipid_species));
            lipidome->selected_lipid_indexes.push_back(i);
            lipidome->visualization_intensities.push_back(lipidome->original_intensities[i]);
            lipidome->PCA_intensities.push_back(lipidome->original_intensities[i]);
            if (contains_val(global_lipid_intensities, lipid_species)) global_lipid_intensities[lipid_species].push_back(lipidome->original_intensities[i]);
        }
        if (selected_lipid_indexes.size() > 0){
            lipidome->m.rewrite(global_lipidome->m, selected_lipid_indexes);
        }
        else {
            remove_lipidomes.push_back(l);
        }
        l++;
    }
    for (int i = (int)remove_lipidomes.size() - 1; i >= 0; --i) selected_lipidomes.erase(selected_lipidomes.begin() + remove_lipidomes[i]);


    // compute mean intensities for global lipidome
    for (uint i = 0; i < global_lipidome->lipids.size(); ++i){
        string lipid_species = global_lipidome->species[i];
        global_lipidome->original_intensities[i] = global_lipid_intensities[lipid_species].mean();
        global_lipidome->normalized_intensities[i] = global_lipid_intensities[lipid_species].mean();
    }


    if (contains_val(study_variable_values, FILE_STUDY_VARIABLE_NAME)){
        for (auto kv : study_variable_values[FILE_STUDY_VARIABLE_NAME].nominal_values){
            if (!kv.second) continue;

            string study = kv.first;
            //map<string, Array> study_lipid_intensities;
            //for (auto kv : global_lipid_intensities) study_lipid_intensities.insert({kv.first, Array()});
            Lipidome* study_lipidome = new Lipidome("Study lipidome - " + study, "");
            set<int> selected_lipid_indexes_set;
            for (auto lipidome : selected_lipidomes){
                if (lipidome->study_variables[FILE_STUDY_VARIABLE_NAME].nominal_value != study) continue;

                for (int i = 0; i < (int)lipidome->species.size(); ++i){
                    string lipid_species = lipidome->species[i];
                    if (!(selection[SPECIES_ITEM][lipid_species] && contains_val(lipid_indexes, lipid_species))) continue;
                    selected_lipid_indexes_set.insert(lipid_indexes.at(lipid_species));
                    //if (contains_val(study_lipid_intensities, lipid_species)) study_lipid_intensities[lipid_species].push_back(lipidome->original_intensities[i]);
                }
            }
            if (selected_lipid_indexes_set.empty()){
                delete study_lipidome;
            }
            else {
                Indexes selected_lipid_indexes;
                for (int i : selected_lipid_indexes_set){
                    selected_lipid_indexes.push_back(i);
                    study_lipidome->lipids.push_back(global_lipidome->lipids[i]);
                    study_lipidome->species.push_back(global_lipidome->species[i]);
                    study_lipidome->classes.push_back(global_lipidome->classes[i]);
                    study_lipidome->categories.push_back(global_lipidome->categories[i]);
                    //study_lipidome->original_intensities.push_back(study_lipid_intensities[global_lipidome->species[i]].mean());
                    //study_lipidome->normalized_intensities.push_back(study_lipid_intensities[global_lipidome->species[i]].mean());

                }
                int n = study_lipidome->lipids.size();
                for (int i = 0; i < n; ++i) study_lipidome->selected_lipid_indexes.push_back(i);
                study_lipidome->original_intensities.resize(n, 1);
                study_lipidome->visualization_intensities.resize(n, 1);
                study_lipidome->normalized_intensities.resize(n, 1);
                study_lipidome->PCA_intensities.resize(n, 1);
                study_lipidome->m.rewrite(global_lipidome->m, selected_lipid_indexes);

                study_lipidomes.push_back(study_lipidome);
            }
        }
    }
}




inline double gauss(double x, double mue, double sigma){
    return exp(-0.5 * sq((x - mue) / sigma)) / sqrt(2 * M_PI);
}



void LipidSpace::normalize_intensities(){

    int n = global_lipidome->m.rows;
    double global_mean = 0;
    double global_stdev = 0;
    double max_value = 0;

    if (global_lipidome->lipids.size() >= 3){
        for (int i = 0; i < n; ++i){
            global_mean += global_lipidome->m.m[i];
        }
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

            for (uint i = 0; i < lipidome->PCA_intensities.size(); ++i){
                lipidome->PCA_intensities[i] = global_stdev * (lipidome->PCA_intensities[i] - values_mean) / values_std;
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
            for (uint i = 0; i < lipidome->normalized_intensities.size(); ++i){
                lipidome->normalized_intensities[i] = global_stdev * lipidome->normalized_intensities[i] / values_std;
            }

            for (uint i = 0; i < lipidome->PCA_intensities.size(); ++i){
                lipidome->PCA_intensities[i] = global_stdev * (lipidome->PCA_intensities[i] - values_mean) / values_std;
                //lipidome->visualization_intensities[i] = 100. * lipidome->visualization_intensities[i] / values_std;
            }
        }
    }

    else {
        string fv = GlobalData::normalization;
        if (uncontains_val(study_variable_values, fv)) return;
        StudyVariableSet &fs = study_variable_values.at(fv);

        for (auto kv : fs.nominal_values){
            Array data;
            for (auto lipidome : selected_lipidomes){
                if (uncontains_val(lipidome->study_variables, fv) || lipidome->study_variables.at(fv).nominal_value != kv.first) continue;
                for (auto val : lipidome->original_intensities) {
                    if (val > 0) data.push_back(val);
                }
            }

            double values_mean = data.mean();
            double values_std = data.stdev();
            if (values_std < 1e-19) continue;

            for (auto lipidome : selected_lipidomes){
                if (uncontains_val(lipidome->study_variables, fv) || lipidome->study_variables.at(fv).nominal_value != kv.first) continue;

                for (int i = 0; i < (int)lipidome->normalized_intensities.size(); ++i){
                    lipidome->normalized_intensities[i] = global_stdev * lipidome->normalized_intensities[i] / values_std;
                }

                for (int i = 0; i < (int)lipidome->PCA_intensities.size(); ++i){
                    lipidome->PCA_intensities[i] = global_stdev * (lipidome->PCA_intensities[i] - values_mean) / values_std;
                    //lipidome->visualization_intensities[i] = 100. * lipidome->visualization_intensities[i] / values_std;
                }
            }
        }
    }


    for (auto lipidome : selected_lipidomes){
        for (int i = 0; i < (int)lipidome->visualization_intensities.size(); ++i){
            int index = lipidome->selected_lipid_indexes[i];
            max_value = max(max_value, lipidome->normalized_intensities[index]);
        }
    }


    for (auto lipidome : selected_lipidomes){
        for (int i = 0; i < (int)lipidome->visualization_intensities.size(); ++i){
            int index = lipidome->selected_lipid_indexes[i];
            //lipidome->visualization_intensities[i] = max(10 * log(lipidome->normalized_intensities[index] / max_value + 1.), 0.2);
            lipidome->visualization_intensities[i] = max(10 * sqrt(sqrt(lipidome->normalized_intensities[index] / max_value)), 0.2);
        }
    }



    // apply for all study lipidomes
    map<string, Array> global_vis_intensities;
    map<string, Array> global_norm_intensities;
    for (auto lipid_species : global_lipidome->species) global_vis_intensities.insert({lipid_species, Array()});
    for (auto lipid_species : global_lipidome->species) global_norm_intensities.insert({lipid_species, Array()});

    for (auto study : study_lipidomes){
        map<string, Array> study_norm_intensities;
        map<string, Array> study_vis_intensities;
        for (auto lipid_species : global_lipidome->species) study_norm_intensities.insert({lipid_species, Array()});
        for (auto lipid_species : global_lipidome->species) study_vis_intensities.insert({lipid_species, Array()});

        for (auto lipidome : selected_lipidomes){
            if (uncontains_val(lipidome->study_variables, FILE_STUDY_VARIABLE_NAME) || "Study lipidome - " + lipidome->study_variables[FILE_STUDY_VARIABLE_NAME].nominal_value != study->cleaned_name) continue;

            for (uint i = 0; i < lipidome->selected_lipid_indexes.size(); ++i){
                string lipid_species = lipidome->species[lipidome->selected_lipid_indexes[i]];
                study_vis_intensities[lipid_species].push_back(lipidome->visualization_intensities[i]);
                global_vis_intensities[lipid_species].push_back(lipidome->visualization_intensities[i]);
            }

            for (uint i = 0; i < lipidome->normalized_intensities.size(); ++i){
                string lipid_species = lipidome->species[i];
                study_norm_intensities[lipid_species].push_back(lipidome->normalized_intensities[i]);
                global_norm_intensities[lipid_species].push_back(lipidome->normalized_intensities[i]);
            }
        }

        for (uint i = 0; i < study->selected_lipid_indexes.size(); ++i){
            int index = study->selected_lipid_indexes[i];
            string lipid_species = study->species[index];
            study->visualization_intensities[i] = study_vis_intensities[lipid_species].mean();
        }

        for (uint i = 0; i < study->normalized_intensities.size(); ++i){
            string lipid_species = study->species[i];
            study->normalized_intensities[i] = study_norm_intensities[lipid_species].mean();
        }
    }


    for (uint i = 0; i < global_lipidome->selected_lipid_indexes.size(); ++i){
        int index = global_lipidome->selected_lipid_indexes[i];
        string lipid_species = global_lipidome->species[index];
        global_lipidome->visualization_intensities[i] = global_vis_intensities[lipid_species].mean();
    }

    for (uint i = 0; i < global_lipidome->normalized_intensities.size(); ++i){
        string lipid_species = global_lipidome->species[i];
        global_lipidome->normalized_intensities[i] = global_norm_intensities[lipid_species].mean();
    }
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

    ifstream infile(mzTabM_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + mzTabM_file + "' could not be found.", FileUnreadable);
    }
    vector<Lipidome*> loaded_lipidomes;
    map<int, string> study_variable_names;

    vector<StudyVariable> headers;
    map<string, LipidAdduct*> lipid_set;

    vector<string> *tokens = 0;
    vector<string> *sample_data = 0;
    vector<string> *content_tokens = 0;
    map<string, StudyVariableType> study_study_variable_types;

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
                        if (uncontains_val(study_variable_names, content_number)){
                            study_variable_names.insert({content_number, key});
                        }
                        else if (study_variable_names[content_number] != key){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", different study variable key than already used, '" + study_variable_names[content_number] + "' vs. '" + key + "'", CorruptedFileFormat);
                        }

                        // is custom value a number?
                        StudyVariableType ft = NominalStudyVariable;
                        if (is_double(value)){
                            bool is_not_missing = false;
                            double val = QString(value.c_str()).toDouble(&is_not_missing);
                            loaded_lipidomes[sample_number - 1]->study_variables.insert({key, StudyVariable(key, val, !is_not_missing)});
                            ft = NumericalStudyVariable;
                        }
                        else {
                            loaded_lipidomes[sample_number - 1]->study_variables.insert({key, StudyVariable(key, value, contains_val(NA_VALUES, value))});
                        }

                        if (uncontains_val(study_study_variable_types, key)){
                            study_study_variable_types.insert({key, ft});
                        }
                        else {
                            if (study_study_variable_types[key] != ft){
                                throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", key of study variable '" + key + "' is already registered with different data type (string, number).", CorruptedFileFormat);
                            }
                        }
                    }
                    else if (content_name == "species"){
                        if (content_tokens->size() < 3 || content_tokens->at(2).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = "Taxonomic species";
                        if (uncontains_val(study_variable_names, -1)){
                            study_variable_names.insert({-1, key});
                        }
                        string value = content_tokens->at(2);
                        loaded_lipidomes[sample_number - 1]->study_variables.insert({key, StudyVariable(key, strip(value, '"'))});
                    }
                    else if (content_name == "tissue"){
                        if (content_tokens->size() < 3 || content_tokens->at(2).size() == 0){
                            throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", line is corrupted.", CorruptedFileFormat);
                        }
                        string key = "Tissue";
                        if (uncontains_val(study_variable_names, -2)){
                            study_variable_names.insert({-2, key});
                        }
                        string value = content_tokens->at(2);
                        loaded_lipidomes[sample_number - 1]->study_variables.insert({key, StudyVariable(key, strip(value, '"'))});
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
                        headers.push_back(StudyVariable("[s]", sample_number));
                    }
                    else {
                        headers.push_back(StudyVariable(header, ""));
                    }
                }
            }

            if (tokens->size() >= 1 && tokens->at(0) == "SML"){
                if (tokens->size() > headers.size()){
                    throw LipidSpaceException("Error in line " + std::to_string(line_num) + ", more elements (" + std::to_string(tokens->size()) + ") than defined in header (" + std::to_string(headers.size()) + ").", CorruptedFileFormat);
                }

                LipidAdduct *l = 0;

                for (uint i = 0; i < tokens->size(); ++i){
                    StudyVariable &f = headers[i];
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

        // checking consistancy of study variables
        set<string> registered_study_variables;
        registered_study_variables.insert(FILE_STUDY_VARIABLE_NAME);
        for (auto kv : study_variable_names) registered_study_variables.insert(kv.second);
        for (auto lipidome : loaded_lipidomes){
            if (lipidome->cleaned_name == "-"){
                throw LipidSpaceException("Error, sample has no sample name.", CorruptedFileFormat);
            }

            for (auto study_variable : registered_study_variables){
                if (uncontains_val(lipidome->study_variables, study_variable)){
                    throw LipidSpaceException("Error, study variable '" + study_variable + "' not defined for sample '" + lipidome->cleaned_name + "'.", CorruptedFileFormat);
                }
            }
        }

        if (study_variable_values.size() > 1){
            for (auto study_variable : registered_study_variables){
                if (uncontains_val(study_variable_values, study_variable)){
                    throw LipidSpaceException("Error, study variable '" + study_variable + "' is not registered, yet.", StudyVariableNotRegistered);
                }
            }

            for (auto kv : study_variable_values){
                if (uncontains_val(registered_study_variables, kv.first)){
                    throw LipidSpaceException("Error, study variable '" + kv.first + "' is not present in imported file.", StudyVariableNotRegistered);
                }
            }

            // add new values into existing study variables
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->study_variables){
                    StudyVariableSet &fs = study_variable_values[kv.first];
                    if (kv.second.study_variable_type == NumericalStudyVariable){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        // register study variables
        else {
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->study_variables){
                    if (uncontains_val(study_variable_values, kv.first)){
                        study_variable_values.insert({kv.first, StudyVariableSet(kv.first, kv.second.study_variable_type)});
                    }
                    StudyVariableSet &fs = study_variable_values[kv.first];
                    if (kv.second.study_variable_type == NumericalStudyVariable){
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



void LipidSpace::load_flat_table(ImportData *import_data){
    vector<TableColumnType> *column_types = import_data->column_types;
    FileTableHandler *fth = import_data->file_table_handler;
    MappingData *mapping_data = import_data->mapping_data;
    string flat_table_file = import_data->table_file;
    string sheet = import_data->sheet;
    Logging::write_log("Importing table '" + flat_table_file + "' as flat table.");

    vector<Lipidome*> loaded_lipidomes;
    set<string> registered_study_variables;

    try {
        vector<int> sample_columns;
        int lipid_species_column = -1;
        int quant_column = -1;
        set<string> index_key_pairs;
        vector<int> study_variable_columns_numerical;
        vector<int> study_variable_columns_nominal;

        for (int i = 0; i < (int)column_types->size(); ++i) {
            switch(column_types->at(i)){
                case StudyVariableColumnNominal: study_variable_columns_nominal.push_back(i); break;
                case StudyVariableColumnNumerical: study_variable_columns_numerical.push_back(i); break;
                case SampleColumn: sample_columns.push_back(i); break;
                case LipidColumn: lipid_species_column = i; break;
                case QuantColumn: quant_column = i; break;
                default: break;
            }
        }

        if (sample_columns.size() == 0 || lipid_species_column == -1 || quant_column == -1){
            throw LipidSpaceException("Error while loading flat table '" + flat_table_file + "': not all essential columns defined.", NoColumnFound);
        }

        map<string, LipidAdduct*> lipid_map;
        map<string, Lipidome*> lipidome_map;
        vector<string> study_variable_names_nominal;
        vector<string> study_variable_names_numerical;
        map<string, LipidAdduct*> load_lipids;

        for (auto fi : study_variable_columns_nominal){
            string study_variable = fth->headers.at(fi);
            study_variable_names_nominal.push_back(study_variable);

            if (mapping_data != 0 && contains_val(mapping_data->at(NominalStudyVariable), study_variable)){
                Mapping &m = mapping_data->at(NominalStudyVariable)[study_variable];
                if (m.action == RenameAction || m.action == MappingTo) study_variable = m.mapping;
            }
            registered_study_variables.insert(study_variable);
        }
        for (auto fi : study_variable_columns_numerical){
            string study_variable = fth->headers.at(fi);
            study_variable_names_numerical.push_back(study_variable);
            if (mapping_data != 0 && contains_val(mapping_data->at(NumericalStudyVariable), study_variable)){
                Mapping &m = mapping_data->at(NumericalStudyVariable)[study_variable];
                if (m.action == RenameAction || m.action == MappingTo) study_variable = m.mapping;
            }
            registered_study_variables.insert(study_variable);
        }


        for (auto tokens : fth->rows){

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
                lipidome = new Lipidome(sample_name, flat_table_file, sheet);
                lipidome_map.insert({sample_name, lipidome});
                loaded_lipidomes.push_back(lipidome);
            }
            else {
                lipidome = lipidome_map[sample_name];
            }

            // handle study variables
            for (int i = 0; i < (int)study_variable_columns_nominal.size(); ++i){
                string study_variable_name = study_variable_names_nominal[i];
                string study_variable_value = tokens.at(study_variable_columns_nominal[i]);

                if (contains_val(NA_VALUES, study_variable_value)) study_variable_value = NO_VALUE_CHAR;

                if (mapping_data != 0 && contains_val(mapping_data->at(NominalStudyVariable), study_variable_name) && contains_val(mapping_data->at(NominalValue), study_variable_name + "/" + study_variable_value)){
                    Mapping &mn = mapping_data->at(NominalStudyVariable)[study_variable_name];
                    if (mn.action == RenameAction || mn.action == MappingTo) study_variable_name = mn.mapping;

                    Mapping &mv = mapping_data->at(NominalValue)[study_variable_name + "/" + study_variable_value];
                    if (mv.action == RenameAction || mv.action == MappingTo) study_variable_value = mv.mapping;
                }
                lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, contains_val(NA_VALUES, study_variable_value))});

            }
            for (int i = 0; i < (int)study_variable_columns_numerical.size(); ++i){
                bool is_not_missing = false;
                string study_variable_name = study_variable_names_numerical[i];
                double study_variable_value = QString(tokens.at(study_variable_columns_numerical[i]).c_str()).toDouble(&is_not_missing);

                if (mapping_data != 0 && contains_val(mapping_data->at(NumericalStudyVariable), study_variable_name)){
                    Mapping &m = mapping_data->at(NumericalStudyVariable)[study_variable_name];
                    if (m.action == RenameAction || m.action == MappingTo) study_variable_name = m.mapping;
                }

                lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
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
            if (uncontains_val(lipid_name_translations[NORMALIZED_NAME], translated_name))
                lipid_name_translations[NORMALIZED_NAME].insert({translated_name, lipid_string});

            if (uncontains_val(lipid_name_translations[NORMALIZED_NAME], lipid_table_name))
                lipid_name_translations[NORMALIZED_NAME].insert({lipid_table_name, lipid_string});

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

        // checking consistancy of study variables
        registered_study_variables.insert(FILE_STUDY_VARIABLE_NAME);
        for (auto lipidome : loaded_lipidomes){
            if (lipidome->cleaned_name == "-"){
                throw LipidSpaceException("Error, sample has no sample name.", CorruptedFileFormat);
            }

            for (auto study_variable : registered_study_variables){
                if (uncontains_val(lipidome->study_variables, study_variable)){
                    throw LipidSpaceException("Error, study variable '" + study_variable + "' not defined for sample '" + lipidome->cleaned_name + "'.", CorruptedFileFormat);
                }
            }
        }




        // check if new study variables must be registered to existing lipidomes
        if (mapping_data){
            for (auto kv : mapping_data->at(NumericalStudyVariable)){
                if ((kv.second.action == RegisterNewDefault || kv.second.action == RegisterNewNaN) && kv.second.study_variable_type == NumericalStudyVariable){
                    string study_variable_name = kv.second.name;
                    double study_variable_value = (kv.second.action == RegisterNewDefault) ? atof(kv.second.mapping.c_str()) : 0.;
                    bool is_not_missing = (kv.second.action == RegisterNewDefault);

                    study_variable_values.insert({study_variable_name, StudyVariableSet(study_variable_name, NumericalStudyVariable)});
                    if (is_not_missing) study_variable_values[study_variable_name].numerical_values.insert(study_variable_value);

                    for (auto lipidome : lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
                else if ((kv.second.action == InsertDefault || kv.second.action == InsertNaN) && kv.second.study_variable_type == NumericalStudyVariable){
                    string study_variable_name = kv.second.name;
                    double study_variable_value = (kv.second.action == InsertDefault) ? atof(kv.second.mapping.c_str()) : 0.;
                    bool is_not_missing = (kv.second.action == InsertDefault);

                    for (auto lipidome : loaded_lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
            }

            for (auto kv : mapping_data->at(NominalStudyVariable)){
                if ((kv.second.action == RegisterNewDefault || kv.second.action == RegisterNewNaN) && kv.second.study_variable_type == NominalStudyVariable){
                    string study_variable_name = kv.second.name;
                    string study_variable_value = (kv.second.action == RegisterNewDefault) ? kv.second.mapping : NO_VALUE_CHAR;
                    bool is_not_missing = (kv.second.action == RegisterNewDefault);

                    study_variable_values.insert({study_variable_name, StudyVariableSet(study_variable_name, NominalStudyVariable)});
                    study_variable_values[study_variable_name].nominal_values.insert({study_variable_value, true});

                    for (auto lipidome : lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
                else if ((kv.second.action == InsertDefault || kv.second.action == InsertNaN) && kv.second.study_variable_type == NominalStudyVariable){
                    string study_variable_name = kv.second.name;
                    string study_variable_value = (kv.second.action == InsertDefault) ? kv.second.mapping : NO_VALUE_CHAR;
                    bool is_not_missing = (kv.second.action == InsertDefault);

                    for (auto lipidome : loaded_lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
            }
        }



        if (study_variable_values.size() > 1){
            for (auto study_variable : registered_study_variables){
                if (uncontains_val(study_variable_values, study_variable)){
                    throw LipidSpaceException("Error, study variable '" + study_variable + "' is not registered, yet.", StudyVariableNotRegistered);
                }
            }

            for (auto kv : study_variable_values){
                if (uncontains_val(registered_study_variables, kv.first)){
                    throw LipidSpaceException("Error, study variable '" + kv.first + "' is not present in imported file.", StudyVariableNotRegistered);
                }
            }

            // add new values into existing study variables
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->study_variables){

                    StudyVariableSet &fs = study_variable_values[kv.first];
                    if (kv.second.study_variable_type == NumericalStudyVariable){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }
        // register study variables
        else {
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->study_variables){

                    if (uncontains_val(study_variable_values, kv.first)){
                        study_variable_values.insert({kv.first, StudyVariableSet(kv.first, kv.second.study_variable_type)});
                    }

                    StudyVariableSet &fs = study_variable_values[kv.first];
                    if (kv.second.study_variable_type == NumericalStudyVariable){
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

    fileLoaded();
}





void LipidSpace::load_column_table(ImportData *import_data){
    vector<TableColumnType> *column_types = import_data->column_types;
    FileTableHandler *fth = import_data->file_table_handler;
    MappingData *mapping_data = import_data->mapping_data;
    string data_table_file = import_data->table_file;
    string sheet = import_data->sheet;
    Logging::write_log("Importing table '" + data_table_file + "' as lipid column table.");

    bool has_sample_col = false;
    for (auto column_type : *column_types){
        has_sample_col |= (column_type == SampleColumn);
    }

    if (fth->headers.size() != column_types->size()){
        throw LipidSpaceException("Error during table import: headers size (" + std::to_string(fth->headers.size()) + ") not equal to size of column type vector (" + std::to_string(column_types->size()) + ").");
    }

    if (!has_sample_col){
        throw LipidSpaceException("No sample column was defined", NoColumnFound);
    }

    vector<string> study_variables_names_numerical;
    vector<string> study_variables_names_nominal;
    vector<LipidAdduct*> lipids;
    map<string, LipidAdduct*> lipid_set;
    vector<Lipidome*> loaded_lipidomes;

    try {
        // go through the column and handle them according to their column type
        for (int i = 0; i < (int)fth->headers.size(); ++i){
            TableColumnType column_type = column_types->at(i);
            switch(column_type){
                case StudyVariableColumnNominal:
                    study_variables_names_nominal.push_back(fth->headers.at(i));
                    break;

                case StudyVariableColumnNumerical:
                    study_variables_names_numerical.push_back(fth->headers.at(i));
                    break;

                case LipidColumn:
                    {
                        LipidAdduct* l = load_lipid(fth->headers.at(i), lipid_set);
                        lipids.push_back(l);
                        break;
                    }

                default:
                    break;
            }
        }

        for (auto tokens : fth->rows){
            map<string, StudyVariable> study_variables;
            vector<LipidAdduct*> measurement_lipids;
            Array intensities;
            string measurement = "";
            int study_variable_counter_nominal = 0;
            int study_variable_counter_numerical = 0;
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

                    case StudyVariableColumnNominal:
                        {
                            string study_variable_name = study_variables_names_nominal[study_variable_counter_nominal];
                            string study_variable_value = tokens.at(i);
                            string study_variable_key = study_variable_name + "/" + study_variable_value;

                            if (contains_val(NA_VALUES, study_variable_value)) study_variable_value = NO_VALUE_CHAR;

                            if (mapping_data != 0 && contains_val(mapping_data->at(NominalStudyVariable), study_variable_name) && contains_val(mapping_data->at(NominalValue), study_variable_key)){
                                Mapping &mn = mapping_data->at(NominalStudyVariable)[study_variable_name];
                                if (mn.action == RenameAction || mn.action == MappingTo) study_variable_name = mn.mapping;

                                Mapping &mv = mapping_data->at(NominalValue)[study_variable_key];
                                if (mv.action == RenameAction || mv.action == MappingTo) study_variable_value = mv.mapping;
                            }

                            study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, contains_val(NA_VALUES, study_variable_value))});
                        }
                        study_variable_counter_nominal++;
                        break;



                    case StudyVariableColumnNumerical:
                        {
                            string study_variable_name = study_variables_names_numerical[study_variable_counter_numerical];
                            bool is_not_missing = false;
                            double study_variable_value = QString(tokens.at(i).c_str()).toDouble(&is_not_missing);

                            if (mapping_data != 0 && contains_val(mapping_data->at(NumericalStudyVariable), study_variable_name)){
                                Mapping &m = mapping_data->at(NumericalStudyVariable)[study_variable_name];
                                if (m.action == RenameAction || m.action == MappingTo) study_variable_name = m.mapping;
                            }
                            study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                        }
                        study_variable_counter_numerical++;
                        break;

                    default:
                        break;
                }
            }

            loaded_lipidomes.push_back(new Lipidome(measurement, data_table_file, sheet));
            Lipidome *lipidome = loaded_lipidomes.back();
            for (auto kv : study_variables){
                lipidome->study_variables.insert({kv.first, StudyVariable(&kv.second)});
            }

            for (auto l : measurement_lipids){
                lipidome->lipids.push_back(l);
                lipidome->species.push_back(l->get_lipid_string());
                lipidome->classes.push_back(l->get_lipid_string(CLASS));
                lipidome->categories.push_back(l->get_lipid_string(CATEGORY));
            }
            lipidome->original_intensities.reset(intensities);
        }



        // checking consistancy of study variables
        set<string> registered_study_variables;
        registered_study_variables.insert(FILE_STUDY_VARIABLE_NAME);
        for (auto study_variable : study_variables_names_numerical){
            if (mapping_data != 0 && contains_val(mapping_data->at(NumericalStudyVariable), study_variable)){
                Mapping &m = mapping_data->at(NumericalStudyVariable)[study_variable];
                if (m.action == RenameAction || m.action == MappingTo) study_variable = m.mapping;
                else if (m.action == RegisterNewNaN || m.action  == RegisterNewDefault) continue;
            }
            registered_study_variables.insert(study_variable);
        }
        for (auto study_variable : study_variables_names_nominal){
            if (mapping_data != 0 && contains_val(mapping_data->at(NominalStudyVariable), study_variable)){
                Mapping &m = mapping_data->at(NominalStudyVariable)[study_variable];
                if (m.action == RenameAction || m.action == MappingTo) study_variable = m.mapping;
                else if (m.action == RegisterNewNaN || m.action  == RegisterNewDefault) continue;
            }
            registered_study_variables.insert(study_variable);
        }

        for (auto lipidome : loaded_lipidomes){
            if (lipidome->cleaned_name == "-"){
                throw LipidSpaceException("Error, sample has no sample name.", CorruptedFileFormat);
            }

            for (auto study_variable : registered_study_variables){
                if (uncontains_val(lipidome->study_variables, study_variable)){
                    throw LipidSpaceException("Error, study variable '" + study_variable + "' not defined for sample '" + lipidome->cleaned_name + "'.", CorruptedFileFormat);
                }
            }
        }

        // check if new study variables must be registered to existing lipidomes
        if (mapping_data){
            for (auto kv : mapping_data->at(NumericalStudyVariable)){
                if ((kv.second.action == RegisterNewDefault || kv.second.action == RegisterNewNaN) && kv.second.study_variable_type == NumericalStudyVariable){
                    string study_variable_name = kv.second.name;
                    double study_variable_value = (kv.second.action == RegisterNewDefault) ? atof(kv.second.mapping.c_str()) : 0.;
                    bool is_not_missing = (kv.second.action == RegisterNewDefault);

                    study_variable_values.insert({study_variable_name, StudyVariableSet(study_variable_name, NumericalStudyVariable)});
                    if (is_not_missing) study_variable_values[study_variable_name].numerical_values.insert(study_variable_value);

                    for (auto lipidome : lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
                else if ((kv.second.action == InsertDefault || kv.second.action == InsertNaN) && kv.second.study_variable_type == NumericalStudyVariable){
                    string study_variable_name = kv.second.name;
                    double study_variable_value = (kv.second.action == InsertDefault) ? atof(kv.second.mapping.c_str()) : 0.;
                    bool is_not_missing = (kv.second.action == InsertDefault);

                    for (auto lipidome : loaded_lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
            }

            for (auto kv : mapping_data->at(NominalStudyVariable)){
                if ((kv.second.action == RegisterNewDefault || kv.second.action == RegisterNewNaN) && kv.second.study_variable_type == NominalStudyVariable){
                    string study_variable_name = kv.second.name;
                    string study_variable_value = (kv.second.action == RegisterNewDefault) ? kv.second.mapping : NO_VALUE_CHAR;
                    bool is_not_missing = (kv.second.action == RegisterNewDefault);

                    study_variable_values.insert({study_variable_name, StudyVariableSet(study_variable_name, NominalStudyVariable)});
                    study_variable_values[study_variable_name].nominal_values.insert({study_variable_value, true});

                    for (auto lipidome : lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
                else if ((kv.second.action == InsertDefault || kv.second.action == InsertNaN) && kv.second.study_variable_type == NominalStudyVariable){
                    string study_variable_name = kv.second.name;
                    string study_variable_value = (kv.second.action == InsertDefault) ? kv.second.mapping : NO_VALUE_CHAR;
                    bool is_not_missing = (kv.second.action == InsertDefault);

                    for (auto lipidome : loaded_lipidomes){
                        lipidome->study_variables.insert({study_variable_name, StudyVariable(study_variable_name, study_variable_value, !is_not_missing)});
                    }

                    registered_study_variables.insert(study_variable_name);
                }
            }
        }


        if (study_variable_values.size() > 1){
            for (auto study_variable : registered_study_variables){
                if (uncontains_val(study_variable_values, study_variable)){
                    throw LipidSpaceException("Error, study variable '" + study_variable + "' is not registered, yet.", StudyVariableNotRegistered);
                }
            }

            for (auto kv : study_variable_values){
                if (uncontains_val(registered_study_variables, kv.first)){
                    throw LipidSpaceException("Error, study variable '" + kv.first + "' is not present in imported file.", StudyVariableNotRegistered);
                }
            }

            // add new values into existing study variables
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->study_variables){
                    StudyVariableSet &fs = study_variable_values[kv.first];
                    if (kv.second.study_variable_type == NumericalStudyVariable){
                        fs.numerical_values.insert(kv.second.numerical_value);
                    }
                    else {
                        fs.nominal_values.insert({kv.second.nominal_value, true});
                    }
                }
            }
        }



        // register study variables
        else {
            for (auto lipidome : loaded_lipidomes){
                for (auto kv : lipidome->study_variables){

                    if (uncontains_val(study_variable_values, kv.first)){
                        study_variable_values.insert({kv.first, StudyVariableSet(kv.first, kv.second.study_variable_type)});
                    }

                    StudyVariableSet &fs = study_variable_values[kv.first];
                    if (kv.second.study_variable_type == NumericalStudyVariable){
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

    fileLoaded();
}




void LipidSpace::load_table(ImportData *import_data){
    switch (import_data->table_type){
        case ROW_PIVOT_TABLE: load_row_table(import_data); break;
        case COLUMN_PIVOT_TABLE: load_column_table(import_data); break;
        case FLAT_TABLE: load_flat_table(import_data); break;
    }
}




// load and parse lipid table, lipids per row, measurements per column
void LipidSpace::load_row_table(ImportData *import_data){
    vector<TableColumnType> *column_types = import_data->column_types;
    FileTableHandler *fth = import_data->file_table_handler;
    string table_file = import_data->table_file;
    string sheet = import_data->sheet;
    Logging::write_log("Reading table '" + table_file + "' lipid row table.");

    vector<Array> intensities;
    map<string, LipidAdduct*> lipid_set;
    vector<Lipidome*> loaded_lipidomes;

    try {


        // no specific column order is provided, we assume that first column contains
        // lipid species names and all remaining columns correspond to a sample containing
        // intensity / quant values
        if (column_types == 0){

            for (auto header : fth->headers){
                loaded_lipidomes.push_back(new Lipidome(header, table_file, sheet));
                intensities.push_back(Array());
            }

            for (auto tokens : fth->rows){
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
                    string header = fth->headers.at(i);
                    if (uncontains_val(doublettes, header)){
                        doublettes.insert({header, 1});
                    }
                    else {
                        header += "." + std::to_string(++doublettes[header]);
                    }

                    selection[3].insert({header, true});
                    loaded_lipidomes.push_back(new Lipidome(header, table_file, sheet));
                    intensities.push_back(Array());
                }
            }

            for (auto tokens : fth->rows){

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
        }
        if (study_variable_values.size() > 1){
            throw LipidSpaceException("Error, study variables are already registered, table does not contain any variables and thus cannot be imported.", StudyVariableNotRegistered);
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
            study_variable_values[FILE_STUDY_VARIABLE_NAME].nominal_values.insert({lipidome->study_variables[FILE_STUDY_VARIABLE_NAME].nominal_value, true});

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
    if (uncontains_val(lipid_name_translations[NORMALIZED_NAME], translated_name))
        lipid_name_translations[NORMALIZED_NAME].insert({translated_name, lipid_string});

    if (uncontains_val(lipid_name_translations[NORMALIZED_NAME], lipid_name))
        lipid_name_translations[NORMALIZED_NAME].insert({lipid_name, lipid_string});

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



void LipidSpace::run_analysis(){

    // compute PCA matrixes for the complete lipidome
    int num_for_PCA = compute_global_distance_matrix();
    if (num_for_PCA == 0){
        return;
    }

    if (num_for_PCA >= 3){
        cols_for_pca = min(cols_for_pca, (int)global_lipidome->lipids.size() - 1);

        // perform the principal component analysis
        Matrix pca;
        global_distances.rewrite(global_lipidome->m);
        global_lipidome->m.PCA(pca, cols_for_pca);
        global_lipidome->m.rewrite(pca);
    }
    else {
        global_lipidome->m.reset(selected_lipidomes.size(), global_lipidome->lipids.size());
    }

    // cutting the global PCA matrix back to a matrix for each lipidome
    separate_matrixes();
    normalize_intensities();

    if (selected_lipidomes.size() > 1){
        compute_hausdorff_matrix();
    }

    analysis_finished = true;
}




void LipidSpace::lipid_analysis(bool report_progress){
    if (lipidomes.size() == 0){
        if (progress && !progress->stop_progress && report_progress){
            progress->finish();
        }
        return;
    }
    Logging::write_log("Started analysis");

    if (progress && report_progress){
        progress->prepare(9);
    }

    auto start = high_resolution_clock::now();
    analysis_finished = false;
    if (dendrogram_root){
        delete dendrogram_root;
        dendrogram_root = 0;
    }
    selected_lipidomes.clear();
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    statistics_lipids.clear();


    int num_for_PCA = 0;
    // compute PCA matrixes for the complete lipidome
    if (!progress || !progress->stop_progress){
        num_for_PCA = compute_global_distance_matrix();
        if (progress && report_progress){
            progress->increment();
        }
        if (num_for_PCA == 0){
            if (progress && report_progress){
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
            if (progress && report_progress){
                progress->connect(&global_lipidome->m, &Matrix::increment, progress, &Progress::increment);
            }
            Matrix pca;
            global_distances.rewrite(global_lipidome->m);
            global_lipidome->m.PCA(pca, cols_for_pca);
            global_lipidome->m.rewrite(pca);
            if (progress && report_progress){
                progress->disconnect(&global_lipidome->m, SIGNAL(increment()), 0, 0);
            }
        }
    }
    else {
        global_lipidome->m.reset(selected_lipidomes.size(), global_lipidome->lipids.size());
    }

    // cutting the global PCA matrix back to a matrix for each lipidome
    if (!progress || !progress->stop_progress){
        separate_matrixes();
        if (progress && report_progress){
            progress->increment();
        }
        normalize_intensities();
        if (progress && report_progress){
            progress->increment();
        }
    }

    if (!progress || !progress->stop_progress){
        if (selected_lipidomes.size() > 1){
            compute_hausdorff_matrix();
            if (progress && report_progress){
                progress->increment();
            }
        }
    }

    if (!progress || !progress->stop_progress){
        if (selected_lipidomes.size() > 1){
            create_dendrogram();
            if (progress && report_progress){
                progress->increment();
            }
        }
    }





    if (!progress || !progress->stop_progress){
        // for statistics
        statistics_matrix.reset(0, 0);
        map<LipidAdduct*, int> lipid_map;
        map<string, int> lipid_name_map;
        // setting up lipid to column in matrix map
        for (uint i = 0; i < global_lipidome->lipids.size(); ++i){
            LipidAdduct* lipid = global_lipidome->lipids[i];
            if (uncontains_val(lipid_map, lipid)){
                lipid_map.insert({lipid, lipid_map.size()});
                lipid_name_map.insert({global_lipidome->species[i], lipid_name_map.size()});
            }
        }

        // set up matrix for multiple linear regression
        statistics_matrix.reset(selected_lipidomes.size(), lipid_map.size());
        for (uint r = 0; r < selected_lipidomes.size(); ++r){
            Lipidome* lipidome = selected_lipidomes[r];
            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                if (contains_val(lipid_map, lipidome->lipids[i])){
                    statistics_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->normalized_intensities[i];
                }
            }
        }

        statistics_lipids.resize(lipid_name_map.size());
        for (auto kv : lipid_name_map) statistics_lipids[kv.second] = kv.first;

    }





    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Process: " << duration.count() << endl;
    analysis_finished = true;

    if (progress && !progress->stop_progress && report_progress){
        progress->finish();
        Logging::write_log("Finished analysis with a union of " + std::to_string(global_lipidome->lipids.size()) + " lipids among " + std::to_string(selected_lipidomes.size()) + " lipidome" + (selected_lipidomes.size() > 1 ? "s" : "") + " in total.");
    }
}




void LipidSpace::feature_analysis(bool report_progress){
    Array target_values;
    map<string, double> nominal_target_values;
    int nom_counter = 0;
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    Matrix global_matrix;
    vector< Gene* >genes;
    Array missing_values;
    bool is_nominal = study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

    if (uncontains_val(study_variable_values, target_variable)){
        if (progress && !progress->stop_progress && report_progress){
            progress->finish();
        }
        return;
    }
    Logging::write_log("Started feature analysis");



    // select lipidomes for feature selection
    vector<Lipidome*> lipidomes_for_feature_selection;
    for (auto lipidome :  lipidomes){
        bool select_lipidome = true;
        string lipidome_name = lipidome->cleaned_name;

        for (auto kv : lipidome->study_variables){
            string study_variable_name = kv.first;
            StudyVariable &study_variable = kv.second;
            if (study_variable.study_variable_type == NominalStudyVariable){
                if (!study_variable_values[study_variable_name].nominal_values[study_variable.nominal_value]){
                    select_lipidome = false;
                    break;
                }
            }
            else {
                double numerical_value = study_variable.numerical_value;
                auto filter = study_variable_values[study_variable_name].numerical_filter.first;
                auto filter_values = study_variable_values[study_variable_name].numerical_filter.second;
                switch (filter){

                    case LessFilter: // inverse logic, Less tells us, what to keep, greater we filter out
                        if (filter_values.size() < 1) continue;
                        if (filter_values[0] < numerical_value) select_lipidome = false;
                        break;

                    case GreaterFilter:
                        if (filter_values.size() < 1) continue;
                        if (filter_values[0] > numerical_value) select_lipidome = false;
                        break;

                    case EqualFilter:
                        {
                            if (filter_values.size() < 1) continue;
                            bool filtered_out = true; // < 1e-16 due to floating point rounding errors
                            for (double filter_value : filter_values) filtered_out &= !(fabs(numerical_value - filter_value) < 1e-16);
                            select_lipidome = !filtered_out;
                        }
                        break;

                    case WithinRange:
                        if (filter_values.size() < 2) continue;
                        if (numerical_value < filter_values[0] || filter_values[1] < numerical_value) select_lipidome = false;
                        break;

                    case OutsideRange:
                        if (filter_values.size() < 2) continue;
                        if (filter_values[0] <= numerical_value && numerical_value <= filter_values[1]) select_lipidome = false;
                        break;

                    default:
                        break;
                }
            }
        }

        if (select_lipidome && contains_val(selection[3], lipidome_name) && selection[3][lipidome_name]){
            lipidomes_for_feature_selection.push_back(lipidome);
        }
    }



    // perform the principal component analysis
    if (!progress || !progress->stop_progress){
        // set the step size for the next analyses

        // setup array for target variable values, if nominal then each with incrementing number
        if (is_nominal){
            for (auto lipidome : lipidomes_for_feature_selection){
                string nominal_value = lipidome->study_variables[target_variable].nominal_value;

                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                }
                target_values.push_back(nominal_target_values[nominal_value]);
            }
        }
        else {
            for (auto lipidome : lipidomes_for_feature_selection){
                target_values.push_back(lipidome->study_variables[target_variable].numerical_value);
            }
        }

        // setting up lipid to column in matrix map
        for (auto lipidome : lipidomes_for_feature_selection){
            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                LipidAdduct* lipid = lipidome->lipids[i];
                if (uncontains_val(lipid_map, lipid) && contains_val(lipidome->study_variables, target_variable) && !lipidome->study_variables[target_variable].missing){
                    lipid_map.insert({lipid, lipid_map.size()});
                    lipid_name_map.insert({lipidome->species[i], lipid_name_map.size()});
                }
            }
        }


        if (is_nominal && nom_counter <= 1){
            if (progress && !progress->stop_progress && report_progress){
                progress->finish();
            }
            return;
        }

        // set up matrix for multiple linear regression
        global_matrix.reset(lipidomes_for_feature_selection.size(), lipid_map.size());
        for (uint r = 0; r < lipidomes_for_feature_selection.size(); ++r){
            Lipidome* lipidome = lipidomes_for_feature_selection[r];

            for (uint i = 0; i < lipidome->lipids.size(); ++i){
                global_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->normalized_intensities[i];
            }
        }

        // counting missing values per lipid
        missing_values.resize(global_matrix.cols, 0);
        for (int c = 0; c < global_matrix.cols; ++c){
            double* row = &(global_matrix.m[c * global_matrix.rows]);
            int missing = 0;
            for (int r = 0; r < global_matrix.rows; ++r){
                missing += row[r] <= 1e-15;
            }
            missing_values[c] = missing;
        }
    }

    if (is_nominal){
        global_matrix.scale();
    }



    // determining the upper number of features to consider
    int n = global_matrix.cols;// - !is_nominal; // -1 because we added a column of constant values 1, 1, 1, 1, ...
    int n_features = n;
    if (n > 1) n = min(n_features - 2, (int)sqrt(n) * 2);

    // perform the principal component analysis
    if (!progress || !progress->stop_progress){
        // set the step size for the next analyses
        if (progress && report_progress){
            progress->prepare(n * n_features);
        }

        genes.resize(n + 1, 0);
        genes[0] = new Gene(n_features + 1);
        genes[0]->score = is_nominal ? 0 : INFINITY;


        if (progress && report_progress){
            progress->increment();
        }
    }

    // implementation of sequential forward lipid (feature) selection
    for(int i = 1; i <= n && (!progress || !progress->stop_progress); ++i){
        if (progress->stop_progress) break;
        Gene* best = 0;
        int pos = 0;
        double best_score = is_nominal ? 0 : INFINITY;
        Gene* last = genes[i - 1];
        while (pos < n_features && (!progress || !progress->stop_progress)){
            if (progress->stop_progress) break;
            if (!last->gene_code[pos]){
                Gene* new_gene = new Gene(last);
                new_gene->gene_code[pos] = true;

                Indexes lipid_indexes;
                new_gene->get_indexes(lipid_indexes);
                Matrix sub_lipids;
                sub_lipids.rewrite(global_matrix, {}, lipid_indexes);

                double missing_lipids = 0;
                for (auto feature_index : lipid_indexes) missing_lipids += missing_values[feature_index];
                double cnt_lipids = lipid_indexes.size() * global_matrix.rows;

                if (is_nominal){
                    Array summed_values;
                    summed_values.resize(sub_lipids.rows);

                    for (int c = 0; c < sub_lipids.cols; c++){
                    double *col = &(sub_lipids.m[c * sub_lipids.rows]);
                        for (int r = 0; r < sub_lipids.rows; ++r){
                            summed_values[r] += col[r];
                        }
                    }

                    vector<Array> arrays(nom_counter);
                    for (int r = 0; r < sub_lipids.rows; ++r) arrays[target_values[r]].push_back(summed_values[r]);

                    double acc = compute_accuracy(arrays);
                    acc = __abs(1. / (double)nom_counter - acc) + 1. / (double)nom_counter;
                    new_gene->score = acc * (cnt_lipids - missing_lipids) / cnt_lipids;

                    // search for maximal nominal score
                    if (!best || best_score < new_gene->score || best_score == INFINITY){
                        best = new_gene;
                        best_score = new_gene->score;
                    }
                    else {
                        delete new_gene;
                    }
                }


                else {
                    Array constants(sub_lipids.rows, 1);
                    sub_lipids.add_column(constants);

                    Array coefficiants;
                    coefficiants.compute_coefficiants(sub_lipids, target_values);    // estimating coefficiants
                    new_gene->score = compute_aic(sub_lipids, coefficiants, target_values);  // computing aic
                    new_gene->score *= sqrt(1 + missing_lipids);

                    // search for minimal numerical score
                    if (!best || best_score > new_gene->score){
                        best = new_gene;
                        best_score = new_gene->score;
                    }
                    else {
                        delete new_gene;
                    }

                }

            }
            ++pos;
            if (progress && report_progress){
                progress->increment();
            }
        }

        genes[i] = best;
    }


    if (!progress || !progress->stop_progress){
        // find the feature subset with the lowest best_score
        int best_pos = 0;
        if (is_nominal){
            double best_score = 0;
            for (uint i = 1; i < genes.size(); ++i){
                if (best_score < genes[i]->score){
                    best_score = genes[i]->score;
                    best_pos = i;
                }
            }
        }
        else {
            double best_score = INFINITY;
            for (uint i = 1; i < genes.size(); ++i){
                if (best_score == INFINITY || best_score > genes[i]->score){
                    best_score = genes[i]->score;
                    best_pos = i;
                }
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

        if (progress && !progress->stop_progress && report_progress){
            progress->finish();
        }
        emit reassembled();
    }
}




void LipidSpace::complete_feature_analysis(){
    complete_feature_analysis_table.clear();
    complete_feature_analysis_lipids.clear();
    if (progress){
        progress->prepare(max(0, (int)study_variable_values.size() - 1));
    }
    for (auto kv : study_variable_values){
        if (progress && progress->stop_progress) break;
        if (kv.first == FILE_STUDY_VARIABLE_NAME) continue;

        complete_feature_analysis_table.push_back(vector<double>());
        complete_feature_analysis_lipids.push_back(set<string>());

        target_variable = kv.first;
        feature_analysis(false);

        if (progress && progress->stop_progress) break;
        lipid_analysis(false);

        if (progress && progress->stop_progress) break;


        for (auto &kv_s : selection[0]){
            if (kv_s.second) complete_feature_analysis_lipids.back().insert(kv_s.first);
        }


        for (auto kv2 : study_variable_values){
            if (progress && progress->stop_progress) break;
            if (kv2.first == FILE_STUDY_VARIABLE_NAME) continue;
            target_variable = kv2.first;

            // setup array for target variable values, if nominal then each with incrementing number
            map<string, double> nominal_target_values;
            Indexes target_indexes;
            vector<Array> series;
            Array target_values;
            int nom_counter = 0;
            vector<string> nominal_values;

            bool is_nominal = study_variable_values[target_variable].study_variable_type == NominalStudyVariable;
            if (is_nominal){
                for (auto lipidome : selected_lipidomes){
                    if (lipidome->study_variables[target_variable].missing) continue;

                    string nominal_value = lipidome->study_variables[target_variable].nominal_value;
                    if (uncontains_val(nominal_target_values, nominal_value)){
                        nominal_target_values.insert({nominal_value, nom_counter++});
                        nominal_values.push_back(nominal_value);
                    }
                    target_indexes.push_back(nominal_target_values[nominal_value]);
                }
            }
            else {
                for (auto lipidome : selected_lipidomes){
                    if (lipidome->study_variables[target_variable].missing) continue;

                    target_values.push_back(lipidome->study_variables[target_variable].numerical_value);
                }
            }

            if (progress && progress->stop_progress) break;
            Matrix stat_matrix(statistics_matrix);

            // if any lipidome has a missing study variable, discard the lipidome from the statistic
            Indexes lipidomes_to_keep;
            for (int r = 0; r < stat_matrix.rows; ++r){
                if (!selected_lipidomes[r]->study_variables[target_variable].missing) lipidomes_to_keep.push_back(r);
            }
            Matrix tmp;
            tmp.rewrite(stat_matrix, lipidomes_to_keep);
            stat_matrix.rewrite(tmp);


            if (progress && progress->stop_progress) break;
            if (is_nominal){
                series.resize(nom_counter);

                if (stat_matrix.cols > 1) stat_matrix.scale();
                for (int r = 0; r < stat_matrix.rows; ++r){
                    if (progress && progress->stop_progress) break;
                    double sum = 0;
                    for (int c = 0; c < stat_matrix.cols; c++){
                        double val = stat_matrix(r, c);
                        if (!isnan(val) && !isinf(val)) sum += val;
                    }
                    if (stat_matrix.cols > 1 || sum > 1e-15){
                        series[target_indexes[r]].push_back(sum);
                    }
                }
                if (progress && progress->stop_progress) break;

                if (nom_counter >= 2){
                    complete_feature_analysis_table.back().push_back(compute_accuracy(series));
                }
                else {
                    complete_feature_analysis_table.back().push_back(-1);
                }
            }
            else {
                Array constants(stat_matrix.rows, 1);
                stat_matrix.add_column(constants);

                Array coefficiants;
                if (progress && progress->stop_progress) break;

                coefficiants.compute_coefficiants(stat_matrix, target_values);    // estimating coefficiants
                Array S;
                if (progress && progress->stop_progress) break;
                S.mult(stat_matrix, coefficiants);


                double mx = 0, my = 0, ny = 0;
                // computing the study variable mean based on missing values of lipids
                for (uint r = 0; r < S.size(); ++r){
                    mx += S[r];
                    my += target_values[r];
                    ny += 1;
                }
                mx /= ny;
                my /= ny;
                if (progress && progress->stop_progress) break;

                // estimate slope and intercept factors for linear regression
                double slope_num = 0, slope_denom = 0;
                for (uint r = 0; r < S.size(); ++r){
                    slope_num += (S[r] - mx) * (target_values[r] - my);
                    slope_denom += sq(S[r] - mx);
                }
                double slope = slope_num / slope_denom;
                double intercept = my - slope * mx;

                double SQR = 0, SQT = 0;
                for (uint r = 0; r < S.size(); ++r){
                    SQR += sq(target_values[r] - (slope * S[r] + intercept));
                    SQT += sq(target_values[r] - my);
                }
                complete_feature_analysis_table.back().push_back(1. - SQR / SQT);
            }




        }

        if (progress){
            progress->increment();
        }
    }
    if (progress && !progress->stop_progress){
        progress->finish();
    }
}





void LipidSpace::run(){
    if (process_id == 1){
        lipid_analysis();
    }

    else if (process_id == 2 && target_variable != ""){
        feature_analysis();
    }

    else if (process_id == 3){
        //complete_feature_analysis();

        complete_feature_analysis_table.clear();
        complete_feature_analysis_lipids.clear();

        if (progress){
            progress->prepare(1);
        }

        LipidSpace lipid_space_clone(this);
        lipid_space_clone.complete_feature_analysis();

        // copy results
        for (auto vd : lipid_space_clone.complete_feature_analysis_table){
            complete_feature_analysis_table.push_back(vector<double>());
            for (auto value : vd) complete_feature_analysis_table.back().push_back(value);
        }
        for (auto ss : lipid_space_clone.complete_feature_analysis_lipids){
            complete_feature_analysis_lipids.push_back(set<string>());
            for (auto value : ss) complete_feature_analysis_lipids.back().insert(value);
        }

        lipid_space_clone.progress = 0;
    }

    process_id = 0;
}



void LipidSpace::reset_analysis(){
    analysis_finished = false;
    study_variable_values.clear();
    study_variable_values.insert({FILE_STUDY_VARIABLE_NAME, StudyVariableSet(FILE_STUDY_VARIABLE_NAME, NominalStudyVariable)});
    if (dendrogram_root){
        delete dendrogram_root;
        dendrogram_root = 0;
    }
    selected_lipidomes.clear();
    dendrogram_sorting.clear();
    dendrogram_points.clear();
    lipid_name_translations[0].clear();
    lipid_name_translations[1].clear();
    lipid_name_translations[2].clear();
    lipid_sortings.clear();
    global_distances.clear();
    complete_feature_analysis_table.clear();
    complete_feature_analysis_lipids.clear();
    statistics_lipids.clear();
    statistics_matrix.clear();
    hausdorff_distances.clear();

    for (int i = 0; i < 4; ++i) selection[i].clear();

    for (auto kv : all_lipids) delete kv.second;
    all_lipids.clear();

    for (auto lipidome : lipidomes) delete lipidome;
    lipidomes.clear();
    cols_for_pca = cols_for_pca_init;

    delete global_lipidome;
    global_lipidome = new Lipidome("Global lipidome", "");

    for (auto lipidome : study_lipidomes) delete lipidome;
    study_lipidomes.clear();
}




