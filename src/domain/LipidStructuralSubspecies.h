#ifndef LIPID_STRUCTURAL_SUBSPECIES_H
#define LIPID_STRUCTURAL_SUBSPECIES_H

#include <string>
#include "LipidExceptions.h"
#include "LipidSpeciesInfo.h"
#include "LipidEnums.h"
#include "LipidMolecularSubspecies.h"
#include <sstream>
#include <vector>
#include "FattyAcid.h"
#include <map>

using namespace std;

class LipidStructuralSubspecies : LipidMolecularSubspecies {
public:
    
    LipidStructuralSubspecies(string head_group, vector<FattyAcid*> _fa = NULL);
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
};

#endif /* LIPID_STRUCTURAL_SUBSPECIES_H */
