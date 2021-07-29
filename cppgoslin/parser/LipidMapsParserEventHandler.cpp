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


#include "cppgoslin/parser/LipidMapsParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&LipidMapsParserEventHandler::y, this, placeholders::_1)})
    

LipidMapsParserEventHandler::LipidMapsParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    fa_list = new vector<FattyAcid*>();
        
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    
    reg("mediator_pre_event", mediator_event);
    reg("sphingoxine_pre_event", mediator_event);
    
    reg("sgl_species_pre_event", set_species_level);
    reg("species_fa_pre_event", set_species_level);
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
    reg("hg_threepl_pre_event", set_head_group_name);
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
    
    reg("structural_mod_pre_event", set_structural_subspecies_level);
    reg("single_mod_pre_event", set_mod);
    reg("mod_text_pre_event", set_mod_text);
    reg("mod_pos_pre_event", set_mod_pos);
    reg("mod_num_pre_event", set_mod_num);
    reg("single_mod_post_event", add_functional_group);
    
    debug = "";
} 


LipidMapsParserEventHandler::~LipidMapsParserEventHandler(){
    delete fa_list;
}
    
    
void LipidMapsParserEventHandler::reset_lipid(TreeNode* node){
    level = ISOMERIC_SUBSPECIES;
    lipid = NULL;
    head_group = "";
    lcb = NULL;
    fa_list->clear();
    current_fa = NULL;
    use_head_group = false;
    omit_fa = false;
    db_position = 0;
    db_numbers = -1;
    db_cistrans = "";
    headgroup = NULL;
    mod_pos = -1;
    mod_num = 1;
    mod_text = "";
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
    db_position = 0;
    db_cistrans = "";
}


void LipidMapsParserEventHandler::add_db_position(TreeNode* node){
    if (current_fa != NULL){
        current_fa->double_bonds->double_bond_positions.insert({db_position, db_cistrans});
        
        if (db_cistrans != "E" && db_cistrans != "Z"){
            level = min(level, STRUCTURAL_SUBSPECIES);
        }
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
   
   
   
void LipidMapsParserEventHandler::set_structural_subspecies_level(TreeNode* node){
    level = min(level, STRUCTURAL_SUBSPECIES);
}


void LipidMapsParserEventHandler::set_mod(TreeNode* node){
    mod_text = "";
    mod_pos = -1;
    mod_num = 1;
}


void LipidMapsParserEventHandler::set_mod_text(TreeNode* node){
    mod_text = node->get_text();
}


void LipidMapsParserEventHandler::set_mod_pos(TreeNode* node){
    mod_pos = atoi(node->get_text().c_str());
}


void LipidMapsParserEventHandler::set_mod_num(TreeNode* node){
    mod_num = atoi(node->get_text().c_str());
}   
    
    
void LipidMapsParserEventHandler::add_functional_group(TreeNode* node){
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group(mod_text);
    functional_group->position = mod_pos;
    functional_group->count = mod_num;
    string fg_name = functional_group->name;
    if (uncontains_p(current_fa->functional_groups, fg_name)) current_fa->functional_groups->insert({fg_name, vector<FunctionalGroup*>()});
    current_fa->functional_groups->at(fg_name).push_back(functional_group);
}
        
        
void LipidMapsParserEventHandler::new_fa(TreeNode *node) {
    db_numbers = -1;
    current_fa = new FattyAcid("FA" + std::to_string(fa_list->size() + 1));
}
    
    

void LipidMapsParserEventHandler::new_lcb(TreeNode *node) {
    lcb = new FattyAcid("LCB");
    lcb->lcb = true;
    current_fa = lcb;
}
        
        

void LipidMapsParserEventHandler::clean_lcb(TreeNode *node) {
    if (db_numbers > -1 && db_numbers != current_fa->double_bonds->get_num()){
        throw LipidException("Double bond count does not match with number of double bond positions");
    }
    if (current_fa->double_bonds->double_bond_positions.size() == 0 && current_fa->double_bonds->get_num() > 0){
        level = min(level, STRUCTURAL_SUBSPECIES);
    }
    current_fa = NULL;
}
    
    
        

void LipidMapsParserEventHandler::append_fa(TreeNode *node) {
    if (db_numbers > -1 && db_numbers != current_fa->double_bonds->get_num()){
        throw LipidException("Double bond count does not match with number of double bond positions");
    }
    if (current_fa->double_bonds->double_bond_positions.size() == 0 && current_fa->double_bonds->get_num() > 0){
        level = min(level, STRUCTURAL_SUBSPECIES);
    }
    
    if (level == STRUCTURAL_SUBSPECIES || level == ISOMERIC_SUBSPECIES){
            current_fa->position = fa_list->size() + 1;
    }
    
    if (current_fa->num_carbon == 0){
        omit_fa = true;
    }
    fa_list->push_back(current_fa);
    current_fa = NULL;
}
    
    
void LipidMapsParserEventHandler::add_ether(TreeNode* node){
    string ether = node->get_text();
    if (ether == "O-") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "P-") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
}
    
    
    
void LipidMapsParserEventHandler::add_hydroxyl(TreeNode* node){
    int num_h = atoi(node->get_text().c_str());
    
    if (Headgroup::get_category(head_group) == SP && current_fa->lcb && head_group != "Cer" && head_group != "LCB") num_h -= 1;
    
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
}


    
void LipidMapsParserEventHandler::add_hydroxyl_lcb(TreeNode* node){
    string hydroxyl = node->get_text();
    int num_h = 0;
    if (hydroxyl == "m") num_h = 1;
    else if (hydroxyl == "d") num_h = 2;
    else if (hydroxyl == "t") num_h = 3;
    
    
    if (Headgroup::get_category(head_group) == SP && current_fa->lcb && head_group != "Cer" && head_group != "LCB") num_h -= 1;
    
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
}
    
    
void LipidMapsParserEventHandler::add_double_bonds(TreeNode* node){
    current_fa->double_bonds->num_double_bonds += atoi(node->get_text().c_str());
}
    
    
void LipidMapsParserEventHandler::add_carbon(TreeNode* node){
    current_fa->num_carbon = atoi(node->get_text().c_str());
}
    
    

void LipidMapsParserEventHandler::build_lipid(TreeNode* node){
    if (omit_fa && head_group_exceptions.find(head_group) != head_group_exceptions.end()){
        head_group = "L" + head_group;
    }
    
    if (lcb != NULL){
        level = min(level, STRUCTURAL_SUBSPECIES);
        for (auto fa : *fa_list) fa->position += 1;
        fa_list->insert(fa_list->begin(), lcb);
    }
    
    lipid = NULL;
    LipidSpecies *ls = NULL;
    
    headgroup = new Headgroup(head_group, 0, use_head_group);
    
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
    
        
