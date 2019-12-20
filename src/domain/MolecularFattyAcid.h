#ifndef MOLECULAR_FATTY_ACID_H
#define MOLECULAR_FATTY_ACID_H

#include "FattyAcid.h"
#include <string>
#include "LipidExceptions.h"

using namespace std;

class MolecularFattyAcid : FattyAcid {

public:
    int num_double_bonds;
    
    MolecularFattyAcid(string name, int num_carbon, int _num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position = -1) : FattyAcid(name, num_carbon, num_hydroxyl, lipid_FA_bond_type, lcb, position){
        if (_num_double_bonds < 0){
            throw ConstraintViolationException("MolecularFattyAcid must have at least 0 double bonds!");
        }
            
        num_double_bonds = _num_double_bonds;
    }
};            

#endif /* MOLECULAR_FATTY_ACID_H */