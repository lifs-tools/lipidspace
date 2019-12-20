
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
    
    
string LipidAdduct::get_lipid_string(LipidLevel level = NULL){
    stringstream s;
    
    if (lipid) s << lipid->get_lipid_string(level);
    else return "";
    
    if (adduct) s << self.adduct.get_lipid_string();
    
    string lipid_name;
    s >> lipid_name;
    
    return lipid_name;
}
    
    
string LipidAdduct::get_lipid_fragment_string(LipidLevel level = NULL){
    stringstream s;
    
    if (lipid) s << lipid->get_lipid_string(level);
    else return "";
    
    if (adduct) s << self.adduct.get_lipid_string();
    
    if (fragment){
        s << " - " << self.fragment.get_lipid_string();
    }
    
    string lipid_name;
    s >> lipid_name;
    
    return lipid_name;
}