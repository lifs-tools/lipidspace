#ifndef LIPID_ADDUCT_H
#define LIPID_ADDUCT_H

#include "cppgoslin/domain/MolecularFattyAcid.h"
#include <string>
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/LipidSpecies.h"
#include "cppgoslin/domain/Adduct.h"
#include "cppgoslin/domain/Fragment.h"
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
    string get_lipid_string(LipidLevel level = NO_LEVEL);
    string get_lipid_fragment_string(LipidLevel level = NO_LEVEL);
    string get_class_name();
};

#endif /* LIPID_ADDUCT_H */
