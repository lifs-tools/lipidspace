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