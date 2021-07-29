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


#include "cppgoslin/parser/HmdbParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&HmdbParserEventHandler::y, this, placeholders::_1)})
    

HmdbParserEventHandler::HmdbParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
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
    reg("st_sub2_hg_pre_event", set_head_group_name);
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
    reg("fa_lcb_suffix_type_pre_event", add_one_hydroxyl);
    reg("furan_fa_pre_event", furan_fa);
    reg("interlink_fa_pre_event", interlink_fa);
    reg("lipid_suffix_pre_event", lipid_suffix);
    
}


HmdbParserEventHandler::~HmdbParserEventHandler(){
    delete fa_list;
}


void HmdbParserEventHandler::reset_lipid(TreeNode *node) {
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
}


void HmdbParserEventHandler::set_isomeric_level(TreeNode* node){
    db_position = 0;
    db_cistrans = "";
}


void HmdbParserEventHandler::add_db_position(TreeNode* node){
    if (current_fa != NULL){
        current_fa->double_bonds->double_bond_positions.insert({db_position, db_cistrans});
    }
}


void HmdbParserEventHandler::add_db_position_number(TreeNode* node){
    db_position = atoi(node->get_text().c_str());
}


void HmdbParserEventHandler::add_cistrans(TreeNode* node){
    db_cistrans = node->get_text();
}


void HmdbParserEventHandler::set_head_group_name(TreeNode *node) {
    head_group = node->get_text();
}


void HmdbParserEventHandler::set_species_level(TreeNode *node) {
    level = SPECIES;
}
    



void HmdbParserEventHandler::set_molecular_level(TreeNode *node) {
    level = MOLECULAR_SUBSPECIES;
}


void HmdbParserEventHandler::mediator_event(TreeNode* node){
    use_head_group = true;
    head_group = node->get_text();
}
    
    

void HmdbParserEventHandler::new_fa(TreeNode *node) {
    current_fa = new FattyAcid("FA" + to_string(fa_list->size() + 1));
}
    
    

void HmdbParserEventHandler::new_lcb(TreeNode *node) {
    lcb = new FattyAcid("LCB");
    lcb->lcb = true;
    current_fa = lcb;
}
        
        

void HmdbParserEventHandler::clean_lcb(TreeNode *node) {
    current_fa = NULL;
}
    
    
        

void HmdbParserEventHandler::append_fa(TreeNode *node) {
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
    
    

void HmdbParserEventHandler::build_lipid(TreeNode *node) {
    if (lcb){
        level = min(level, STRUCTURAL_SUBSPECIES);
        for (auto& fa : *fa_list) fa->position += 1;
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
    
    

void HmdbParserEventHandler::add_ether(TreeNode *node) {
    string ether = node->get_text();
    if (ether == "O-" || ether == "o-") current_fa->lipid_FA_bond_type = ETHER_PLASMANYL;
    else if (ether == "P-") current_fa->lipid_FA_bond_type = ETHER_PLASMENYL;
    else throw UnsupportedLipidException("Fatty acyl chain of type '" + ether + "' is currently not supported");
}
    
    

void HmdbParserEventHandler::add_hydroxyl(TreeNode *node) {
    string old_hydroxyl = node->get_text();
    int num_h = 0;
    if (old_hydroxyl == "d") num_h = 2;
    else if (old_hydroxyl == "t") num_h = 3;
    
    
    if (Headgroup::get_category(head_group) == SP && current_fa->lcb && head_group != "Cer" && head_group != "LCB") num_h -= 1;
    
    FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
    functional_group->count = num_h;
    if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
    current_fa->functional_groups->at("OH").push_back(functional_group);
}


void HmdbParserEventHandler::add_one_hydroxyl(TreeNode *node) {
    if (contains_p(current_fa->functional_groups, "OH") && current_fa->functional_groups->at("OH").at(0)->position == -1){
        current_fa->functional_groups->at("OH").at(0)->count += 1;
    }
    else {
        FunctionalGroup* functional_group = KnownFunctionalGroups::get_functional_group("OH");
        if (uncontains_p(current_fa->functional_groups, "OH")) current_fa->functional_groups->insert({"OH", vector<FunctionalGroup*>()});
        current_fa->functional_groups->at("OH").push_back(functional_group);
    }
}
    

void HmdbParserEventHandler::add_double_bonds(TreeNode *node) {
    current_fa->double_bonds->num_double_bonds = atoi(node->get_text().c_str());
}
    
    

void HmdbParserEventHandler::add_carbon(TreeNode *node) {
    current_fa->num_carbon = atoi(node->get_text().c_str());
}
    

void HmdbParserEventHandler::furan_fa(TreeNode *node) {
    throw UnsupportedLipidException("Furan fatty acyl chains are currently not supported");
}
    

void HmdbParserEventHandler::interlink_fa(TreeNode *node) {
    throw UnsupportedLipidException("Interconnected fatty acyl chains are currently not supported");
}
    

void HmdbParserEventHandler::lipid_suffix(TreeNode *node) {
    throw UnsupportedLipidException("Lipids with suffix '" + node->get_text() + "' are currently not supported");
}

        

        
