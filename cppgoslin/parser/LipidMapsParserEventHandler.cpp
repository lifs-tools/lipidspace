/*
MIT License

Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de
                   Nils Hoffmann  -  nils.hoffmann {at} isas.de

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


#include "cppgoslin/parser/LipidMapsParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&LipidMapsParserEventHandler::y, this, placeholders::_1)})
    

LipidMapsParserEventHandler::LipidMapsParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    fa_list = new vector<FattyAcid*>();
        
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    
    reg("mediator_var_pre_event", mediator_event);
    reg("sphingoxine_pre_event", mediator_event);
    
    reg("sgl_species_pre_event", set_species_level);
    reg("tgl_species_pre_event", set_species_level);
    reg("dpl_species_pre_event", set_species_level);
    reg("cl_species_pre_event", set_species_level);
    reg("dsl_species_pre_event", set_species_level);
    reg("fa2_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa3_unsorted_pre_event", set_molecular_subspecies_level);
    reg("fa4_unsorted_pre_event", set_molecular_subspecies_level);
    
    reg("fa_no_hg_pre_event", pure_fa);
    
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
    reg("mediator_const_pre_event", set_head_group_name);
    reg("pk_hg_pre_event", set_head_group_name);
    reg("hg_fa_pre_event", set_head_group_name);
    reg("hg_lsl_pre_event", set_head_group_name);
    
    reg("lcb_pre_event", new_lcb);
    reg("lcb_post_event", clean_lcb);
    reg("fa_pre_event", new_fa);
    reg("fa_post_event", append_fa);
    
    reg("db_single_position_pre_event", set_isomeric_level);
    reg("db_single_position_post_event", add_db_position);
    reg("db_position_number_pre_event", add_db_position_number);
    reg("cistrans_pre_event", add_cistrans);
    
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
    db_position = 0;
    db_cistrans = "";
}
    
void LipidMapsParserEventHandler::set_molecular_subspecies_level(TreeNode* node){
    level = MOLECULAR_SUBSPECIES;
}

void LipidMapsParserEventHandler::pure_fa(TreeNode* node){
    head_group = "FA";
}

    
    
void LipidMapsParserEventHandler::mediator_event(TreeNode* node){
    use_head_group = true;
    head_group = node->get_text();
}


void LipidMapsParserEventHandler::set_isomeric_level(TreeNode* node){
    level = ISOMERIC_SUBSPECIES;
    db_position = 0;
    db_cistrans = "";
}


void LipidMapsParserEventHandler::add_db_position(TreeNode* node){
    if (current_fa != NULL){
        current_fa->double_bond_positions.insert({db_position, db_cistrans});
    }
}


void LipidMapsParserEventHandler::add_db_position_number(TreeNode* node){
    db_position = atoi(node->get_text().c_str());
}


void LipidMapsParserEventHandler::add_cistrans(TreeNode* node){
    db_cistrans = node->get_text();
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
        
void LipidMapsParserEventHandler::new_fa(TreeNode *node) {
    current_fa = new FattyAcid("FA" + std::to_string(fa_list->size() + 1), 2, 0, 0, ESTER, false, 0, NULL);
}
    
    

void LipidMapsParserEventHandler::new_lcb(TreeNode *node) {
    lcb = new FattyAcid("LCB", 2, 0, 1, ESTER, true, 1, NULL);
    current_fa = lcb;
}
        
        

void LipidMapsParserEventHandler::clean_lcb(TreeNode *node) {
    if (level == SPECIES){
        FattyAcid* tmp_lcb = lcb;
        lcb = new LipidSpeciesInfo(tmp_lcb);
        lcb->lipid_FA_bond_type = ESTER;
        delete tmp_lcb;
    }
    
    current_fa = NULL;
}
    
    
        

void LipidMapsParserEventHandler::append_fa(TreeNode *node) {
    switch(level){
        case SPECIES:
            {
                FattyAcid* tmp_fa = current_fa;
                current_fa = new LipidSpeciesInfo(tmp_fa);
                delete tmp_fa;
            }
            break;
        
        case STRUCTURAL_SUBSPECIES:
        case ISOMERIC_SUBSPECIES:
            {
                current_fa->position = fa_list->size() + 1;
            }
            break;
        
        default:
            break;
    }
    
    if (current_fa->num_carbon > 0) fa_list->push_back(current_fa);
    else{
        omit_fa = true;
        delete current_fa;
    }
    current_fa = NULL;
}
    
    
void LipidMapsParserEventHandler::add_ether(TreeNode* node){
    string ether = node->get_text();
    if (ether == "O-") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "P-"){
        current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
        current_fa->num_double_bonds += 1;
    }
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
    current_fa->num_double_bonds += atoi(node->get_text().c_str());
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
            LipidSpeciesInfo lipid_species_info(fa_list->at(0));
            delete fa_list->at(0);
            lipid_species_info.level = SPECIES;
            ls = new LipidSpecies(head_group, NO_CATEGORY, NO_CLASS, &lipid_species_info);
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
        
    else if (level == ISOMERIC_SUBSPECIES){
        ls = new LipidIsomericSubspecies(head_group, fa_list);
    }
    
    
    ls->use_head_group = use_head_group;
    lipid = new LipidAdduct();
    lipid->lipid = ls;
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
}
    
        
