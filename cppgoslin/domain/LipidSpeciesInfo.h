#ifndef LIPID_SPECIES_INFO_H
#define LIPID_SPECIES_INFO_H

#include <string>
#include "cppgoslin/domain/FattyAcid.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/MolecularFattyAcid.h"
#include <typeinfo>

using namespace std;

class LipidSpeciesInfo : public MolecularFattyAcid {
    
public:
    LipidLevel level;
    LipidSpeciesInfo (FattyAcid *fa = NULL);
};
        
#endif /* LIPID_SPECIES_INFO_H */
