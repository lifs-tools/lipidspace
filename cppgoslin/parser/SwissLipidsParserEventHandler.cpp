#include "cppgoslin/parser/SwissLipidsParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&SwissLipidsParserEventHandler::y, this, placeholders::_1)})
    

SwissLipidsParserEventHandler::SwissLipidsParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    fa_list = new vector<FattyAcid*>();
    
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    
    reg("gl_hg_pre_event", set_head_group_name);
    reg("gl_mono_hg_pure_pre_event", set_head_group_name);
    reg("pl_hg_pre_event", set_head_group_name);
    reg("pl_four_hg_pure_pre_event", set_head_group_name);
    reg("sl_hg_pre_event", set_head_group_name);
    
    
    reg("fa_species_pre_event", set_species_level);
    
    
    reg("lcb_pre_event", new_lcb);
    reg("lcb_post_event", clean_lcb);
    reg("fa_pre_event", new_fa);
    reg("fa_post_event", append_fa);
    reg("ether_pre_event", add_ether);
    reg("hydroxyl_pre_event", add_hydroxyl);
    reg("db_pre_event", add_double_bonds);
    reg("carbon_pre_event", add_carbon);
    
}


SwissLipidsParserEventHandler::~SwissLipidsParserEventHandler(){
    delete fa_list;
}


void SwissLipidsParserEventHandler::reset_lipid(TreeNode *node) {
    level = STRUCTURAL_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
}



void SwissLipidsParserEventHandler::set_head_group_name(TreeNode *node) {
    head_group = node->get_text();
}


void SwissLipidsParserEventHandler::set_species_level(TreeNode *node) {
    level = SPECIES;
}
    

    
    

void SwissLipidsParserEventHandler::new_fa(TreeNode *node) {
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
    
    

void SwissLipidsParserEventHandler::new_lcb(TreeNode *node) {
    if (level == SPECIES){
        lcb = new LipidSpeciesInfo();
        lcb->lipid_FA_bond_type = ESTER;
    }
        
    else if (level == STRUCTURAL_SUBSPECIES){
        lcb = new StructuralFattyAcid("LCB", 2, 0, 1, ESTER, true, 1);
    }
    
    current_fa = lcb;
}
        
        

void SwissLipidsParserEventHandler::clean_lcb(TreeNode *node) {
    current_fa = NULL;
}
    
    
        

void SwissLipidsParserEventHandler::append_fa(TreeNode *node) {
    if (level == STRUCTURAL_SUBSPECIES){
        current_fa->position = fa_list->size() + 1;
    }
        
    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    

void SwissLipidsParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        for (auto& fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    
    lipid = NULL;
    LipidSpecies *ls = NULL;
    
    

    
    if (level == SPECIES){
        if (fa_list->size() > 0){
            LipidSpeciesInfo lipid_species_info(fa_list->at(0));
            delete fa_list->at(0);
            lipid_species_info.level = SPECIES;
            ls = new LipidSpecies(head_group, NO_CATEGORY, NO_CLASS, &lipid_species_info);
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
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
    
}
    
    

void SwissLipidsParserEventHandler::add_ether(TreeNode *node) {
    string ether = node->get_text();
    if (ether == "O-") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "P-") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
}
    
    

void SwissLipidsParserEventHandler::add_hydroxyl(TreeNode *node) {
    string old_hydroxyl = node->get_text();
    if (old_hydroxyl == "m") current_fa->num_hydroxyl = 1;
    else if (old_hydroxyl == "d") current_fa->num_hydroxyl = 2;
    else if (old_hydroxyl == "t") current_fa->num_hydroxyl = 3;
}
    
    

void SwissLipidsParserEventHandler::add_double_bonds(TreeNode *node) {
    ((MolecularFattyAcid*)current_fa)->num_double_bonds = atoi(node->get_text().c_str());
}
    
    

void SwissLipidsParserEventHandler::add_carbon(TreeNode *node) {
    current_fa->num_carbon = atoi(node->get_text().c_str());
}

        

        
