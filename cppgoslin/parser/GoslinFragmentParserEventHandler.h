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


#ifndef GOSLIN_FRAGMENT_PARSER_EVENT_HANDLER_H
#define GOSLIN_FRAGMENT_PARSER_EVENT_HANDLER_H

#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/Adduct.h"
#include "cppgoslin/domain/LipidAdduct.h"
#include "cppgoslin/domain/Fragment.h"
#include "cppgoslin/domain/LipidStructuralSubspecies.h"
#include "cppgoslin/domain/LipidIsomericSubspecies.h"
#include "cppgoslin/domain/FattyAcid.h"
#include "cppgoslin/domain/IsomericFattyAcid.h"
#include "cppgoslin/parser/BaseParserEventHandler.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

class GoslinFragmentParserEventHandler : public BaseParserEventHandler<LipidAdduct*> {
public:
    LipidLevel level;
    LipidAdduct *lipid;
    string head_group;
    FattyAcid *lcb;
    vector<FattyAcid*> *fa_list;
    FattyAcid *current_fa;
    Adduct *adduct;
    Fragment *fragment;
    int db_position;
    string db_cistrans;
        
    GoslinFragmentParserEventHandler();
    ~GoslinFragmentParserEventHandler();
    void reset_lipid(TreeNode *node);
    void set_head_group_name(TreeNode *node);
    void set_species_level(TreeNode *node);
    void set_molecular_subspecies_level(TreeNode *node);
    void new_fa(TreeNode *node);
    void new_lcb(TreeNode *node);
    void clean_lcb(TreeNode *node);
    void append_fa(TreeNode *node);
    void build_lipid(TreeNode *node);
    void add_ether(TreeNode *node);
    void add_old_hydroxyl(TreeNode *node);
    void add_double_bonds(TreeNode *node);
    void add_carbon(TreeNode *node);
    void add_hydroxyl(TreeNode *node);
    void new_adduct(TreeNode *node);
    void add_adduct(TreeNode *node);
    void add_charge(TreeNode *node);
    void add_charge_sign(TreeNode *node);
    void add_fragment_name(TreeNode *node);
    
    void set_isomeric_level(TreeNode* node);
    void add_db_position(TreeNode* node);
    void add_db_position_number(TreeNode* node);
    void add_cistrans(TreeNode* node);
};


#endif /* GOSLIN_FRAGMENT_PARSER_EVENT_HANDLER_H */
        
