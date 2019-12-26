#include "Fragment.h"

    
Fragment::Fragment(string _name){
    name = _name;
}
    
string Fragment::get_lipid_string(){
    return name;
}
