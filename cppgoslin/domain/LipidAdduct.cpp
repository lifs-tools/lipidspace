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


#include "LipidAdduct.h"


LipidAdduct::LipidAdduct(){
    lipid = NULL;
    adduct = NULL;
    fragment = NULL;
    sum_formula = "";
}

LipidAdduct::~LipidAdduct(){
    if (lipid) delete lipid;
    if (adduct) delete adduct;
    if (fragment) delete fragment;
}
    
    
string LipidAdduct::get_lipid_string(LipidLevel level){
    stringstream s;
    if (lipid) s << lipid->get_lipid_string(level);
    else return "";
    
    if (adduct) s << adduct->get_lipid_string();
    
    return s.str();
}


string LipidAdduct::get_class_name(){
    return (lipid) ? lipid->get_class_name() : "";
}

double LipidAdduct::get_mass(){
    double mass = 0;
    if (lipid) mass += lipid->get_mass();
    if (adduct){
        mass += adduct->get_mass();
        double charge = adduct->get_charge();
        if (charge != 0) mass = (mass - charge * ELECTRON_REST_MASS) / fabs(charge);
    }
    
    return mass;
}

    
    
string LipidAdduct::get_lipid_fragment_string(LipidLevel level){
    stringstream s;
    
    if (lipid) s << lipid->get_lipid_string(level);
    else return "";
    
    if (adduct) s << adduct->get_lipid_string();
    
    if (fragment){
        s << " - " << fragment->get_lipid_string();
    }
    
    return s.str();
}
