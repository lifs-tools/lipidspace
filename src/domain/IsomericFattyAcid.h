#ifndef LIPID_ADDUCT_Y
#define LIPID_ADDUCT_Y

#include "StructuralFattyAcid.h"
#include <string>
#include "LipidEnums.h"
#include <vector>

using namespace std;

class IsomericFattyAcid : StructuralFattyAcid {
public:
    vector<int> double_bond_positions;
    
    IsomericFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, vector<int> *_double_bond_positions, int position);
};

#endif /* LIPID_ADDUCT_Y */