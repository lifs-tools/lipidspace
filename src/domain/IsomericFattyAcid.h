#ifndef ISOMERIC_FATTY_ACID_H
#define ISOMERIC_FATTY_ACID_H

#include "StructuralFattyAcid.h"
#include <string>
#include "LipidEnums.h"
#include <map>

using namespace std;

class IsomericFattyAcid : StructuralFattyAcid {
public:
    map<int, string>* double_bond_positions;
    
    IsomericFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, map<int, string> *_double_bond_positions, int position);
    
    ~IsomericFattyAcid();
};

#endif /* ISOMERIC_FATTY_ACID_H */
