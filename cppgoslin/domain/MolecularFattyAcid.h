#ifndef MOLECULAR_FATTY_ACID_H
#define MOLECULAR_FATTY_ACID_H

#include "cppgoslin/domain/FattyAcid.h"
#include <string>
#include "cppgoslin/domain/LipidExceptions.h"
#include <typeinfo>

using namespace std;

class MolecularFattyAcid : public FattyAcid {

public:
    MolecularFattyAcid(string name, int num_carbon, int _num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position = -1);
    MolecularFattyAcid(FattyAcid* fa);
    string to_string(bool special_case);
    ~MolecularFattyAcid();
};            

#endif /* MOLECULAR_FATTY_ACID_H */
