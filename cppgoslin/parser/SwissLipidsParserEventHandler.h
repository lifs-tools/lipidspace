#ifndef SWISS_LIPIDS_PARSER_EVENT_HANDLER_H
#define SWISS_LIPIDS_PARSER_EVENT_HANDLER_H

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
#include <fstream>
#include <sstream>

using namespace std;

class SwissLipidsParserEventHandler : public BaseParserEventHandler<LipidAdduct*> {
public:
    LipidLevel level;
    LipidAdduct *lipid;
    string head_group;
    FattyAcid *lcb;
    vector<FattyAcid*> *fa_list;
    FattyAcid *current_fa;
    bool use_head_group;
        
    SwissLipidsParserEventHandler();
    ~SwissLipidsParserEventHandler();
    
    void reset_lipid(TreeNode *node);
    void build_lipid(TreeNode *node);
    void set_head_group_name(TreeNode *node);
    void set_species_level(TreeNode *node);
    void set_molecular_level(TreeNode *node);
    void new_lcb(TreeNode *node);
    void clean_lcb(TreeNode *node);
    void new_fa(TreeNode *node);
    void append_fa(TreeNode *node);
    void add_ether(TreeNode *node);
    void add_hydroxyl(TreeNode *node);
    void add_double_bonds(TreeNode *node);
    void add_carbon(TreeNode *node);
    void mediator_event(TreeNode* node);
    
};


#endif /* SWISS_LIPIDS_PARSER_EVENT_HANDLER_H */
        
