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


#include "LipidSpeciesInfo.h"

LipidSpeciesInfo::LipidSpeciesInfo () : FattyAcid() {
    level = UNDEFINED_LEVEL;
}


LipidSpeciesInfo::LipidSpeciesInfo (FattyAcid *fa) : FattyAcid(fa) {
    level = UNDEFINED_LEVEL;
}

void LipidSpeciesInfo::clone(FattyAcid *fa){
    if (fa) {
        name = fa->name;
        position = fa->position;
        num_carbon = fa->num_carbon;
        num_hydroxyl = fa->num_hydroxyl;
        num_double_bonds = fa->num_double_bonds;
        lipid_FA_bond_type = fa->lipid_FA_bond_type;
        lcb = fa->lcb;
        for (auto kv : fa->double_bond_positions){
            double_bond_positions.insert({kv.first, kv.second});
        }
    }
    else {
        name = "";
        position = 0;
        num_carbon = 0;
        num_double_bonds = 0;
        num_hydroxyl = 0;
        lipid_FA_bond_type = ESTER;
        lcb = false;
    }
}


ElementTable* LipidSpeciesInfo::get_elements(int num_fa){

    ElementTable* elements = FattyAcid::get_elements();
    elements->at(ELEMENT_O) += num_fa - 1;
    elements->at(ELEMENT_H) -= num_fa - 1;
    
    return elements;
}
