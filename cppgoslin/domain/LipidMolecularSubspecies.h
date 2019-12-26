#ifndef LIPID_MOLECULAR_SUBSPECIES_H
#define LIPID_MOLECULAR_SUBSPECIES_H

#include "cppgoslin/domain/MolecularFattyAcid.h"
#include <string>
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidSpecies.h"
#include "cppgoslin/domain/LipidEnums.h"
#include <sstream>

using namespace std;

class LipidMolecularSubspecies : public LipidSpecies {
public:
    map<string, FattyAcid*> fa;
    vector<FattyAcid*> fa_list;

    LipidMolecularSubspecies (string head_group, vector<FattyAcid*> *_fa = NULL);
    ~LipidMolecularSubspecies();
    string build_lipid_subspecies_name(string fa_separator);
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
};

#endif /* LIPID_MOLECULAR_SUBSPECIES_H */
