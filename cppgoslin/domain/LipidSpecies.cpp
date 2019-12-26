#include "LipidSpecies.h"

    
LipidSpecies::LipidSpecies(string _head_group, LipidCategory _lipid_category, LipidClass _lipid_class, LipidSpeciesInfo *lipid_species_info){
    head_group = head_group;
    lipid_category = (_lipid_category != UNDEFINED_CATEGORY) ? _lipid_category : get_category(head_group);
    
    lipid_class = (_lipid_class != UNDEFINED_CLASS) ? _lipid_class : get_class(head_group);
    if (lipid_species_info != NULL){
        info.level = lipid_species_info->level;
        info.num_carbon = lipid_species_info->num_carbon;
        info.num_hydroxyl = lipid_species_info->num_hydroxyl;
        info.num_double_bonds = lipid_species_info->num_double_bonds;
        info.lipid_FA_bond_type = lipid_species_info->lipid_FA_bond_type;
        delete lipid_species_info;
    }
    else {
        info.level = UNDEFINED_LEVEL;
    }
    use_head_group = false;
}


string LipidSpecies::get_lipid_string(LipidLevel level){
    if (level == UNDEFINED_LEVEL){
        stringstream s;
        s << "LipidSpecies does not know how to create a lipid string for level " << level;
        string error_message;
        s >> error_message;
        throw RuntimeException(error_message);
    }
    
    else if (level == CATEGORY){
        return get_category_string(lipid_category);
    }
        
    else if (level == CLASS) {
        return get_class_string(lipid_class);
    }
        
    else if (level == SPECIES){
        stringstream st;
        st << (!use_head_group ? get_class_string(lipid_class) : head_group);
        
        if (info.num_carbon > 0){
            st <<  " " << info.num_carbon;
            st << ":" << info.num_double_bonds;
            if (info.num_hydroxyl > 0)  st << ";" << info.num_hydroxyl;
            st << FattyAcid::suffix(info.lipid_FA_bond_type);
        }
        string lipid_string;
        st >> lipid_string;
        return lipid_string;
    }
        
    else {
        stringstream stt;
        stt << "LipidSpecies does not know how to create a lipid string for level " << level;
        string error_message2;
        stt >> error_message2;
        throw RuntimeException(error_message2);
    }
}


LipidCategory LipidSpecies::get_category(string _head_group){
    if (!StringCategory.size()){
        for (const auto& kvp : lipid_classes){
            LipidCategory category = kvp.second.lipid_category;
            for (auto hg : kvp.second.synonyms){
                StringCategory.insert(pair<string, LipidCategory>(hg, category));
            }
        }
    }
    
    
    if (StringCategory.find(_head_group) == StringCategory.end()){
        throw RuntimeException("Lipid with head group '" + _head_group + "' not found.");
    }
    return StringCategory.at(_head_group);
}


LipidClass LipidSpecies::get_class(string _head_group){
    if (!StringClass.size()){
        for (const auto& kvp : lipid_classes){
            LipidClass l_class = kvp.first;
            for (auto hg : kvp.second.synonyms){
                StringClass.insert(pair<string, LipidClass>(hg, l_class));
            }
        }
    }
    
    if (StringClass.find(_head_group) == StringClass.end()){
        throw RuntimeException("Lipid with head group '" + _head_group + "' not found.");
    }
    return StringClass.at(_head_group);
}


string LipidSpecies::get_class_string(LipidClass _lipid_class){
    return ClassString.at(_lipid_class);
}


string LipidSpecies::get_category_string(LipidCategory _lipid_category){
    return CategoryString.at(_lipid_category);
}
