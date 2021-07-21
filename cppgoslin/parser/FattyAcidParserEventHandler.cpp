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


#include "cppgoslin/parser/FattyAcidParserEventHandler.h"

#define reg(x, y) BaseParserEventHandler<LipidAdduct*>::registered_events->insert({x, bind(&FattyAcidParserEventHandler::y, this, placeholders::_1)})
#define FA_I ("fa" + std::to_string(current_fa.size()))

FattyAcidParserEventHandler::FattyAcidParserEventHandler() : BaseParserEventHandler<LipidAdduct*>() {
    
        
    reg("lipid_pre_event", reset_lipid);
    reg("lipid_post_event", build_lipid);
    reg("fatty_acid_post_event", set_fatty_acid);
    
    reg("acid_single_type_pre_event", set_fatty_acyl_type);
    reg("ol_ending_pre_event", set_fatty_acyl_type);
    reg("double_bond_position_pre_event", set_double_bond_information);
    reg("double_bond_position_post_event", add_double_bond_information);
    reg("db_number_post_event", set_double_bond_position);
    reg("cistrans_post_event", set_cistrans);
    reg("acid_type_double_post_event", check_db);
    
    // lengths
    reg("functional_length_pre_event", reset_length);
    reg("fatty_length_pre_event", reset_length);
    reg("functional_length_post_event", set_functional_length);
    reg("fatty_length_post_event", set_fatty_length);
    
    // numbers
    reg("notation_specials_pre_event", special_number);
    reg("notation_last_digit_pre_event", last_number);
    reg("notation_second_digit_pre_event", second_number);
    
    // functional groups
    reg("functional_group_pre_event", set_functional_group);
    reg("functional_group_post_event", add_functional_group);
    reg("functional_pos_pre_event", set_functional_pos);
    reg("functional_position_pre_event", set_functional_position);
    reg("functional_group_type_pre_event", set_functional_type);
    
    // cyclo / epoxy
    reg("cyclo_position_pre_event", set_functional_group);
    reg("cyclo_position_post_event", rearrange_cycle);
    reg("epoxy_pre_event", set_functional_group);
    reg("epoxy_post_event", add_epoxy);
    reg("cycle_pre_event", set_cycle);
    reg("methylene_post_event", set_methylene);

    // dioic
    reg("dioic_pre_event", set_functional_group);
    reg("dioic_post_event", set_dioic);
    reg("dioic_acid_pre_event", set_fatty_acyl_type);
    reg("dial_post_event", set_dial);

    
    // prosta
    reg("prosta_pre_event", set_prosta);
    reg("prosta_post_event", add_cyclo);
    reg("reduction_pre_event", set_functional_group);
    reg("reduction_post_event", reduction);
    reg("homo_post_event", homo);

    
    // recursion
    reg("recursion_description_pre_event", set_recursion);
    reg("recursion_description_post_event", add_recursion);
    reg("recursion_pos_pre_event", set_recursion_pos);
    reg("yl_ending_pre_event", set_yl_ending);
    reg("acetic_acid_post_event", set_acetic_acid);
    reg("acetic_recursion_pre_event", set_recursion);
    reg("acetic_recursion_post_event", add_recursion);
    reg("hydroxyl_number_pre_event", add_hydroxyl);
    reg("ol_pre_event", setup_hydroxyl);
    reg("ol_post_event", add_hydroxyls);
    reg("ol_pos_post_event", set_yl_ending);
    
    
    // wax esters
    reg("wax_ester_pre_event", set_recursion);
    reg("wax_ester_post_event", add_wax_ester);
    reg("ate_post_event", set_ate);
    reg("isoprop_post_event", set_iso);
    reg("isobut_post_event", set_iso);
    
    // CoA
    reg("CoA_post_event", set_coa);
    reg("methyl_pre_event", set_methyl);
    
    // CAR
    reg("CAR_pre_event", set_car);
    reg("CAR_post_event", add_car);
    
    // amine
    reg("ethanolamine_post_event", add_ethanolamine);
    reg("amine_n_pre_event", set_recursion);
    reg("amine_n_post_event", add_amine);
    reg("amine_post_event", add_amine_name);
    
    // functional group position summary
    reg("fg_pos_summary_pre_event", set_functional_group);
    reg("fg_pos_summary_post_event", add_summary);
    reg("func_stereo_pre_event", add_func_stereo);
}


void reset_lipid(TreeNode *node) {
}


void build_lipid(TreeNode *node) {
}


void set_fatty_acid(TreeNode *node) {
}


void set_fatty_acyl_type(TreeNode *node) {
}


void set_double_bond_information(TreeNode *node) {
}


void add_double_bond_information(TreeNode *node) {
}


void set_double_bond_position(TreeNode *node) {
}


void set_cistrans(TreeNode *node) {
}


void check_db(TreeNode *node) {
}


void reset_length(TreeNode *node) {
}


void set_functional_length(TreeNode *node) {
}


void set_fatty_length(TreeNode *node) {
}


void special_number(TreeNode *node) {
}


void last_number(TreeNode *node) {
}


void second_number(TreeNode *node) {
}


void set_functional_group(TreeNode *node) {
}


void add_functional_group(TreeNode *node) {
}


void set_functional_pos(TreeNode *node) {
}


void set_functional_position(TreeNode *node) {
}


void set_functional_type(TreeNode *node) {
}


void rearrange_cycle(TreeNode *node) {
}


void add_epoxy(TreeNode *node) {
}


void set_cycle(TreeNode *node) {
}


void set_methylene(TreeNode *node) {
}


void set_dioic(TreeNode *node) {
}


void set_dial(TreeNode *node) {
}


void set_prosta(TreeNode *node) {
}


void add_cyclo(TreeNode *node) {
}


void reduction(TreeNode *node) {
}


void homo(TreeNode *node) {
}


void set_recursion(TreeNode *node) {
}


void add_recursion(TreeNode *node) {
}


void set_recursion_pos(TreeNode *node) {
}


void set_yl_ending(TreeNode *node) {
}


void set_acetic_acid(TreeNode *node) {
}


void add_hydroxyl(TreeNode *node) {
}


void setup_hydroxyl(TreeNode *node) {
}


void add_hydroxyls(TreeNode *node) {
}


void add_wax_ester(TreeNode *node) {
}


void set_ate(TreeNode *node) {
}


void set_iso(TreeNode *node) {
}


void set_coa(TreeNode *node) {
}


void set_methyl(TreeNode *node) {
}


void set_car(TreeNode *node) {
}


void add_car(TreeNode *node) {
}


void add_ethanolamine(TreeNode *node) {
}


void add_amine(TreeNode *node) {
}


void add_amine_name(TreeNode *node) {
}


void add_summary(TreeNode *node) {
}


void add_func_stereo(TreeNode *node) {
}
