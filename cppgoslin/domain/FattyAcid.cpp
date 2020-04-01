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


#include "FattyAcid.h"

FattyAcid::FattyAcid(string _name, int _num_carbon, int _num_double_bonds, int _num_hydroxyl, LipidFaBondType _lipid_FA_bond_type, bool _lcb, int _position){
    name = _name;
    position = _position;
    num_carbon = _num_carbon;
    num_double_bonds = _num_double_bonds;
    num_hydroxyl = _num_hydroxyl;
    lipid_FA_bond_type = _lipid_FA_bond_type;
    lcb = _lcb;
    
    if (num_carbon < 2){
        throw ConstraintViolationException("FattyAcid must have at least 2 carbons! Got " + to_string(num_carbon));
    }
    
    if (position < -1){
        throw ConstraintViolationException("FattyAcid position must be greater or equal to -1 (undefined) or greater or equal to 0 (0 = first position)!");
    }
    
    if (num_hydroxyl < 0){
        throw ConstraintViolationException("FattyAcid must have at least 0 hydroxy groups! Got " + to_string(num_hydroxyl));
    }
}

FattyAcid::FattyAcid(FattyAcid* fa){
    name = fa->name;
    position = fa->position;
    num_carbon = fa->num_carbon;
    num_hydroxyl = fa->num_hydroxyl;
    lipid_FA_bond_type = fa->lipid_FA_bond_type;
    lcb = fa->lcb;
}

FattyAcid::~FattyAcid(){

}

string FattyAcid::suffix(LipidFaBondType lipid_FA_bond_type){
    switch(lipid_FA_bond_type){
        case (ETHER_PLASMANYL): return "a";
        case (ETHER_PLASMENYL): return "p";
        default: return "";
    }
}


string FattyAcid::to_string(bool special_case, LipidLevel level){
    stringstream s;
    
    string lipid_suffix = suffix(lipid_FA_bond_type);
    if (special_case && lipid_suffix.length() > 0) s << "O-";
    s << num_carbon << ":" << num_double_bonds;
    
    if (num_hydroxyl) s << ";" << num_hydroxyl;
    s << lipid_suffix;
    return s.str();
}
