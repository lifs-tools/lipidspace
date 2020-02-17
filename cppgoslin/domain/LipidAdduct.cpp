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
