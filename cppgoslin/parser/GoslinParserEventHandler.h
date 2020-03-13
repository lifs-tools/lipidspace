#ifndef GOSLIN_PARSER_EVENT_HANDLER_H
#define GOSLIN_PARSER_EVENT_HANDLER_H

#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/Adduct.h"
#include "cppgoslin/domain/LipidAdduct.h"
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

class GoslinParserEventHandler : public BaseParserEventHandler<LipidAdduct*> {
public:
    LipidLevel level;
    LipidAdduct *lipid;
    string head_group;
    FattyAcid *lcb;
    vector<FattyAcid*> *fa_list;
    FattyAcid *current_fa;
    Adduct *adduct;
    int db_position;
    string db_cistrans;
        
    GoslinParserEventHandler();
    ~GoslinParserEventHandler();
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
    
    void set_isomeric_level(TreeNode* node);
    void add_db_position(TreeNode* node);
    void add_db_position_number(TreeNode* node);
    void add_cistrans(TreeNode* node);
};


#endif /* GOSLIN_PARSER_EVENT_HANDLER_H */
        
