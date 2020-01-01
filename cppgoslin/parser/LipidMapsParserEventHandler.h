#ifndef LIPID_MAPS_PARSER_EVENT_HANDLER_H
#define LIPID_MAPS_PARSER_EVENT_HANDLER_H

#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/Adduct.h"
#include "cppgoslin/domain/LipidAdduct.h"
#include "cppgoslin/domain/LipidStructuralSubspecies.h"
#include "cppgoslin/domain/FattyAcid.h"
#include "cppgoslin/parser/BaseParserEventHandler.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

static const set<string> head_group_exceptions = {"PA", "PC", "PE", "PG", "PI", "PS"};

class LipidMapsParserEventHandler : public BaseParserEventHandler<LipidAdduct*> {
public:
    LipidLevel level;
    LipidAdduct *lipid;
    string head_group;
    FattyAcid *lcb;
    vector<FattyAcid*> *fa_list;
    FattyAcid *current_fa;
    bool omit_fa;
    bool use_head_group;

    LipidMapsParserEventHandler();
    ~LipidMapsParserEventHandler();
    void reset_lipid(TreeNode* node);
    void set_molecular_subspecies_level(TreeNode* node);
    void mediator_event(TreeNode* node);
    void set_head_group_name(TreeNode* node);
    void set_species_level(TreeNode* node);
    void increment_hydroxyl(TreeNode* node);
    void new_fa(TreeNode* node);
    void new_lcb(TreeNode* node);
    void clean_lcb(TreeNode* node);
    void append_fa(TreeNode* node);
    void add_ether(TreeNode* node);
    void add_hydroxyl(TreeNode* node);
    void add_double_bonds(TreeNode* node);
    void add_carbon(TreeNode* node);
    void build_lipid(TreeNode* node);
    void add_hydroxyl_lcb(TreeNode* node);
        
};
#endif /* LIPID_MAPS_PARSER_EVENT_HANDLER_H */
