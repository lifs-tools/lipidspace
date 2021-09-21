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
    

GoslinParserEventHandler::GoslinParserEventHandler() : LipidBaseParserEventHandler() {    
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
}


void GoslinParserEventHandler::reset_lipid(TreeNode *node) {
    level = FULL_STRUCTURE;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
    adduct = NULL;
    db_position = 0;
    db_cistrans = "";
    unspecified_ether = false;
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
        if (db_cistrans != "E" && db_cistrans != "Z") set_lipid_level(STRUCTURE_DEFINED);
    }
}


void GoslinParserEventHandler::add_db_position_number(TreeNode* node){
    db_position = atoi(node->get_text().c_str());
}


void GoslinParserEventHandler::add_cistrans(TreeNode* node){
    db_cistrans = node->get_text();
}
    
    

void GoslinParserEventHandler::set_molecular_subspecies_level(TreeNode *node) {
    set_lipid_level(MOLECULAR_SPECIES);
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
    set_lipid_level(STRUCTURE_DEFINED);
    lcb->set_type(LCB_REGULAR);
}
        
        

void GoslinParserEventHandler::clean_lcb(TreeNode *node) {
    if (current_fa->double_bonds->double_bond_positions.size() == 0 && current_fa->double_bonds->get_num() > 0){
        set_lipid_level(SN_POSITION);
    }
    current_fa = NULL;
}
    
    
        

void GoslinParserEventHandler::append_fa(TreeNode *node) {
    if (current_fa->lipid_FA_bond_type == ETHER_UNSPECIFIED){
        throw LipidException("Lipid with unspecified ether bond cannot be treated properly.");
    }
    if (current_fa->double_bonds->double_bond_positions.size() == 0 && current_fa->double_bonds->get_num() > 0){
        set_lipid_level(SN_POSITION);
    }
        
    
    if (current_fa->double_bonds->get_num() < 0){
        throw LipidException("Double bond count does not match with number of double bond positions");
    }
    
    if (is_level(level, COMPLETE_STRUCTURE | FULL_STRUCTURE | STRUCTURE_DEFINED | SN_POSITION)){
            current_fa->position = fa_list->size() + 1;
    }
    

    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    

void GoslinParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        set_lipid_level(STRUCTURE_DEFINED);
        for (auto& fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    Headgroup *headgroup = prepare_headgroup_and_checks();
    
    LipidAdduct *lipid = new LipidAdduct();
    lipid->lipid = assemble_lipid(headgroup);
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

    if (sp_regular_lcb()) num_h -= 1;
    
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

    if (sp_regular_lcb()) num_h -= 1;
    
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
    level = min(level, STRUCTURE_DEFINED);
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
        

        
