#ifndef LIPID_STRUCTURAL_SUBSPECIES_H
#define LIPID_STRUCTURAL_SUBSPECIES_H

#include <string>
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidSpeciesInfo.h"
#include "cppgoslin/domain/StructuralFattyAcid.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/LipidMolecularSubspecies.h"
#include <sstream>
#include <vector>
#include "cppgoslin/domain/FattyAcid.h"
#include <map>

using namespace std;

class LipidStructuralSubspecies : public LipidMolecularSubspecies {
public:
    
    LipidStructuralSubspecies(string head_group, vector<FattyAcid*> *_fa = NULL);
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
};

#endif /* LIPID_STRUCTURAL_SUBSPECIES_H */
