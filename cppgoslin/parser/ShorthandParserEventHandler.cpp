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


#include "cppgoslin/parser/ShorthandParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&ShorthandParserEventHandler::y, this, placeholders::_1)})
    

ShorthandParserEventHandler::ShorthandParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    
    // set categories
    reg("sl_pre_event", pre_sphingolipid);
    reg("sl_post_event", post_sphingolipid);
    reg("sl_hydroxyl_pre_event", set_hydroxyl);
    
    // set adduct events
    reg("adduct_info_pre_event", new_adduct);
    reg("adduct_pre_event", add_adduct);
    reg("charge_pre_event", add_charge);
    reg("charge_sign_pre_event", add_charge_sign);
    
    // set species events
    reg("med_species_pre_event", set_species_level);
    reg("gl_species_pre_event", set_species_level);
    reg("gl_molecular_species_pre_event", set_molecular_level);
    reg("pl_species_pre_event", set_species_level);
    reg("pl_molecular_species_double_pre_event", set_molecular_level);
    reg("pl_molecular_species_triple_pre_event", set_molecular_level);
    reg("sl_species_pre_event", set_species_level);
    reg("pl_single_pre_event", set_molecular_level);
    reg("unsorted_fa_separator_pre_event", set_molecular_level);
    reg("ether_num_pre_event", set_ether_num);
    
    // set head groups events
    reg("med_hg_single_pre_event", set_headgroup_name);
    reg("med_hg_double_pre_event", set_headgroup_name);
    reg("med_hg_triple_pre_event", set_headgroup_name);
    reg("gl_hg_single_pre_event", set_headgroup_name);
    reg("gl_hg_double_pre_event", set_headgroup_name);
    reg("gl_hg_true_double_pre_event", set_headgroup_name);
    reg("gl_hg_triple_pre_event", set_headgroup_name);
    reg("pl_hg_single_pre_event", set_headgroup_name);
    reg("pl_hg_double_pre_event", set_headgroup_name);
    reg("pl_hg_quadro_pre_event", set_headgroup_name);
    reg("sl_hg_single_pre_event", set_headgroup_name);
    reg("pl_hg_double_fa_hg_pre_event", set_headgroup_name);
    reg("sl_hg_double_name_pre_event", set_headgroup_name);
    reg("st_hg_pre_event", set_headgroup_name);
    reg("st_hg_ester_pre_event", set_headgroup_name);
    reg("hg_pip_pure_m_pre_event", set_headgroup_name);
    reg("hg_pip_pure_d_pre_event", set_headgroup_name);
    reg("hg_pip_pure_t_pre_event", set_headgroup_name);
    reg("hg_PE_PS_pre_event", set_headgroup_name);

    // set head group headgroup_decorators
    reg("carbohydrate_pre_event", set_carbohydrate);
    reg("carbohydrate_structural_pre_event", set_carbohydrate_structural);
    reg("carbohydrate_isomeric_pre_event", set_carbohydrate_isomeric);
    
    // fatty acyl events
    reg("lcb_post_event", set_lcb);
    reg("fatty_acyl_chain_pre_event", new_fatty_acyl_chain);
    reg("fatty_acyl_chain_post_event", add_fatty_acyl_chain);
    reg("carbon_pre_event", set_carbon);
    reg("db_count_pre_event", set_double_bond_count);
    reg("db_position_number_pre_event", set_double_bond_position);
    reg("db_single_position_pre_event", set_double_bond_information);
    reg("db_single_position_post_event", add_double_bond_information);
    reg("cistrans_pre_event", set_cistrans);
    reg("ether_type_pre_event", set_ether_type);
    
    // set functional group events
    reg("func_group_data_pre_event", set_functional_group);
    reg("func_group_data_post_event", add_functional_group);
    reg("func_group_pos_number_pre_event", set_functional_group_position);
    reg("func_group_name_pre_event", set_functional_group_name);
    reg("func_group_count_pre_event", set_functional_group_count);
    reg("stereo_type_pre_event", set_functional_group_stereo);
    reg("molecular_func_group_name_pre_event", set_molecular_func_group);
    
    // set cycle events
    reg("func_group_cycle_pre_event", set_cycle);
    reg("func_group_cycle_post_event", add_cycle);
    reg("cycle_start_pre_event", set_cycle_start);
    reg("cycle_end_pre_event", set_cycle_end);
    reg("cycle_number_pre_event", set_cycle_number);
    reg("cycle_db_cnt_pre_event", set_cycle_db_count);
    reg("cycle_db_positions_pre_event", set_cycle_db_positions);
    reg("cycle_db_positions_post_event", check_cycle_db_positions);
    reg("cycle_db_position_number_pre_event", set_cycle_db_position);
    reg("cycle_db_position_cis_trans_pre_event", set_cycle_db_position_cistrans);
    reg("cylce_element_pre_event", add_cycle_element);
    
    // set linkage events
    reg("fatty_acyl_linkage_pre_event", set_acyl_linkage);
    reg("fatty_acyl_linkage_post_event", add_acyl_linkage);
    reg("fatty_alkyl_linkage_pre_event", set_alkyl_linkage);
    reg("fatty_alkyl_linkage_post_event", add_alkyl_linkage);
    reg("fatty_linkage_number_pre_event", set_fatty_linkage_number);
    reg("fatty_acyl_linkage_sign_pre_event", set_linkage_type);
    reg("hydrocarbon_chain_pre_event", set_hydrocarbon_chain);
    reg("hydrocarbon_chain_post_event", add_hydrocarbon_chain);
    reg("hydrocarbon_number_pre_event", set_fatty_linkage_number);
    
    // set remaining events
    reg("ring_stereo_pre_event", set_ring_stereo);
    reg("pl_hg_fa_pre_event", set_hg_acyl);
    reg("pl_hg_fa_post_event", add_hg_acyl);
    reg("pl_hg_alk_pre_event", set_hg_alkyl);
    reg("pl_hg_alk_post_event", add_hg_alkyl);
    reg("pl_hg_species_pre_event", add_pl_species_data);
    reg("hg_pip_m_pre_event", suffix_decorator_molecular);
    reg("hg_pip_d_pre_event", suffix_decorator_molecular);
    reg("hg_pip_t_pre_event", suffix_decorator_molecular);
    reg("hg_PE_PS_type_pre_event", suffix_decorator_species);
    
    
    debug = "";
}


ShorthandParserEventHandler::~ShorthandParserEventHandler(){
}



void ShorthandParserEventHandler::reset_lipid(TreeNode *node) {
    level = STRUCTURAL_SUBSPECIES;
    lipid = NULL;
    adduct = NULL;
    headgroup = "";
    fa_list.clear();
    current_fa.clear();
    headgroup_decorators.clear();
    tmp.remove_all();
    
}



void ShorthandParserEventHandler::build_lipid(TreeNode *node) {
    
    
    lipid = NULL;
    LipidSpecies *ls = NULL;
    
    Headgroup* head_group = new Headgroup(headgroup);
    switch (level){
        case SPECIES: ls = new LipidSpecies(head_group, &fa_list); break;
        case MOLECULAR_SUBSPECIES: ls = new LipidMolecularSubspecies(head_group, &fa_list); break;
        case STRUCTURAL_SUBSPECIES: ls = new LipidStructuralSubspecies(head_group, &fa_list); break;
        case ISOMERIC_SUBSPECIES: ls = new LipidIsomericSubspecies(head_group, &fa_list); break;
        default: break;
    }
    lipid = new LipidAdduct();
    lipid->lipid = ls;
    lipid->adduct = adduct;
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
    
}

void ShorthandParserEventHandler::set_lipid_level(LipidLevel level){

}



void ShorthandParserEventHandler::add_cycle_element(TreeNode *node){

}



void ShorthandParserEventHandler::set_headgroup_name(TreeNode *node){

}



void ShorthandParserEventHandler::set_carbohydrate(TreeNode *node){

}



void ShorthandParserEventHandler::set_carbohydrate_structural(TreeNode *node){

}



void ShorthandParserEventHandler::set_carbohydrate_isomeric(TreeNode *node){

}



void ShorthandParserEventHandler::suffix_decorator_molecular(TreeNode *node){

}



void ShorthandParserEventHandler::suffix_decorator_species(TreeNode *node){

}



void ShorthandParserEventHandler::set_pl_hg_triple(TreeNode *node){

}



void ShorthandParserEventHandler::pre_sphingolipid(TreeNode *node){

}



void ShorthandParserEventHandler::set_ring_stereo(TreeNode *node){

}



void ShorthandParserEventHandler::post_sphingolipid(TreeNode *node){

}



void ShorthandParserEventHandler::set_hydroxyl(TreeNode *node){

}



void ShorthandParserEventHandler::set_lcb(TreeNode *node){

}



void ShorthandParserEventHandler::add_pl_species_data(TreeNode *node){

}



void ShorthandParserEventHandler::new_fatty_acyl_chain(TreeNode *node){

}



void ShorthandParserEventHandler::add_fatty_acyl_chain(TreeNode *node){

}



void ShorthandParserEventHandler::set_double_bond_position(TreeNode *node){

}



void ShorthandParserEventHandler::set_double_bond_information(TreeNode *node){

}



void ShorthandParserEventHandler::add_double_bond_information(TreeNode *node){

}



void ShorthandParserEventHandler::set_cistrans(TreeNode *node){

}



void ShorthandParserEventHandler::set_functional_group(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle(TreeNode *node){

}



void ShorthandParserEventHandler::add_cycle(TreeNode *node){

}



void ShorthandParserEventHandler::set_fatty_linkage_number(TreeNode *node){

}



void ShorthandParserEventHandler::set_hg_acyl(TreeNode *node){

}



void ShorthandParserEventHandler::add_hg_acyl(TreeNode *node){

}



void ShorthandParserEventHandler::set_hg_alkyl(TreeNode *node){

}



void ShorthandParserEventHandler::add_hg_alkyl(TreeNode *node){

}



void ShorthandParserEventHandler::set_linkage_type(TreeNode *node){

}



void ShorthandParserEventHandler::set_hydrocarbon_chain(TreeNode *node){

}



void ShorthandParserEventHandler::add_hydrocarbon_chain(TreeNode *node){

}



void ShorthandParserEventHandler::set_acyl_linkage(TreeNode *node){

}



void ShorthandParserEventHandler::add_acyl_linkage(TreeNode *node){

}



void ShorthandParserEventHandler::set_alkyl_linkage(TreeNode *node){

}



void ShorthandParserEventHandler::add_alkyl_linkage(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_start(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_end(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_number(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_db_count(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_db_positions(TreeNode *node){

}



void ShorthandParserEventHandler::check_cycle_db_positions(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_db_position(TreeNode *node){

}



void ShorthandParserEventHandler::set_cycle_db_position_cistrans(TreeNode *node){

}



void ShorthandParserEventHandler::set_functional_group_position(TreeNode *node){

}



void ShorthandParserEventHandler::set_functional_group_name(TreeNode *node){

}



void ShorthandParserEventHandler::set_functional_group_count(TreeNode *node){

}



void ShorthandParserEventHandler::set_functional_group_stereo(TreeNode *node){

}



void ShorthandParserEventHandler::set_molecular_func_group(TreeNode *node){

}



void ShorthandParserEventHandler::add_functional_group(TreeNode *node){

}



void ShorthandParserEventHandler::set_ether_type(TreeNode *node){

}



void ShorthandParserEventHandler::set_ether_num(TreeNode *node){

}



void ShorthandParserEventHandler::set_species_level(TreeNode *node){

}



void ShorthandParserEventHandler::set_molecular_level(TreeNode *node){

}



void ShorthandParserEventHandler::set_carbon(TreeNode *node){

}



void ShorthandParserEventHandler::set_double_bond_count(TreeNode *node){

}



void ShorthandParserEventHandler::new_adduct(TreeNode *node){

}



void ShorthandParserEventHandler::add_adduct(TreeNode *node){

}



void ShorthandParserEventHandler::add_charge(TreeNode *node){

}



void ShorthandParserEventHandler::add_charge_sign(TreeNode *node){

}



    
