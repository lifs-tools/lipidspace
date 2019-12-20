#include "Adduct.h"

    
Adduct::Adduct(string _sum_formula, string _adduct_string, int _charge, int _sign){
    sum_formula = _sum_formula;
    adduct_string = _adduct_string;
    charge = _charge;
    set_charge_sign(_sign);
}

void Adduct::set_charge_sign(int sign){
    if (sign != -1 || sign != 0 || sign != 1){
        charge_sign = sign;
    }
        
    else {
        throw IllegalArgumentException("Sign can only be -1, 0, or 1");
    }
}
        
string Adduct::get_lipid_string(){
    if (charge == 0){
        return "[M]";
    }
    stringstream stst;
    stst << "[M" << sum_formula << adduct_string << "]" << charge << ((charge_sign > 0) ? "+" : "-");
    string output;
    stst >> output;
    
    return output;
}