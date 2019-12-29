#include "cppgoslin/parser/GoslinParserEventHandler.h"


GoslinParserEventHandler::GoslinParserEventHandler() : BaseParserEventHandler() {
    reset_lipid(NULL);
    /*
    registered_events["lipid_pre_event"] = reset_lipid
    registered_events["lipid_post_event"] = build_lipid
    
    registered_events["hg_cl_pre_event"] = set_head_group_name
    registered_events["hg_mlcl_pre_event"] = set_head_group_name
    registered_events["hg_pl_pre_event"] = set_head_group_name
    registered_events["hg_lpl_pre_event"] = set_head_group_name
    registered_events["hg_lpl_o_pre_event"] = set_head_group_name
    registered_events["hg_pl_o_pre_event"] = set_head_group_name
    registered_events["hg_lsl_pre_event"] = set_head_group_name
    registered_events["hg_dsl_pre_event"] = set_head_group_name
    registered_events["ch_pre_event"] = set_head_group_name
    registered_events["hg_che_pre_event"] = set_head_group_name
    registered_events["mediator_pre_event"] = set_head_group_name
    registered_events["hg_mgl_pre_event"] = set_head_group_name
    registered_events["hg_dgl_pre_event"] = set_head_group_name
    registered_events["hg_sgl_pre_event"] = set_head_group_name
    registered_events["hg_tgl_pre_event"] = set_head_group_name
    
    registered_events["gl_species_pre_event"] = set_species_level
    registered_events["pl_species_pre_event"] = set_species_level
    registered_events["chc_pre_event"] = set_species_level
    registered_events["sl_species_pre_event"] = set_species_level
    registered_events["fa2_unsorted_pre_event"] = set_molecular_subspecies_level
    registered_events["fa3_unsorted_pre_event"] = set_molecular_subspecies_level
    registered_events["fa4_unsorted_pre_event"] = set_molecular_subspecies_level
    
    registered_events["lcb_pre_event"] = new_lcb
    registered_events["lcb_post_event"] = clean_lcb
    registered_events["fa_pre_event"] = new_fa
    registered_events["fa_post_event"] = append_fa
    
    registered_events["ether_pre_event"] = add_ether
    registered_events["old_hydroxyl_pre_event"] = add_old_hydroxyl
    registered_events["db_count_pre_event"] = add_double_bonds
    registered_events["carbon_pre_event"] = add_carbon
    registered_events["hydroxyl_pre_event"] = add_hydroxyl
    
    
    registered_events["adduct_info_pre_event"] = new_adduct
    registered_events["adduct_pre_event"] = add_adduct
    registered_events["charge_pre_event"] = add_charge
    registered_events["charge_sign_pre_event"] = add_charge_sign
    */
}




void GoslinParserEventHandler::reset_lipid(TreeNode *node) {
    level = STRUCTURAL_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list = new vector<FattyAcid*>;
    current_fa = NULL;
    adduct = NULL;
}

void GoslinParserEventHandler::set_head_group_name(TreeNode *node) {
    head_group = node->get_text();
}

void GoslinParserEventHandler::set_species_level(TreeNode *node) {
    level = SPECIES;
}
    
    
void GoslinParserEventHandler::set_molecular_subspecies_level(TreeNode *node) {
    level = MOLECULAR_SUBSPECIES;
}
    
    
void GoslinParserEventHandler::new_fa(TreeNode *node) {
    switch(level){
        case SPECIES:
            {
                current_fa = new LipidSpeciesInfo();
            }
            break;
        
        case MOLECULAR_SUBSPECIES:
            {
                stringstream s;
                s << "FA" << (fa_list->size() + 1);
                string fa;
                s >> fa;
                current_fa = new MolecularFattyAcid(fa, 2, 0, 0, ESTER, false, -1);
            }
            break;
        
        case STRUCTURAL_SUBSPECIES:
            {
                stringstream s1;
                s1 << "FA" << (fa_list->size() + 1);
                string fa1;
                s1 >> fa1;
                current_fa = new StructuralFattyAcid(fa1, 2, 0, 0, ESTER, false, 0);
            }
            break;
        
        default:
            break;
    }
}
    
    
void GoslinParserEventHandler::new_lcb(TreeNode *node) {
    if (level == SPECIES){
        lcb = new LipidSpeciesInfo();
        lcb->lipid_FA_bond_type = ESTER;
    }
        
    else if (level == STRUCTURAL_SUBSPECIES){
        lcb = new StructuralFattyAcid("LCB", 2, 0, 1, ESTER, true, 1);
    }
    
    current_fa = lcb;
}
        
        
void GoslinParserEventHandler::clean_lcb(TreeNode *node) {
    current_fa = NULL;
}
    
    
        
void GoslinParserEventHandler::append_fa(TreeNode *node) {
    if (level == STRUCTURAL_SUBSPECIES){
        current_fa->position = fa_list->size() + 1;
    }
        
    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    
void GoslinParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        for (auto fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    lipid = NULL;    
    
    LipidSpecies *ls = NULL;
    
    if (level == SPECIES){
        if (fa_list->size() > 0){
            LipidSpeciesInfo *lipid_species_info = new LipidSpeciesInfo(fa_list->at(0));
            lipid_species_info->level = SPECIES;
            ls = new LipidSpecies(head_group, UNDEFINED_CATEGORY, UNDEFINED_CLASS, lipid_species_info);
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
}
    
    
void GoslinParserEventHandler::add_ether(TreeNode *node) {
    string ether = node->get_text();
    if (ether == "a") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "p") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
}
    
    
void GoslinParserEventHandler::add_old_hydroxyl(TreeNode *node) {
    string old_hydroxyl = node->get_text();
    if (old_hydroxyl == "d") current_fa->num_hydroxyl = 2;
    else if (old_hydroxyl == "t") current_fa->num_hydroxyl = 3;
}
    
    
void GoslinParserEventHandler::add_double_bonds(TreeNode *node) {
    ((MolecularFattyAcid*)current_fa)->num_double_bonds = atoi(node->get_text().c_str());
}
    
    
void GoslinParserEventHandler::add_carbon(TreeNode *node) {
    current_fa->num_carbon = atoi(node->get_text().c_str());
}
    
    
void GoslinParserEventHandler::add_hydroxyl(TreeNode *node) {
    current_fa->num_hydroxyl = atoi(node->get_text().c_str());
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
        

        
