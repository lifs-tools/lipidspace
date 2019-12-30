#include "IsomericFattyAcid.h"

IsomericFattyAcid::IsomericFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, map<int, string> *_double_bond_positions, int position) : StructuralFattyAcid(name, num_carbon, num_double_bonds, num_hydroxyl, lipid_FA_bond_type, lcb, position) {
    for (auto kvp : *_double_bond_positions){
        double_bond_positions.insert({kvp.first, kvp.second});
    }
}

