#ifndef LIPID_ADDUCT_H
#define LIPID_ADDUCT_H

#include "MolecularFattyAcid.h"
#include <string>
#include "LipidExceptions.h"
#include "LipidEnums.h"
#include "LipidSpecies.h"
#include "Adduct.h"
#include "Fragment.h"
#include <sstream>

using namespace std;

class LipidAdduct {
public:
    LipidSpecies *lipid;
    Adduct *adduct;
    Fragment *fragment;
    string sum_formula;
    
    LipidAdduct();
    ~LipidAdduct();
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
    string get_lipid_fragment_string(LipidLevel level = UNDEFINED_LEVEL);
};

#endif /* LIPID_ADDUCT_H */
