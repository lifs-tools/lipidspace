#include "LipidSpecies.h"

#include <iostream>
using namespace std;

LipidSpecies::LipidSpecies(string _head_group, LipidCategory _lipid_category, LipidClass _lipid_class, LipidSpeciesInfo *lipid_species_info){
    head_group = _head_group;
    lipid_category = (_lipid_category != NO_CATEGORY) ? _lipid_category : get_category(head_group);
    lipid_class = (_lipid_class != NO_CLASS) ? _lipid_class : get_class(head_group);
    if (lipid_species_info != NULL){
        info.level = lipid_species_info->level;
        info.num_carbon = lipid_species_info->num_carbon;
        info.num_hydroxyl = lipid_species_info->num_hydroxyl;
        info.num_double_bonds = lipid_species_info->num_double_bonds;
        info.lipid_FA_bond_type = lipid_species_info->lipid_FA_bond_type;
    }
    else {
        info.level = UNDEFINED_LEVEL;
    }
    use_head_group = false;
}


LipidSpecies::~LipidSpecies(){
    
}


string LipidSpecies::get_lipid_string(LipidLevel level){
    switch (level){
            
        default:
            throw RuntimeException("LipidSpecies does not know how to create a lipid string for level " + to_string(level));
            
        case UNDEFINED_LEVEL:
            throw RuntimeException("LipidSpecies does not know how to create a lipid string for level " + to_string(level));
            
        case CATEGORY:
            return get_category_string(lipid_category);
            
        case CLASS:
            return (!use_head_group ? get_class_string(lipid_class) : head_group);
            
        case NO_LEVEL:
        case SPECIES:
            stringstream st;
            st << (!use_head_group ? get_class_string(lipid_class) : head_group);
            
            if (info.num_carbon > 0){
                st <<  " " << info.num_carbon;
                st << ":" << info.num_double_bonds;
                if (info.num_hydroxyl > 0)  st << ";" << info.num_hydroxyl;
                st << FattyAcid::suffix(info.lipid_FA_bond_type);
            }
            
            return st.str();
    }
    return "";
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
    
    
    auto cat = StringCategory.find(_head_group);
    return (cat != StringCategory.end()) ? StringCategory.at(_head_group) : UNDEFINED;
}


LipidClass LipidSpecies::get_class(string _head_group){
    if (!StringClass.size()){
        for (auto kvp : lipid_classes){
            LipidClass l_class = kvp.first;
            for (auto hg : kvp.second.synonyms){
                StringClass.insert({hg, l_class});
            }
        }
    }
    
    auto cl = StringClass.find(_head_group);
    return (cl != StringClass.end()) ? cl->second : UNDEFINED_CLASS;
}


string LipidSpecies::get_class_string(LipidClass _lipid_class){
    if (!ClassString.size()){
        for (auto kvp : lipid_classes){
            ClassString.insert({kvp.first, kvp.second.synonyms.at(0)});
        }
    }
    auto cl = ClassString.find(_lipid_class);
    return (cl != ClassString.end()) ? ClassString.at(_lipid_class) : "UNDEFINED";
}


string LipidSpecies::get_class_name(){
    return lipid_classes.at(lipid_class).class_name;
}


string LipidSpecies::get_category_string(LipidCategory _lipid_category){
    return CategoryString.at(_lipid_category);
}
