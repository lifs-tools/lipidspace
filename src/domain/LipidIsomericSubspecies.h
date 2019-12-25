#ifndef LIPID_ISOMERIC_SUBSPECIES_H
#define LIPID_ISOMERIC_SUBSPECIES_H

#include <string>
#include "LipidExceptions.h"
#include "LipidStructuralSubspecies.h"
#include "IsomericFattyAcid.h"
#include <sstream>
#include <vector>

using namespace std;

class LipidIsomericSubspecies : public LipidStructuralSubspecies {

    LipidIsomericSubspecies(string head_group, vector<FattyAcid*>* fa = NULL);
    string build_lipid_isomeric_substructure_name();
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
            
};

#endif /* LIPID_ISOMERIC_SUBSPECIES_H */
