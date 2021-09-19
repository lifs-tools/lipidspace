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


#include "cppgoslin/parser/GoslinParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&GoslinParserEventHandler::y, this, placeholders::_1)})
    

GoslinParserEventHandler::GoslinParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    fa_list = new vector<FattyAcid*>();
    
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    
    reg("hg_cl_pre_event", set_head_group_name);
    reg("hg_mlcl_pre_event", set_head_group_name);
    reg("hg_pl_pre_event", set_head_group_name);
    reg("hg_lpl_pre_event", set_head_group_name);
    reg("hg_lpl_o_pre_event", set_head_group_name);
    reg("hg_pl_o_pre_event", set_head_group_name);
    reg("hg_lsl_pre_event", set_head_group_name);
    reg("hg_dsl_pre_event", set_head_group_name);
    reg("st_pre_event", set_head_group_name);
    reg("hg_ste_pre_event", set_head_group_name);
    reg("hg_stes_pre_event", set_head_group_name);
    reg("mediator_pre_event", set_head_group_name);
    reg("hg_mgl_pre_event", set_head_group_name);
    reg("hg_dgl_pre_event", set_head_group_name);
    reg("hg_sgl_pre_event", set_head_group_name);
    reg("hg_tgl_pre_event", set_head_group_name);
    reg("hg_dlcl_pre_event", set_head_group_name);
    reg("hg_sac_di_pre_event", set_head_group_name);
    reg("hg_sac_f_pre_event", set_head_group_name);
    reg("hg_tpl_pre_event", set_head_group_name);  
    
    reg("gl_species_pre_event", set_species_level);
    reg("pl_species_pre_event", set_species_level);
    reg("sl_species_pre_event", set_species_level);
    reg("fa2_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa3_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa4_unsorted_pre_event", set_molecular_subspecies_level);
    reg("slbpa_pre_event", set_molecular_subspecies_level);
    reg("dlcl_pre_event", set_molecular_subspecies_level);
    reg("mlcl_pre_event", set_molecular_subspecies_level);
    
    reg("lcb_pre_event", new_lcb);
    reg("lcb_post_event", clean_lcb);
    reg("fa_pre_event", new_fa);
    reg("fa_post_event", append_fa);
    
    reg("db_single_position_pre_event", set_isomeric_level);
    reg("db_single_position_post_event", add_db_position);
    reg("db_position_number_pre_event", add_db_position_number);
    reg("cistrans_pre_event", add_cistrans);
    
    
    reg("ether_pre_event", add_ether);
    reg("old_hydroxyl_pre_event", add_old_hydroxyl);
    reg("db_count_pre_event", add_double_bonds);
    reg("carbon_pre_event", add_carbon);
    reg("hydroxyl_pre_event", add_hydroxyl);
    
    
    reg("adduct_info_pre_event", new_adduct);
    reg("adduct_pre_event", add_adduct);
    reg("charge_pre_event", add_charge);
    reg("charge_sign_pre_event", add_charge_sign);
    
    
    reg("lpl_pre_event", set_molecular_subspecies_level);
    reg("lpl_o_pre_event", set_molecular_subspecies_level);
    reg("hg_lpl_oc_pre_event", set_unspecified_ether);
    reg("hg_pl_oc_pre_event", set_unspecified_ether);
    debug = "";
}


GoslinParserEventHandler::~GoslinParserEventHandler(){
    delete fa_list;
}


void GoslinParserEventHandler::reset_lipid(TreeNode *node) {
    level = ISOMERIC_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
    adduct = NULL;
    db_position = 0;
    db_cistrans = "";
    unspecified_ether = false;
    headgroup = NULL;
}


void GoslinParserEventHandler::set_unspecified_ether(TreeNode *node) {
    unspecified_ether = true;
}

void GoslinParserEventHandler::set_head_group_name(TreeNode *node) {
    head_group = node->get_text();
}


void GoslinParserEventHandler::set_species_level(TreeNode *node) {
    set_lipid_level(SPECIES);
}


void GoslinParserEventHandler::set_isomeric_level(TreeNode* node){
    db_position = 0;
    db_cistrans = "";
}


void GoslinParserEventHandler::add_db_position(TreeNode* node){
    if (current_fa != NULL){
        current_fa->double_bonds->double_bond_positions.insert({db_position, db_cistrans});
    }
}


void GoslinParserEventHandler::add_db_position_number(TreeNode* node){
    db_position = atoi(node->get_text().c_str());
}


void GoslinParserEventHandler::add_cistrans(TreeNode* node){
    db_cistrans = node->get_text();
}
    
    

void GoslinParserEventHandler::set_molecular_subspecies_level(TreeNode *node) {
    set_lipid_level(MOLECULAR_SUBSPECIES);
}
    
    
void GoslinParserEventHandler::new_fa(TreeNode *node) {
    LipidFaBondType lipid_FA_bond_type = ESTER;
    if (unspecified_ether){
        unspecified_ether = false;
        lipid_FA_bond_type = ETHER_UNSPECIFIED;
    }
    current_fa = new FattyAcid("FA" + std::to_string(fa_list->size() + 1), 2, 0, 0, lipid_FA_bond_type);
}
    
    

void GoslinParserEventHandler::new_lcb(TreeNode *node) {
    lcb = new FattyAcid("LCB");
    current_fa = lcb;
    set_lipid_level(STRUCTURAL_SUBSPECIES);
    lcb->set_type(LCB_REGULAR);
}



void GoslinParserEventHandler::set_lipid_level(LipidLevel _level){
    level = min(level, _level);
}
        
        

void GoslinParserEventHandler::clean_lcb(TreeNode *node) {
    current_fa = NULL;
}
    
    
        

void GoslinParserEventHandler::append_fa(TreeNode *node) {
    if (current_fa->lipid_FA_bond_type == ETHER_UNSPECIFIED){
        throw LipidException("Lipid with unspecified ether bond cannot be treated properly.");
    }
    if (current_fa->double_bonds->double_bond_positions.size() == 0 && current_fa->double_bonds->get_num() > 0){
        level = min(level, STRUCTURAL_SUBSPECIES);
    }
        
    
    if (current_fa->double_bonds->get_num() < 0){
        throw LipidException("Double bond count does not match with number of double bond positions");
    }
    
    if (level == STRUCTURAL_SUBSPECIES || level == ISOMERIC_SUBSPECIES){
            current_fa->position = fa_list->size() + 1;
    }
    

    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    

void GoslinParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        level = min(level, STRUCTURAL_SUBSPECIES);
        for (auto& fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    
    lipid = NULL;
    LipidSpecies *ls = NULL;
    
    headgroup = new Headgroup(head_group);
    
    int true_fa = 0;
    for (auto fa : *fa_list){
        true_fa += fa->num_carbon > 0 || fa->double_bonds->get_num() > 0;
    }
    int poss_fa = contains(LipidClasses::get_instance().lipid_classes, headgroup->lipid_class) ? LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class).possible_num_fa : 0;
    
    // make lyso
    bool can_be_lyso = contains(LipidClasses::get_instance().lipid_classes, Headgroup::get_class("L" + head_group)) ? contains(LipidClasses::get_instance().lipid_classes.at(Headgroup::get_class("L" + head_group)).special_cases, "Lyso") : 0;
    
    if (true_fa + 1 == poss_fa && level != SPECIES && headgroup->lipid_category == GP && can_be_lyso){
        head_group = "L" + head_group;
        delete headgroup;
        headgroup = new Headgroup(head_group);
        poss_fa = contains(LipidClasses::get_instance().lipid_classes, headgroup->lipid_class) ? LipidClasses::get_instance().lipid_classes.at(headgroup->lipid_class).possible_num_fa : 0;
    }
    
    else if (true_fa + 2 == poss_fa && level != SPECIES && headgroup->lipid_category == GP && head_group == "CL"){
        head_group = "DL" + head_group;
        
        headgroup->decorators = 0;
        delete headgroup;
        headgroup = new Headgroup(head_group);
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
    
    
    // make LBC exception
    if (fa_list->size() > 0 && headgroup->sp_exception) fa_list->front()->set_type(LCB_EXCEPTION);
    
    switch (level){
        case SPECIES: ls = new LipidSpecies(headgroup, fa_list); break;
        case MOLECULAR_SUBSPECIES: ls = new LipidMolecularSubspecies(headgroup, fa_list); break;
        case STRUCTURAL_SUBSPECIES: ls = new LipidStructuralSubspecies(headgroup, fa_list); break;
        case ISOMERIC_SUBSPECIES: ls = new LipidIsomericSubspecies(headgroup, fa_list); break;
        default: break;
    }
    lipid = new LipidAdduct();
    lipid->lipid = ls;
    lipid->adduct = adduct;
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
    
}
    
    

void GoslinParserEventHandler::add_ether(TreeNode *node) {
    string ether = node->get_text();
    if (ether == "a") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "p"){
        current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
        current_fa->double_bonds->num_double_bonds = max(0, current_fa->double_bonds->num_double_bonds - 1);
    }
}
    
    

void GoslinParserEventHandler::add_old_hydroxyl(TreeNode *node) {
    string old_hydroxyl = node->get_text();
    int num_h = 0;
    if (old_hydroxyl == "d") num_h = 2;
    else if (old_hydroxyl == "t") num_h = 3;
    
    
    if (Headgroup::get_category(head_group) == SP && (current_fa->lipid_FA_bond_type == LCB_REGULAR || current_fa->lipid_FA_bond_type == LCB_EXCEPTION) && !(head_group == "Cer" || head_group == "LCB")) num_h -= 1;
    
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
}
    
    

void GoslinParserEventHandler::add_double_bonds(TreeNode *node) {
    current_fa->double_bonds->num_double_bonds = atoi(node->get_text().c_str());
}
    
    

void GoslinParserEventHandler::add_carbon(TreeNode *node) {
    current_fa->num_carbon = atoi(node->get_text().c_str());
}
    
    

void GoslinParserEventHandler::add_hydroxyl(TreeNode *node) {
    int num_h = atoi(node->get_text().c_str());

    if (Headgroup::get_category(head_group) == SP && (current_fa->lipid_FA_bond_type == LCB_REGULAR || current_fa->lipid_FA_bond_type == LCB_EXCEPTION) && !(head_group == "Cer" || head_group == "LCB")) num_h -= 1;
    
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
    level = min(level, STRUCTURAL_SUBSPECIES);
}
    
    

void GoslinParserEventHandler::new_adduct(TreeNode *node) {
    adduct = new Adduct("", "", 0, 0);
}
    
    

void GoslinParserEventHandler::add_adduct(TreeNode *node) {
    adduct->adduct_string = node->get_text();
}
    
    

void GoslinParserEventHandler::add_charge(TreeNode *node) {
    adduct->charge = atoi(node->get_text().c_str());
}
    
    

void GoslinParserEventHandler::add_charge_sign(TreeNode *node) {
    string sign = node->get_text();
    if (sign == "+") adduct->set_charge_sign(1);
    else if (sign == "-") adduct->set_charge_sign(-1);
}
        

        
