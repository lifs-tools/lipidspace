/*
MIT License

Copyright (c) the authors (listed in global LICENSE file)

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


#include "cppgoslin/parser/SwissLipidsParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&SwissLipidsParserEventHandler::y, this, placeholders::_1)})
    

SwissLipidsParserEventHandler::SwissLipidsParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    fa_list = new vector<FattyAcid*>();
    
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    reg("fa_hg_pre_event", set_head_group_name);
    reg("gl_hg_pre_event", set_head_group_name);
    reg("gl_molecular_hg_pre_event", set_head_group_name);
    reg("mediator_pre_event", mediator_event);
    reg("gl_mono_hg_pre_event", set_head_group_name);
    reg("pl_hg_pre_event", set_head_group_name);
    reg("pl_three_hg_pre_event", set_head_group_name);
    reg("pl_four_hg_pre_event", set_head_group_name);
    reg("sl_hg_pre_event", set_head_group_name);
    reg("st_species_hg_pre_event", set_head_group_name);
    reg("st_sub1_hg_pre_event", set_head_group_name);
    reg("st_sub2_hg_pre_event", set_head_group_name_se);
    reg("fa_species_pre_event", set_species_level);
    reg("gl_molecular_pre_event", set_molecular_level);
    reg("unsorted_fa_separator_pre_event", set_molecular_level);
    reg("fa2_unsorted_pre_event", set_molecular_level);
    reg("fa3_unsorted_pre_event", set_molecular_level);
    reg("fa4_unsorted_pre_event", set_molecular_level);
    reg("db_single_position_pre_event", set_isomeric_level);
    reg("db_single_position_post_event", add_db_position);
    reg("db_position_number_pre_event", add_db_position_number);
    reg("cistrans_pre_event", add_cistrans);
    reg("lcb_pre_event", new_lcb);
    reg("lcb_post_event", clean_lcb);
    reg("fa_pre_event", new_fa);
    reg("fa_post_event", append_fa);
    reg("ether_pre_event", add_ether);
    reg("hydroxyl_pre_event", add_hydroxyl);
    reg("db_count_pre_event", add_double_bonds);
    reg("carbon_pre_event", add_carbon);
    reg("sl_lcb_species_pre_event", set_species_level);
    reg("st_species_fa_post_event", set_species_fa);
    reg("fa_lcb_suffix_type_pre_event", add_fa_lcb_suffix_type);
    reg("fa_lcb_suffix_number_pre_event", add_suffix_number);
    reg("pl_three_post_event", set_nape);
    
    debug = "";
}


SwissLipidsParserEventHandler::~SwissLipidsParserEventHandler(){
    delete fa_list;
}


void SwissLipidsParserEventHandler::reset_lipid(TreeNode *node) {
    level = ISOMERIC_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
    use_head_group = false;
    db_position = 0;
    db_cistrans = "";
    headgroup = NULL;
    headgroup_decorators = new vector<HeadgroupDecorator*>();
    suffix_number = -1;
}


void SwissLipidsParserEventHandler::set_nape(TreeNode *node){
    head_group = "PE-N";
    HeadgroupDecorator* hgd = new HeadgroupDecorator("decorator_acyl", -1, 1, 0, true);
    headgroup_decorators->push_back(hgd);
    hgd->functional_groups->insert({"decorator_acyl", vector<FunctionalGroup*>()});
    hgd->functional_groups->at("decorator_acyl").push_back(fa_list->at(fa_list->size() - 1));
    fa_list->pop_back();
}


void SwissLipidsParserEventHandler::set_isomeric_level(TreeNode* node){
    db_position = 0;
    db_cistrans = "";
}


void SwissLipidsParserEventHandler::add_db_position(TreeNode* node){
    if (current_fa != NULL){
        current_fa->double_bonds->double_bond_positions.insert({db_position, db_cistrans});
    }
}


void SwissLipidsParserEventHandler::add_db_position_number(TreeNode* node){
    db_position = atoi(node->get_text().c_str());
}


void SwissLipidsParserEventHandler::add_cistrans(TreeNode* node){
    db_cistrans = node->get_text();
}


void SwissLipidsParserEventHandler::set_head_group_name(TreeNode *node) {
    head_group = node->get_text();
}


void SwissLipidsParserEventHandler::set_head_group_name_se(TreeNode *node){
    head_group = replace_all(node->get_text(), "(", " ");
}



void SwissLipidsParserEventHandler::set_level(LipidLevel _level){
    level = min(level, _level);
}



void SwissLipidsParserEventHandler::set_species_level(TreeNode *node) {
    set_level(SPECIES);
}
    



void SwissLipidsParserEventHandler::set_molecular_level(TreeNode *node) {
    set_level(MOLECULAR_SUBSPECIES);
}


void SwissLipidsParserEventHandler::mediator_event(TreeNode* node){
    use_head_group = true;
    head_group = node->get_text();
}
    
    

void SwissLipidsParserEventHandler::new_fa(TreeNode *node) {
    current_fa = new FattyAcid("FA" + to_string(fa_list->size() + 1));
}
    
    

void SwissLipidsParserEventHandler::new_lcb(TreeNode *node) {
    lcb = new FattyAcid("LCB");
    lcb->lcb = true;
    current_fa = lcb;
    set_level(STRUCTURAL_SUBSPECIES);
}
        
        

void SwissLipidsParserEventHandler::clean_lcb(TreeNode *node) {
    current_fa = NULL;
}
    
    
        

void SwissLipidsParserEventHandler::append_fa(TreeNode *node) {
    if (current_fa->double_bonds->get_num() < 0){
        throw LipidException("Double bond count does not match with number of double bond positions");
    }
    
    if (current_fa->double_bonds->double_bond_positions.size() == 0 && current_fa->double_bonds->get_num() > 0){
        level = min(level, STRUCTURAL_SUBSPECIES);
    }
    
    if (level == STRUCTURAL_SUBSPECIES || level == ISOMERIC_SUBSPECIES){
            current_fa->position = fa_list->size() + 1;
    }
    
    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    

void SwissLipidsParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        level = min(level, STRUCTURAL_SUBSPECIES);
        for (auto& fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    lipid = NULL;
    LipidSpecies *ls = NULL;

    headgroup = new Headgroup(head_group, headgroup_decorators, use_head_group);
    
    int true_fa = 0;
    for (auto fa : *fa_list){
        true_fa += fa->num_carbon > 0 || fa->double_bonds->get_num() > 0;
    }
    int poss_fa = contains(LipidClasses::get_instance().lipid_classes, headgroup->lipid_class) ? LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class).possible_num_fa : 0;
    
    
    // make lyso
    bool can_be_lyso = contains(LipidClasses::get_instance().lipid_classes, Headgroup::get_class("L" + head_group)) ? contains(LipidClasses::get_instance().lipid_classes.at(Headgroup::get_class("L" + head_group)).special_cases, "Lyso") : 0;
    
    if (true_fa + 1 == poss_fa && level != SPECIES && headgroup->lipid_category == GP && can_be_lyso){
        head_group = "L" + head_group;
        
        headgroup->decorators = 0;
        delete headgroup;
        headgroup = new Headgroup(head_group, headgroup_decorators, use_head_group);
        poss_fa = contains(LipidClasses::get_instance().lipid_classes, headgroup->lipid_class) ? LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class).possible_num_fa : 0;
    }
    
    else if (true_fa + 2 == poss_fa && level != SPECIES && headgroup->lipid_category == GP && head_group == "CL"){
        head_group = "DL" + head_group;
        
        headgroup->decorators = 0;
        delete headgroup;
        headgroup = new Headgroup(head_group, headgroup_decorators, use_head_group);
        poss_fa = contains(LipidClasses::get_instance().lipid_classes, headgroup->lipid_class) ? LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class).possible_num_fa : 0;
    }
    
    
    
    if (level == SPECIES){
        if (true_fa == 0 && poss_fa != 0){
            string hg_name = headgroup->headgroup;
            delete headgroup;
            throw ConstraintViolationException("No fatty acyl information lipid class '" + hg_name + "' provided.");
        }
    }
        
    else if (true_fa != poss_fa && (level == ISOMERIC_SUBSPECIES || level == STRUCTURAL_SUBSPECIES)){
        string hg_name = headgroup->headgroup;
        delete headgroup;
        throw ConstraintViolationException("Number of described fatty acyl chains (" + std::to_string(true_fa) + ") not allowed for lipid class '" + hg_name + "' (having " + std::to_string(poss_fa) + " fatty aycl chains).");
    }
    
    int max_num_fa = contains(LipidClasses::get_instance().lipid_classes, headgroup->lipid_class) ? LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class).max_num_fa : 0;
    if (max_num_fa != (int)fa_list->size()) level = min(level, MOLECULAR_SUBSPECIES);

    switch (level){
        case SPECIES: ls = new LipidSpecies(headgroup, fa_list); break;
        case MOLECULAR_SUBSPECIES: ls = new LipidMolecularSubspecies(headgroup, fa_list); break;
        case STRUCTURAL_SUBSPECIES: ls = new LipidStructuralSubspecies(headgroup, fa_list); break;
        case ISOMERIC_SUBSPECIES: ls = new LipidIsomericSubspecies(headgroup, fa_list); break;
        default: break;
    }
    lipid = new LipidAdduct();
    lipid->lipid = ls;
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
}
    
    

void SwissLipidsParserEventHandler::add_ether(TreeNode *node) {
    string ether = node->get_text();
    if (ether == "O-") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "P-") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
}
    
    

void SwissLipidsParserEventHandler::add_hydroxyl(TreeNode *node) {
    string old_hydroxyl = node->get_text();
    int num_h = 0;
    if (old_hydroxyl == "m") num_h = 1;
    else if (old_hydroxyl == "d") num_h = 2;
    else if (old_hydroxyl == "t") num_h = 3;
    
    
    if (Headgroup::get_category(head_group) == SP && current_fa->lcb && head_group != "Cer" && head_group != "LCB") num_h -= 1;
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
}


void SwissLipidsParserEventHandler::add_one_hydroxyl(TreeNode *node) {
    if (contains_p(current_fa->functional_groups, "OH") && current_fa->functional_groups->at("OH").at(0)->position == -1){
        current_fa->functional_groups->at("OH").at(0)->count += 1;
    }
    else {
        FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
        if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
        current_fa->functional_groups->at("OH").push_back(functional_group);
    }
}
    
    

void SwissLipidsParserEventHandler::add_double_bonds(TreeNode *node) {
    current_fa->double_bonds->num_double_bonds += atoi(node->get_text().c_str());
}



void SwissLipidsParserEventHandler::add_suffix_number(TreeNode *node){
    suffix_number = atoi(node->get_text().c_str());
}



void SwissLipidsParserEventHandler::add_fa_lcb_suffix_type(TreeNode *node){
    string suffix_type = node->get_text();
    if (suffix_type == "me"){
        suffix_type = "Me";
        current_fa->num_carbon -= 1;
    }
        
    FunctionalGroup *functional_group = KnownFunctionalGroups::get_functional_group(suffix_type);
    functional_group->position = suffix_number;
    if (functional_group->position == -1) set_level(STRUCTURAL_SUBSPECIES);
    if (uncontains_p(current_fa->functional_groups, suffix_type)) current_fa->functional_groups->insert({suffix_type, vector<FunctionalGroup*>()});
    current_fa->functional_groups->at(suffix_type).push_back(functional_group);
            
    suffix_number = -1;
}
    
    

void SwissLipidsParserEventHandler::add_carbon(TreeNode *node) {
    current_fa->num_carbon = atoi(node->get_text().c_str());
}

        

void SwissLipidsParserEventHandler::set_species_fa(TreeNode *node){
    head_group += " 27:1";
    fa_list->at(fa_list->size() -1)->num_carbon -= 27;
    fa_list->at(fa_list->size() -1)->double_bonds->num_double_bonds -= 1;
}
        
