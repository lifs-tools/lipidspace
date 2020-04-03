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


#ifndef ISOMERIC_FATTY_ACID_H
#define ISOMERIC_FATTY_ACID_H

#include "cppgoslin/domain/StructuralFattyAcid.h"
#include <string>
#include "cppgoslin/domain/LipidEnums.h"
#include <map>
#include <typeinfo>

using namespace std;

class IsomericFattyAcid : public StructuralFattyAcid {
public:
    map<int, string> double_bond_positions;
    
    IsomericFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position, map<int, string> *_double_bond_positions);
    IsomericFattyAcid(FattyAcid* fa);
    string to_string(bool special_case, LipidLevel level = NO_LEVEL);
    ~IsomericFattyAcid();
};

#endif /* ISOMERIC_FATTY_ACID_H */
