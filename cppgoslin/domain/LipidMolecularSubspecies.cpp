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


#include "LipidMolecularSubspecies.h"


LipidMolecularSubspecies::LipidMolecularSubspecies (Headgroup* _headgroup, vector<FattyAcid*> *_fa) : LipidSpecies(_headgroup, _fa) {
    info->level = MOLECULAR_SUBSPECIES;
    for (auto fatty_acid : fa_list){
        if (contains(fa, fatty_acid->name)){
            throw ConstraintViolationException("FA names must be unique! FA with name " + fatty_acid->name + " was already added!");
        }
        fa.insert({fatty_acid->name, fatty_acid});
    }
           
            
    // add 0:0 dummys
    for (int i = (int)_fa->size(); i < info->total_fa; ++i){
        FattyAcid *fatty_acid = new FattyAcid("FA" + std::to_string(i + _fa->size() + 1));
        info->add(fatty_acid);
        fa.insert({fatty_acid->name, fatty_acid});
        fa_list.push_back(fatty_acid);
    }
}


string LipidMolecularSubspecies::build_lipid_subspecies_name(LipidLevel level){
    if (level == NO_LEVEL) level = MOLECULAR_SUBSPECIES;
    
    string fa_separator = (level != MOLECULAR_SUBSPECIES || headgroup->lipid_category == SP) ? "/" : "_";
    stringstream lipid_name;
    lipid_name << headgroup->get_lipid_string(level);
    

    string fa_headgroup_separator = (headgroup->lipid_category != ST) ? " " : "/";
    
    switch (level){
        case ISOMERIC_SUBSPECIES:
        case STRUCTURAL_SUBSPECIES:
            if (fa_list.size() > 0){
                lipid_name << fa_headgroup_separator;
                int i = 0;
    
                for (auto fatty_acid : fa_list){
                    if (i++ > 0) lipid_name << fa_separator;
                    lipid_name << fatty_acid->to_string(level);
                }
            }
            break;
            
        default:
            bool go_on = false;
            for (auto fatty_acid : fa_list){
                if (fatty_acid->num_carbon > 0){
                    go_on = true;
                    break;
                }
            }
            
            if (go_on){
                lipid_name << fa_headgroup_separator;
                int i = 0;
                for (auto fatty_acid : fa_list){
                    if (fatty_acid->num_carbon > 0){
                        if (i++ > 0) lipid_name << fa_separator;
                        lipid_name << fatty_acid->to_string(level);
                    }
                }
            }
            break;
    }
    return lipid_name.str();
}


LipidLevel LipidMolecularSubspecies::get_lipid_level(){
    return MOLECULAR_SUBSPECIES;
}



ElementTable* LipidMolecularSubspecies::get_elements(){
    ElementTable* elements = create_empty_table();
    
    ElementTable* hg_elements = headgroup->get_elements();
    for (auto &kv : *hg_elements) elements->at(kv.first) += kv.second;
    delete hg_elements;
    
    // add elements from all fatty acyl chains
    for (auto fatty_acid : fa_list){
        ElementTable* fa_elements = fatty_acid->get_elements();
        for (auto &kv : *fa_elements) elements->at(kv.first) += kv.second;
        delete fa_elements;
    }
    
    return elements;
}


string LipidMolecularSubspecies::get_lipid_string(LipidLevel level) {
    switch (level){
        case NO_LEVEL:
        case MOLECULAR_SUBSPECIES:
            return build_lipid_subspecies_name(MOLECULAR_SUBSPECIES);
    
        case CATEGORY:
        case CLASS:
        case SPECIES:
            return LipidSpecies::get_lipid_string(level);
    
        default:
            throw IllegalArgumentException("LipidMolecularSubspecies does not know how to create a lipid string for level " + std::to_string(level));
    }
}
