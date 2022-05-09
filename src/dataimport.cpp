#include "lipidspace/dataimport.h"

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
