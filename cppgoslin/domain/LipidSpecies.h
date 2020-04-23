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


#ifndef LIPID_SPECIES_H
#define LIPID_SPECIES_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "cppgoslin/domain/Element.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidSpeciesInfo.h"


using namespace std;
using namespace goslin;


class LipidSpeciesInfo;



class LipidSpecies {
public:
    string head_group;
    LipidCategory lipid_category;
    LipidClass lipid_class;
    LipidSpeciesInfo info;
    bool use_head_group;
    
    map<string, FattyAcid*> fa;
    vector<FattyAcid*> fa_list;
    
    virtual ~LipidSpecies();
    //LipidSpecies(string _head_group);
    LipidSpecies(string _head_group, LipidCategory _lipid_category = NO_CATEGORY, LipidClass lipid_class = NO_CLASS, LipidSpeciesInfo* lipid_species_info = NULL);
    virtual string get_lipid_string(LipidLevel level = NO_LEVEL);
    string get_class_name();
    ElementTable* get_elements();
    
    static LipidCategory get_category(string _head_group);
    static LipidClass get_class(string _head_group);
    static string get_class_string(LipidClass lipid_class);
    static string get_category_string(LipidCategory lipid_category);
    virtual LipidLevel get_lipid_level();
    virtual vector<FattyAcid*> get_fa_list();
    virtual bool validate();
};
            
#endif /* LIPID_SPECIES_H */
