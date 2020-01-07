#include "cppgoslin/parser/GoslinFragmentParserEventHandler.h"


#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&GoslinFragmentParserEventHandler::y, this, placeholders::_1)})
    

GoslinFragmentParserEventHandler::GoslinFragmentParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
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
    reg("ch_pre_event", set_head_group_name);
    reg("hg_che_pre_event", set_head_group_name);
    reg("mediator_pre_event", set_head_group_name);
    reg("hg_mgl_pre_event", set_head_group_name);
    reg("hg_dgl_pre_event", set_head_group_name);
    reg("hg_sgl_pre_event", set_head_group_name);
    reg("hg_tgl_pre_event", set_head_group_name);
    
    reg("gl_species_pre_event", set_species_level);
    reg("pl_species_pre_event", set_species_level);
    reg("chc_pre_event", set_species_level);
    reg("sl_species_pre_event", set_species_level);
    reg("fa2_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa3_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa4_unsorted_pre_event", set_molecular_subspecies_level);
    
    
    reg("lcb_pre_event", new_lcb);
    reg("lcb_post_event", clean_lcb);
    reg("fa_pre_event", new_fa);
    reg("fa_post_event", append_fa);
    
    
    reg("ether_pre_event", add_ether);
    reg("old_hydroxyl_pre_event", add_old_hydroxyl);
    reg("db_count_pre_event", add_double_bonds);
    reg("carbon_pre_event", add_carbon);
    reg("hydroxyl_pre_event", add_hydroxyl);
    
    
    reg("adduct_info_pre_event", new_adduct);
    reg("adduct_pre_event", add_adduct);
    reg("charge_pre_event", add_charge);
    reg("charge_sign_pre_event", add_charge_sign);
    
    reg("fragment_name_pre_event", add_fragment);
}


GoslinFragmentParserEventHandler::~GoslinFragmentParserEventHandler(){
    delete fa_list;
}


void GoslinFragmentParserEventHandler::reset_lipid(TreeNode *node) {
    level = STRUCTURAL_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
    adduct = NULL;
    fragment = NULL;
}


void GoslinFragmentParserEventHandler::set_head_group_name(TreeNode *node) {
    head_group = node->get_text();
}


void GoslinFragmentParserEventHandler::set_species_level(TreeNode *node) {
    level = SPECIES;
}
    
    

void GoslinFragmentParserEventHandler::set_molecular_subspecies_level(TreeNode *node) {
    level = MOLECULAR_SUBSPECIES;
}
    
    

void GoslinFragmentParserEventHandler::new_fa(TreeNode *node) {
    switch(level){
        case SPECIES:
            {
                current_fa = new LipidSpeciesInfo();
            }
            break;
        
        case MOLECULAR_SUBSPECIES:
            {
                current_fa = new MolecularFattyAcid("FA" + to_string(fa_list->size() + 1), 2, 0, 0, ESTER, false, -1);
            }
            break;
        
        case STRUCTURAL_SUBSPECIES:
            {
                current_fa = new StructuralFattyAcid("FA" + to_string(fa_list->size() + 1), 2, 0, 0, ESTER, false, 0);
            }
            break;
        
        default:
            break;
    }
}
    
    

void GoslinFragmentParserEventHandler::new_lcb(TreeNode *node) {
    if (level == SPECIES){
        lcb = new LipidSpeciesInfo();
        lcb->lipid_FA_bond_type = ESTER;
    }
        
    else if (level == STRUCTURAL_SUBSPECIES){
        lcb = new StructuralFattyAcid("LCB", 2, 0, 1, ESTER, true, 1);
    }
    
    current_fa = lcb;
}
        
        

void GoslinFragmentParserEventHandler::clean_lcb(TreeNode *node) {
    current_fa = NULL;
}
    
    
        

void GoslinFragmentParserEventHandler::append_fa(TreeNode *node) {
    if (level == STRUCTURAL_SUBSPECIES){
        current_fa->position = fa_list->size() + 1;
    }
        
    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    

void GoslinFragmentParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        for (auto& fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    
    lipid = NULL;
    LipidSpecies *ls = NULL;
    
    

    
    if (level == SPECIES){
        if (fa_list->size() > 0){
            LipidSpeciesInfo *lipid_species_info = new LipidSpeciesInfo(fa_list->at(0));
            lipid_species_info->level = SPECIES;
            ls = new LipidSpecies(head_group, NO_CATEGORY, NO_CLASS, lipid_species_info);
        }
        else{
            ls = new LipidSpecies(head_group);
        }
    }
        
    else if (level == MOLECULAR_SUBSPECIES){
        ls = new LipidMolecularSubspecies(head_group, fa_list);
    }
        
    else if (level == STRUCTURAL_SUBSPECIES){
        ls = new LipidStructuralSubspecies(head_group, fa_list);
    }
    
    lipid = new LipidAdduct();
    lipid->lipid = ls;
    lipid->adduct = adduct;
    lipid->fragment = fragment;
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
    
}
    
    

void GoslinFragmentParserEventHandler::add_ether(TreeNode *node) {
    string ether = node->get_text();
    if (ether == "a") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "p") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
}
    
    

void GoslinFragmentParserEventHandler::add_old_hydroxyl(TreeNode *node) {
    string old_hydroxyl = node->get_text();
    if (old_hydroxyl == "d") current_fa->num_hydroxyl = 2;
    else if (old_hydroxyl == "t") current_fa->num_hydroxyl = 3;
}
    
    

void GoslinFragmentParserEventHandler::add_double_bonds(TreeNode *node) {
    ((MolecularFattyAcid*)current_fa)->num_double_bonds = atoi(node->get_text().c_str());
}
    
    

void GoslinFragmentParserEventHandler::add_carbon(TreeNode *node) {
    current_fa->num_carbon = atoi(node->get_text().c_str());
}
    
    

void GoslinFragmentParserEventHandler::add_hydroxyl(TreeNode *node) {
    current_fa->num_hydroxyl = atoi(node->get_text().c_str());
}
    
    

void GoslinFragmentParserEventHandler::new_adduct(TreeNode *node) {
    adduct = new Adduct("", "", 0, 0);
}
    
    

void GoslinFragmentParserEventHandler::add_adduct(TreeNode *node) {
    adduct->adduct_string = node->get_text();
}
    
    

void GoslinFragmentParserEventHandler::add_charge(TreeNode *node) {
    adduct->charge = atoi(node->get_text().c_str());
}
    
    

void GoslinFragmentParserEventHandler::add_charge_sign(TreeNode *node) {
    string sign = node->get_text();
    if (sign == "+") adduct->set_charge_sign(1);
    else if (sign == "-") adduct->set_charge_sign(-1);
}
        

        
void GoslinFragmentParserEventHandler::add_fragment(TreeNode *node){
    fragment = new Fragment(node->get_text());
}
