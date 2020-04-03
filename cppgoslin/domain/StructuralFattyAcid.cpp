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


#include "StructuralFattyAcid.h"


StructuralFattyAcid::StructuralFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position) : MolecularFattyAcid(name, num_carbon, num_double_bonds, num_hydroxyl, lipid_FA_bond_type, lcb, position){
    
}

StructuralFattyAcid::StructuralFattyAcid(FattyAcid* fa) : MolecularFattyAcid(fa){
    
}

StructuralFattyAcid::~StructuralFattyAcid(){
    
}


string StructuralFattyAcid::to_string(bool special_case, LipidLevel level){
    switch(level){
        case NO_LEVEL:            
        case STRUCTURAL_SUBSPECIES:
            return FattyAcid::to_string(special_case);
            
        case MOLECULAR_SUBSPECIES:
        case SPECIES:
            return MolecularFattyAcid::to_string(special_case, level);
            
        default:
            throw IllegalArgumentException("StructuralFattyAcid does not know how to create a fatty acid string for level " + to_string(level));
    }
}