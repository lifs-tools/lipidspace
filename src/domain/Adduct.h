#ifndef ADDUCT_H
#define ADDUCT_H

#include <string>
#include "LipidExceptions.h"
#include <sstream>

using namespace std;

class Adduct{
public:
    string sum_formula;
    string adduct_string;
    int charge;
    int charge_sign;
    
    Adduct(string _sum_formula, string _adduct_string, int _charge, int _sign);
    void set_charge_sign(int sign);
    string get_lipid_string();
};

#endif /* ADDUCT_H */
