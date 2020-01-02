#include "cppgoslin/parser/LipidMapsParserEventHandler.h"


#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&LipidMapsParserEventHandler::y, this, placeholders::_1)})
    

LipidMapsParserEventHandler::LipidMapsParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    fa_list = new vector<FattyAcid*>();
        
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    
    reg("mediator_post_event", mediator_event);
    
    reg("sgl_species_pre_event", set_species_level);
    reg("tgl_species_pre_event", set_species_level);
    reg("dpl_species_pre_event", set_species_level);
    reg("cl_species_pre_event", set_species_level);
    reg("dsl_species_pre_event", set_species_level);
    reg("fa2_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa3_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa4_unsorted_pre_event", set_molecular_subspecies_level);
    
    reg("hg_sgl_pre_event", set_head_group_name);
    reg("hg_gl_pre_event", set_head_group_name);
    reg("hg_cl_pre_event", set_head_group_name);
    reg("hg_dpl_pre_event", set_head_group_name);
    reg("hg_lpl_pre_event", set_head_group_name);
    reg("hg_fourpl_pre_event", set_head_group_name);
    reg("sphingosine_name_pre_event", set_head_group_name);
    reg("sphinganine_name_pre_event", set_head_group_name);
    reg("hg_dsl_pre_event", set_head_group_name);
    reg("ch_pre_event", set_head_group_name);
    reg("hg_che_pre_event", set_head_group_name);
    reg("mediator_pre_event", set_head_group_name);
    
    reg("lcb_pre_event", new_lcb);
    reg("lcb_post_event", clean_lcb);
    reg("fa_pre_event", new_fa);
    reg("fa_post_event", append_fa);
    
    reg("ether_pre_event", add_ether);
    reg("hydroxyl_pre_event", add_hydroxyl);
    reg("hydroxyl_lcb_pre_event", add_hydroxyl_lcb);
    reg("db_count_pre_event", add_double_bonds);
    reg("carbon_pre_event", add_carbon);
    
    reg("mod_text_pre_event", increment_hydroxyl);
} 


LipidMapsParserEventHandler::~LipidMapsParserEventHandler(){
    delete fa_list;
}
    
    
void LipidMapsParserEventHandler::reset_lipid(TreeNode* node){
    level = STRUCTURAL_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
    use_head_group = false;
    omit_fa = false;
}
    
void LipidMapsParserEventHandler::set_molecular_subspecies_level(TreeNode* node){
    level = MOLECULAR_SUBSPECIES;
}
    
    
void LipidMapsParserEventHandler::mediator_event(TreeNode* node){
    use_head_group = true;
}
    
    
void LipidMapsParserEventHandler::set_head_group_name(TreeNode* node){
    head_group = node->get_text();
}
    
    
void LipidMapsParserEventHandler::set_species_level(TreeNode* node){
    level = SPECIES;
}
    
    
void LipidMapsParserEventHandler::increment_hydroxyl(TreeNode* node){
    if (node->get_text() == "OH"){
        current_fa->num_hydroxyl += 1;
    }
}
        
void LipidMapsParserEventHandler::new_fa(TreeNode* node){
    if (level == SPECIES){
        current_fa = new LipidSpeciesInfo();
    }
    
    else if (level == MOLECULAR_SUBSPECIES){
        current_fa = new MolecularFattyAcid("FA" + to_string(fa_list->size() + 1), 2, 0, 0, ESTER, false, -1);
    }
    
    else if (level == STRUCTURAL_SUBSPECIES){
        current_fa = new StructuralFattyAcid("FA" + to_string(fa_list->size() + 1), 2, 0, 0, ESTER, false, 0);
    }
}
    
    
void LipidMapsParserEventHandler::new_lcb(TreeNode* node){
    if (level == SPECIES){
        lcb = new LipidSpeciesInfo();
        lcb->lipid_FA_bond_type = ESTER;
    }
    
    else if (level == STRUCTURAL_SUBSPECIES){
        lcb = new StructuralFattyAcid("LCB", 2, 0, 1, ESTER, true, 1);
    }
    current_fa = lcb;
}
        
        
void LipidMapsParserEventHandler::clean_lcb(TreeNode* node){
    current_fa = NULL;
}
    
    
        
void LipidMapsParserEventHandler::append_fa(TreeNode* node){
    if (level == STRUCTURAL_SUBSPECIES){
        current_fa->position = fa_list->size() + 1;
    }
        
    if (current_fa->num_carbon > 0) fa_list->push_back(current_fa);
    else omit_fa = true;
    current_fa = NULL;
}
    
    
void LipidMapsParserEventHandler::add_ether(TreeNode* node){
    string ether = node->get_text();
    if (ether == "O-") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "P-") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
}
    
    
    
void LipidMapsParserEventHandler::add_hydroxyl(TreeNode* node){
    current_fa->num_hydroxyl = atoi(node->get_text().c_str());
}
    
void LipidMapsParserEventHandler::add_hydroxyl_lcb(TreeNode* node){
    string hydroxyl = node->get_text();
    if (hydroxyl == "m") current_fa->num_hydroxyl = 1;
    else if (hydroxyl == "d") current_fa->num_hydroxyl = 2;
    else if (hydroxyl == "t") current_fa->num_hydroxyl = 3;
}
    
    
void LipidMapsParserEventHandler::add_double_bonds(TreeNode* node){
    ((MolecularFattyAcid*)current_fa)->num_double_bonds = atoi(node->get_text().c_str());
}
    
    
void LipidMapsParserEventHandler::add_carbon(TreeNode* node){
    current_fa->num_carbon = atoi(node->get_text().c_str());
}
    
    

void LipidMapsParserEventHandler::build_lipid(TreeNode* node){
    
    if (omit_fa && head_group_exceptions.find(head_group) != head_group_exceptions.end()){
        head_group = "L" + head_group;
    }
    
    if (lcb != NULL){
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
        else {
            ls = new LipidSpecies(head_group);
        }
    }
    else if (level == MOLECULAR_SUBSPECIES) {
        ls = new LipidMolecularSubspecies(head_group, fa_list);
    }
        
    else if (level == STRUCTURAL_SUBSPECIES) {
        ls = new LipidStructuralSubspecies(head_group, fa_list);
    }
    
    ls->use_head_group = use_head_group;

    lipid = new LipidAdduct();
    lipid->lipid = ls;
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
}
    
        
