/*
MIT License

Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de
                   Nils Hoffmann  -  nils.hoffmann {at} isas.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


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
    
    return stst.str();
}

ElementTable* Adduct::get_elements(){
    ElementTable* elements = create_empty_table();
    try{
        
        //SumFormulaParser* adduct_sum_formula_parser = SumFormulaParser::get_instance();
        string adduct_name = adduct_string.substr(1);
        ElementTable* adduct_elements = SumFormulaParser::get_instance().parse(adduct_name);
        for (auto e : *adduct_elements) elements->at(e.first) += e.second;
        delete adduct_elements;
        
    }
    catch (...) {
        return elements;
    }
    
    if (adduct_string.length() > 0 && adduct_string[0] == '-'){
        for (auto e : element_order){
            elements->at(e) *= -1;
        }
    }
    
    return elements;
}



int Adduct::get_charge(){
    return charge * charge_sign;
}



