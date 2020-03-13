#include "IsomericFattyAcid.h"

#include <iostream>
using namespace std;

IsomericFattyAcid::IsomericFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position, map<int, string> *_double_bond_positions) : StructuralFattyAcid(name, num_carbon, num_double_bonds, num_hydroxyl, lipid_FA_bond_type, lcb, position) {
    if (_double_bond_positions != NULL){
        for (auto kvp : *_double_bond_positions){
            double_bond_positions.insert({kvp.first, kvp.second});
        }
    }
}

IsomericFattyAcid::IsomericFattyAcid(FattyAcid* fa) : StructuralFattyAcid(fa){
    IsomericFattyAcid* iso_fa = dynamic_cast<IsomericFattyAcid*>(fa);
    if (iso_fa){
        for (auto& kvp : iso_fa->double_bond_positions){
            double_bond_positions.insert({kvp.first, kvp.second});
        }
    }
}

IsomericFattyAcid::~IsomericFattyAcid(){
    
}


string IsomericFattyAcid::to_string(bool special_case, LipidLevel level){
    switch(level){        
        case NO_LEVEL:
        case ISOMERIC_SUBSPECIES:
            {
                stringstream s;
                string lipid_suffix = suffix(lipid_FA_bond_type);
                if (special_case && lipid_suffix.length() > 0) s << "O-";
                s << num_carbon << ":" << num_double_bonds;
                
                if (double_bond_positions.size()){
                    stringstream db;
                    db << "(";
                    int j = 0;
                    for (auto it : double_bond_positions){
                        if (j > 0) db << ",";
                        db << it.first << it.second;
                        ++j;
                    }
                    db << ")";
                    s << db.str();
                }
                
                if (num_hydroxyl) s << ";" << num_hydroxyl;
                s << lipid_suffix;
                return s.str();
            }
            
        case STRUCTURAL_SUBSPECIES:
        case MOLECULAR_SUBSPECIES:
        case SPECIES:
            {
                return StructuralFattyAcid::to_string(special_case, level);
            }
            
        default:
            throw IllegalArgumentException("IsomericFattyAcid does not know how to create a fatty acid string for level " + to_string(level));
    }
}