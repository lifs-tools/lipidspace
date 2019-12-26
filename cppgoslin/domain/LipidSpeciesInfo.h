#ifndef LIPID_SPECIES_INFO_H
#define LIPID_SPECIES_INFO_H

#include <string>
#include "cppgoslin/domain/FattyAcid.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/MolecularFattyAcid.h"
#include <typeinfo>

using namespace std;

class LipidSpeciesInfo {
    
public:
    LipidLevel level;
    int num_carbon;
    int num_hydroxyl;
    int num_double_bonds;
    LipidFaBondType lipid_FA_bond_type;
    
    LipidSpeciesInfo (FattyAcid *fa = NULL);
};
        
#endif /* LIPID_SPECIES_INFO_H */
