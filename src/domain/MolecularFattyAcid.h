#ifndef MOLECULAR_FATTY_ACID_H
#define MOLECULAR_FATTY_ACID_H

#include "FattyAcid.h"
#include <string>
#include "LipidExceptions.h"

using namespace std;

class MolecularFattyAcid : FattyAcid {

public:
    int num_double_bonds;
    
    MolecularFattyAcid(string name, int num_carbon, int _num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position = -1);
};            

#endif /* MOLECULAR_FATTY_ACID_H */