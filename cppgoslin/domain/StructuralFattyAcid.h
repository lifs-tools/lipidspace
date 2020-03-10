#ifndef STRUCTURAL_FATTY_ACID_H
#define STRUCTURAL_FATTY_ACID_H

#include "cppgoslin/domain/MolecularFattyAcid.h"
#include <string>
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidEnums.h"

using namespace std;

class StructuralFattyAcid : public MolecularFattyAcid {

public:
    StructuralFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position);
    ~StructuralFattyAcid();
};

#endif /* STRUCTURAL_FATTY_ACID_H */
