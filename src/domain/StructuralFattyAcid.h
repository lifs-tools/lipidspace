#ifndef STRUCTURAL_FATTY_ACID_H
#define STRUCTURAL_FATTY_ACID_H

#include "MolecularFattyAcid.h"
#include <string>
#include "LipidExceptions.h"
#include "LipidEnums.h"

using namespace std;

class StructuralFattyAcid : public MolecularFattyAcid {

public:
    StructuralFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position);
};            

#endif /* STRUCTURAL_FATTY_ACID_H */
