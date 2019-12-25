#ifndef ADDUCT_H
#define ADDUCT_H

#include <string>
#include "LipidExceptions.h"
#include "LipidStructuralSubspecies.h"
#include <sstream>
#include <vector>

using namespace std;

class LipidIsomericSubspecies : LipidStructuralSubspecies {

    LipidIsomericSubspecies(string head_group, vector<fattyAcid*>* fa);
    string build_lipid_isomeric_substructure_name();
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
            
};

#endif /* ADDUCT_H*/
