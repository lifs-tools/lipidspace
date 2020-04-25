/*
MIT License

Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de
                   Nils Hoffmann  -  nils.hoffmann {at} isas.de

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

LipidMolecularSubspecies::LipidMolecularSubspecies (string _head_group) : LipidSpecies(_head_group) {
    info.level = MOLECULAR_SUBSPECIES;
}


LipidMolecularSubspecies::LipidMolecularSubspecies (string _head_group, vector<FattyAcid*> *_fa) : LipidSpecies(_head_group) {
    
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = ESTER;
    if (_fa) {
        for (unsigned int i = 0; i < _fa->size(); ++i){
            FattyAcid *fas = new FattyAcid(_fa->at(i));
            delete _fa->at(i);
            
            if (fa.find(fas->name) != fa.end()){
                throw ConstraintViolationException("FA names must be unique! FA with name " + fas->name + " was already added!");
            }
            
            else {
                fa.insert({fas->name, fas});
                fa_list.push_back(fas);
                num_carbon += fas->num_carbon;
                num_hydroxyl += fas->num_hydroxyl;
                num_double_bonds += fas->num_double_bonds;
                
                if (lipid_FA_bond_type == ESTER && (fas->lipid_FA_bond_type == ETHER_PLASMANYL || fas->lipid_FA_bond_type == ETHER_PLASMENYL)){
                    lipid_FA_bond_type = fas->lipid_FA_bond_type;
                }
                
                else if (lipid_FA_bond_type != ESTER && (fas->lipid_FA_bond_type == ETHER_PLASMANYL || fas->lipid_FA_bond_type == ETHER_PLASMENYL)){
                    throw ConstraintViolationException("Only one FA can define an ether bond to the head group! Tried to add " + to_string(fas->lipid_FA_bond_type) + " over existing " + to_string(lipid_FA_bond_type));
                }
            }
        }
    }
    info.level = MOLECULAR_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
    
}

LipidMolecularSubspecies::~LipidMolecularSubspecies(){
    
}

#include <iostream>
using namespace std;

string LipidMolecularSubspecies::build_lipid_subspecies_name(string fa_separator, LipidLevel level){
    stringstream s;
    s << (!use_head_group ? get_class_string(lipid_class) : head_group);
    bool special_case = (lipid_class == PC) | (lipid_class == LPC) | (lipid_class == PE) | (lipid_class == LPE);
    
    if (fa_list.size() > 0){
        s << ((lipid_category !=  ST) ? " " : "/");
        for (unsigned int i = 0; i < fa_list.size(); ++i){
            if (i > 0) s << fa_separator;
            FattyAcid *fatty_acid = fa_list.at(i);
            s << fatty_acid->to_string(special_case);
        }
    }
    
    return s.str();
}


LipidLevel LipidMolecularSubspecies::get_lipid_level(){
    return MOLECULAR_SUBSPECIES;
}


string LipidMolecularSubspecies::get_lipid_string(LipidLevel level) {
    switch (level){
        case NO_LEVEL:
        case MOLECULAR_SUBSPECIES:
            if (!validate()){
                stringstream st;
                st << "Number of fatty acyl chains for '" << get_class_string(lipid_class);
                st << "' is incorrect, should be [";
                int ii = 0;
                for (auto p : LipidClasses::get_instance().lipid_classes.at(lipid_class).possible_num_fa){
                    if (ii++ > 0) st << ", ";
                    st << p;
                }
                st << "], present: " << fa.size();
            
                throw ConstraintViolationException(st.str());
            }
            return build_lipid_subspecies_name("-", level);
    
        case CATEGORY:
        case CLASS:
        case SPECIES:
            return LipidSpecies::get_lipid_string(level);
    
        default:
            stringstream s;
            s << "LipidMolecularSubspecies does not know how to create a lipid string for level " << level;
            string error_message;
            s >> error_message;
            throw IllegalArgumentException("LipidMolecularSubspecies does not know how to create a lipid string for level " + to_string(level));
    }
}


bool LipidMolecularSubspecies::validate(){
    /*
    if (use_head_group) return true;
    if (lipid_classes.find(lipid_class) == lipid_classes.end()) return false;
    if (lipid_classes.at(lipid_class).max_num_fa == 0) return true;
    if ((int)fa_list.size() > lipid_classes.at(lipid_class).max_num_fa) return false;
    if (lipid_classes.at(lipid_class).possible_num_fa.find(fa_list.size()) == lipid_classes.at(lipid_class).possible_num_fa.end()) return false;
    
    int num_lcb = 0;
    for (auto& kv : fa){
        num_lcb += kv.second->lcb;
    }
    if (lipid_category == SP && num_lcb != 1) return false;
    */
    return true;
}
