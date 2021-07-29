/*
MIT License

Copyright (c) the authors (listed in global LICENSE file)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "LipidSpecies.h"

#include <iostream>
using namespace std;

LipidSpecies::LipidSpecies(Headgroup* _headgroup, vector<FattyAcid*>* _fa){
    headgroup = _headgroup;
    
    info = new LipidSpeciesInfo(headgroup->lipid_class);
    info->level = SPECIES;
    
    // add fatty acids
    if (_fa != 0){
        for (auto fatty_acid : *_fa){
            info->add(fatty_acid);
            fa_list.push_back(fatty_acid);
        }
    }
    
    
    if (headgroup->sp_exception){
        if (uncontains_p(info->functional_groups, "OH")) info->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
        info->functional_groups->at("OH").push_back(KnownFunctionalGroups::get_functional_group("OH"));
    }
    
    
    for (auto decorator : *headgroup->decorators){
        if (decorator->name == "decorator_alkyl" || decorator->name == "decorator_acyl"){
            ElementTable* e = decorator->get_elements();
            info->num_carbon += e->at(ELEMENT_C);
            delete e;
            info->double_bonds->num_double_bonds += decorator->get_double_bonds();
        }
    }
        
}


LipidSpecies::~LipidSpecies(){
    for (auto fa : fa_list){
        delete fa;
    }
    
    delete info;
    delete headgroup;
}


LipidLevel LipidSpecies::get_lipid_level(){
    return SPECIES;
}


string LipidSpecies::get_lipid_string(LipidLevel level){
    switch (level){
            
        default:
            throw RuntimeException("LipidSpecies does not know how to create a lipid string for level " + std::to_string(level));
            
        case UNDEFINED_LEVEL:
            throw RuntimeException("LipidSpecies does not know how to create a lipid string for level " + std::to_string(level));
            
        case CLASS:
        case CATEGORY:
            return headgroup->get_lipid_string(level);
            
        case NO_LEVEL:
        case SPECIES:
            stringstream lipid_string;
            lipid_string << headgroup->get_lipid_string(level);
            
            if (info->elements->at(ELEMENT_C) > 0 || info->num_carbon > 0){
                lipid_string << (headgroup->lipid_category != ST ? " " : "/") << info->to_string();
            }
            return lipid_string.str();
    }
    return "";
}



string LipidSpecies::get_extended_class(){
    bool special_case = (info->num_carbon > 0) ? (headgroup->lipid_category == GP) : false;
    string class_name = headgroup->get_class_name();
    if (special_case && (info->extended_class == ETHER_PLASMANYL || info->extended_class == ETHER_UNSPECIFIED)){
        return class_name + "-O";
    }
    
    else if (special_case && info->extended_class == ETHER_PLASMENYL){
        return class_name + "-p";
    }
    
    return class_name;
}


vector<FattyAcid*> LipidSpecies::get_fa_list(){
    return fa_list;
}




ElementTable* LipidSpecies::get_elements(){
    ElementTable* elements = create_empty_table();
    
    switch(info->level){
        case STRUCTURAL_SUBSPECIES:
        case ISOMERIC_SUBSPECIES:
        case SPECIES:
        case MOLECULAR_SUBSPECIES:
            break;

        default:    
            throw LipidException("Element table cannot be computed for lipid level " + std::to_string(info->level));
    }
    
    
    ElementTable* hg_elements = headgroup->get_elements();
    for (auto &kv : *hg_elements) elements->at(kv.first) += kv.second;
    delete hg_elements;
    
    ElementTable* info_elements = info->get_elements();
    for (auto &kv : *info_elements) elements->at(kv.first) += kv.second;
    delete info_elements;
    
    // since only one FA info is provided, we have to treat this single information as
    // if we would have the complete information about all possible FAs in that lipid
    LipidClassMeta &meta = LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class);
    
    int additional_fa = meta.possible_num_fa;
    int remaining_H = meta.max_num_fa - additional_fa;
    int hydrochain = contains(meta.special_cases, "HC");
    
    elements->at(ELEMENT_O) -= -additional_fa + info->num_ethers + headgroup->sp_exception + hydrochain;
    elements->at(ELEMENT_H) += -additional_fa + remaining_H + 2 * info->num_ethers + 2 * hydrochain;
    
    return elements;
}
