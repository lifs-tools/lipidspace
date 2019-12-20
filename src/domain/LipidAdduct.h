#ifndef LIPID_ADDUCT_Y
#define LIPID_ADDUCT_Y

#include "MolecularFattyAcid.h"
#include <string>
#include "LipidExceptions.h"
#include "LipidEnums.h"
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
    string get_lipid_string(LipidLevel level = NULL);
    string get_lipid_fragment_string(LipidLevel level = NULL);
};

#endif /* LIPID_ADDUCT_Y */