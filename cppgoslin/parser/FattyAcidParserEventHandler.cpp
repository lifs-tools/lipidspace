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



const map<string, int> FattyAcidParserEventHandler::last_numbers{{"un", 1}, {"hen", 1}, {"do", 2}, {"di", 2}, {"tri", 3}, {"buta", 4}, {"but", 4}, {"tetra", 4}, {"penta", 5}, {"pent", 5}, {"hexa", 6}, {"hex", 6}, {"hepta", 7}, {"hept", 7}, {"octa", 8}, {"oct", 8}, {"nona", 9}, {"non", 9}};


const map<string, int> FattyAcidParserEventHandler::second_numbers {{"deca", 10}, {"dec", 10}, {"eicosa", 20}, {"eicos", 20 }, {"cosa", 20}, {"cos", 20}, {"triaconta", 30}, {"triacont", 30}, {"tetraconta", 40}, {"tetracont", 40}, {"pentaconta", 50}, {"pentacont", 50}, {"hexaconta", 60}, {"hexacont", 60}, {"heptaconta", 70}, {"heptacont", 70}, {"octaconta", 80}, {"octacont", 80}, {"nonaconta", 90}, {"nonacont", 90}};

const map<string, string> FattyAcidParserEventHandler::func_groups {{"keto", "oxo"}, {"ethyl", "Et"}, {"hydroxy", "OH"}, {"phospho", "Ph"}, {"oxo", "oxo"}, {"bromo", "Br"}, {"methyl", "Me"}, {"hydroperoxy", "OOH"}, {"homo", ""}, {"Epoxy", "Ep"}, {"fluro", "F"}, {"fluoro", "F"}, {"chloro", "Cl"}, {"methylene", "My"}, {"sulfooxy", "Su"}, {"amino", "NH2"}, {"sulfanyl", "SH"}, {"methoxy", "OMe"}, {"iodo", "I"}, {"cyano", "CN"}, {"nitro", "NO2"}, {"OH", "OH"}, {"thio", "SH"}, {"mercapto", "SH"}, {"carboxy", "COOH"}, {"acetoxy", "Ac"}, {"cysteinyl", "Cys"}, {"phenyl", "Phe"}, {"s-glutathionyl", "SGlu"}, {"s-cysteinyl", "SCys"}, {"butylperoxy", "BOO"}, {"dimethylarsinoyl", "MMAs"}, {"methylsulfanyl", "SMe"}, {"imino", "NH"}, {"s-cysteinylglycinyl", "SCG"}};

const map<string, int> FattyAcidParserEventHandler::ate {{"formate", 1}, {"acetate", 2}, {"butyrate", 4}, {"propionate", 3}, {"valerate", 5}, {"isobutyrate", 4}};

const map<string, int> FattyAcidParserEventHandler::special_numbers {{"meth", 1}, {"etha", 2}, {"eth", 2}, {"propa", 3}, {"isoprop", 3}, {"prop", 3}, {"propi", 3}, {"propio", 3}, {"buta", 4}, {"but", 4}, {"butr", 4}, {"valer", 5}, {"eicosa", 20}, {"eicos", 20}, {"icosa", 20}, {"icos", 20}, {"prosta", 20}, {"prost", 20}, {"prostan", 20}};


FattyAcidParserEventHandler::~FattyAcidParserEventHandler(){
}


void FattyAcidParserEventHandler::set_lipid_level(LipidLevel _level){
    level = min(level, _level);
}


void FattyAcidParserEventHandler::reset_lipid(TreeNode *node) {
    level = ISOMERIC_SUBSPECIES;
    headgroup = "";
    fatty_acyl_stack.clear();
    fatty_acyl_stack.push_back(new FattyAcid("FA"));
    tmp.remove_all();
    debug = "";
}



void FattyAcidParserEventHandler::add_position(FunctionalGroup* func_group, int pos){
    func_group->position += func_group->position >= pos;
    try {
        ((Cycle*)func_group)->start += ((Cycle*)func_group)->start >= pos;
        ((Cycle*)func_group)->end += ((Cycle*)func_group)->end >= pos;
    }
    catch (...){}
    
    for (auto &kv : *(func_group->functional_groups)){
        for (auto &fg : kv.second){
            add_position(fg, pos);
        }
    }
}


void FattyAcidParserEventHandler::build_lipid(TreeNode *node) {
    
    if (tmp.contains_key("cyclo_yl")) {
        tmp.set_list("fg_pos", new GenericList());
        tmp.get_list("fg_pos")->add_list(new GenericList());
        tmp.get_list("fg_pos")->get_list(0)->add_int(1);
        tmp.get_list("fg_pos")->get_list(0)->add_string("");
        tmp.get_list("fg_pos")->add_list(new GenericList());
        tmp.get_list("fg_pos")->get_list(1)->add_int(tmp.get_int("cyclo_len"));
        tmp.get_list("fg_pos")->get_list(1)->add_string("");
        add_cyclo(node);
        tmp.remove("cyclo_yl");
        tmp.remove("cyclo_len");
    }
            
    
    if (tmp.contains_key("post_adding")){
        FattyAcid *curr_fa = fatty_acyl_stack.back();
        int s = tmp.get_list("post_adding")->list.size();
        curr_fa->num_carbon += s;
        for (int i = 0; i < s; ++i){
            int pos = tmp.get_list("post_adding")->get_int(i);
            add_position(curr_fa, pos);
            DoubleBonds* db = new DoubleBonds(curr_fa->double_bonds->num_double_bonds);
            for (auto &kv : curr_fa->double_bonds->double_bond_positions){
                db->double_bond_positions.insert({kv.first + (kv.first >= pos), kv.second});
            }
            delete curr_fa->double_bonds;
            curr_fa->double_bonds = db;
        }
    }
    
    
    FattyAcid *curr_fa = fatty_acyl_stack.back();
    if (!curr_fa->double_bonds->double_bond_positions.empty()){
        int db_right = 0;
        for (auto &kv : curr_fa->double_bonds->double_bond_positions) db_right += kv.second.length() > 0;
        if (db_right != (int)curr_fa->double_bonds->double_bond_positions.size()){
            set_lipid_level(STRUCTURAL_SUBSPECIES);
        }
    }
    
    
    Headgroup *head_group = new Headgroup(headgroup);
    
    lipid = new LipidAdduct();
    
    switch(level){
        case ISOMERIC_SUBSPECIES:
            lipid->lipid = new LipidIsomericSubspecies(head_group, &fatty_acyl_stack);
            break;
            
        case STRUCTURAL_SUBSPECIES:
            lipid->lipid = new LipidStructuralSubspecies(head_group, &fatty_acyl_stack);
            break;
            
        case MOLECULAR_SUBSPECIES:
            lipid->lipid = new LipidMolecularSubspecies(head_group, &fatty_acyl_stack);
            break;
            
        case SPECIES:
            lipid->lipid = new LipidSpecies(head_group, &fatty_acyl_stack);
            break;
            
        default:
            break;
    }
    BaseParserEventHandler<LipidAdduct*>::content = lipid;
}


void FattyAcidParserEventHandler::switch_position(FunctionalGroup* func_group, int switch_num){
    func_group->position = switch_num - func_group->position;
    for (auto &kv : *(func_group->functional_groups)){
        for (auto &fg : kv.second){
            switch_position(fg, switch_num);
        }
    }
}


void FattyAcidParserEventHandler::set_fatty_acid(TreeNode *node) {
    FattyAcid* curr_fa = fatty_acyl_stack.back();
    
    if (contains_p(curr_fa->functional_groups, "noyloxy")){
        if (headgroup == "FA") headgroup = "FAHFA";
        
        while (!curr_fa->functional_groups->at("noyloxy").empty()){
            FattyAcid* fa = (FattyAcid*)curr_fa->functional_groups->at("noyloxy").back();
            curr_fa->functional_groups->at("noyloxy").pop_back();
        
            AcylAlkylGroup* acyl = new AcylAlkylGroup(fa);
            acyl->position = fa->position;
            
            if (uncontains_p(curr_fa->functional_groups, "acyl")) curr_fa->functional_groups->insert({"acyl", vector<FunctionalGroup*>()});
            curr_fa->functional_groups->at("acyl").push_back(acyl);
        }
        curr_fa->functional_groups->erase("noyloxy");
    }
        
    else if (contains_p(curr_fa->functional_groups, "nyloxy")){
        while (!curr_fa->functional_groups->at("nyloxy").empty()){
            FattyAcid* fa = (FattyAcid*)curr_fa->functional_groups->at("nyloxy").back();
            curr_fa->functional_groups->at("nyloxy").pop_back();        
            
            AcylAlkylGroup* alkyl = new AcylAlkylGroup(fa, -1, 1, true);
            alkyl->position = fa->position;
            
            if (uncontains_p(curr_fa->functional_groups, "alkyl")) curr_fa->functional_groups->insert({"alkyl", vector<FunctionalGroup*>()});
            curr_fa->functional_groups->at("alkyl").push_back(alkyl);
        }
        curr_fa->functional_groups->erase("nyloxy");
    }
            
    else {
        bool has_yl = false;
        for (auto &kv : *(curr_fa->functional_groups)){
            if (endswith(kv.first, "yl")){
                has_yl = true;
                break;
            }
        }
        if (has_yl){
            while (true){
                string yl = "";
                for (auto &kv : *(curr_fa->functional_groups)){
                    if (endswith(kv.first, "yl")){
                        yl = kv.first;
                        break;
                    }
                }
                if (yl.length() == 0) {
                    break;
                }
            
                while (!curr_fa->functional_groups->at(yl).empty()){
                    FattyAcid* fa = (FattyAcid*)curr_fa->functional_groups->at(yl).back();
                    curr_fa->functional_groups->at(yl).pop_back();
                    
                    if (tmp.contains_key("cyclo")){
                        int cyclo_len = curr_fa->num_carbon;
                        tmp.set_int("cyclo_len", cyclo_len);
                        switch_position(curr_fa, 2 + cyclo_len);
                        DoubleBonds *db = new DoubleBonds(curr_fa->double_bonds->num_double_bonds);
                        for (auto &kv : curr_fa->double_bonds->double_bond_positions) db->double_bond_positions.insert({2 + cyclo_len - kv.first, kv.second});
                        delete curr_fa->double_bonds;
                        curr_fa->double_bonds = db;
                        fa->shift_positions(cyclo_len);
                        
                        for (auto &kv : *(fa->functional_groups)){
                            if (uncontains_p(curr_fa->functional_groups, kv.first)) {
                                curr_fa->functional_groups->insert({kv.first, vector<FunctionalGroup*>()});
                            }
                            for (auto &func_group : kv.second) curr_fa->functional_groups->at(kv.first).push_back(func_group);
                        }
                            
                        curr_fa->num_carbon = cyclo_len + fa->num_carbon;
                        
                        for (auto &kv : fa->double_bonds->double_bond_positions){
                            curr_fa->double_bonds->double_bond_positions.insert({kv.first + cyclo_len, kv.second});
                        }
                        curr_fa->double_bonds->num_double_bonds = curr_fa->double_bonds->double_bond_positions.size();
                        tmp.set_int("cyclo_yl", 1);
                    }
                    else {
                        // add carbon chains here here
                        // special chains: i.e. ethyl, methyl
                        string fg_name = "";
                        if (fa->double_bonds->get_num() == 0 && fa->functional_groups->empty()){
                            FunctionalGroup *fg = 0;
                            if (fa->num_carbon == 1){
                                fg_name = "Me";
                                fg = KnownFunctionalGroups::get_functional_group(fg_name);
                            }
                            else if (fa->num_carbon == 2){
                                fg_name = "Et";
                                fg = KnownFunctionalGroups::get_functional_group(fg_name);
                            }
                            if (fg_name.length() > 0){
                                fg->position = fa->position;
                                if (uncontains_p(curr_fa->functional_groups, fg_name)) curr_fa->functional_groups->insert({fg_name, vector<FunctionalGroup*>()});
                                curr_fa->functional_groups->at(fg_name).push_back(fg);
                            }
                        }
                        if (fg_name.length() == 0){
                            CarbonChain *cc = new CarbonChain(fa, fa->position);
                            if (uncontains_p(curr_fa->functional_groups, "cc")) curr_fa->functional_groups->insert({"cc", vector<FunctionalGroup*>()});
                            curr_fa->functional_groups->at("cc").push_back(cc);
                        }
                    }
                }
                if (tmp.contains_key("cyclo")) tmp.remove("cyclo");
                curr_fa->functional_groups->erase(yl);
            }
        }
    }
        
    if (contains_p(curr_fa->functional_groups, "cyclo")){
        FattyAcid *fa = (FattyAcid*)curr_fa->functional_groups->at("cyclo").front();
        curr_fa->functional_groups->erase("cyclo");
        int start_pos = curr_fa->num_carbon + 1;
        int end_pos = curr_fa->num_carbon + (tmp.contains_key("cyclo_len") ? tmp.get_int("cyclo_len") : 5);
        fa->shift_positions(start_pos - 1);
        
        if (contains_p(curr_fa->functional_groups, "cy")){
            for (auto &cy : curr_fa->functional_groups->at("cy")){
                cy->shift_positions(start_pos - 1);
            }
        }
        for (auto &kv : *(fa->functional_groups)){
            if (uncontains_p(curr_fa->functional_groups, kv.first)){
                curr_fa->functional_groups->insert({kv.first, vector<FunctionalGroup*>()});
            }
            for (auto &func_group : kv.second){
                curr_fa->functional_groups->at(kv.first).push_back(func_group);
            }
        }
        
        curr_fa->num_carbon += fa->num_carbon;
        
        DoubleBonds *db = new DoubleBonds(fa->double_bonds->num_double_bonds);
        for (auto &kv : fa->double_bonds->double_bond_positions) db->double_bond_positions.insert({kv.first + start_pos - 1, kv.second});
        delete curr_fa->double_bonds;
        curr_fa->double_bonds = db;
                
        tmp.set_list("fg_pos", new GenericList());
        tmp.get_list("fg_pos")->add_list(new GenericList());
        tmp.get_list("fg_pos")->add_list(new GenericList());
        tmp.get_list("fg_pos")->get_list(0)->add_int(start_pos);
        tmp.get_list("fg_pos")->get_list(0)->add_string("");
        tmp.get_list("fg_pos")->get_list(1)->add_int(end_pos);
        tmp.get_list("fg_pos")->get_list(1)->add_string("");
        
        add_cyclo(node);
        
        if (tmp.contains_key("cyclo_len")) tmp.remove("cyclo_len");
        if (tmp.contains_key("cyclo")) tmp.remove("cyclo");
    }
        
    else if (tmp.contains_key("cyclo")){
        tmp.set_list("fg_pos", new GenericList());
        tmp.get_list("fg_pos")->add_list(new GenericList());
        tmp.get_list("fg_pos")->add_list(new GenericList());
        tmp.get_list("fg_pos")->get_list(0)->add_int(1);
        tmp.get_list("fg_pos")->get_list(0)->add_string("");
        tmp.get_list("fg_pos")->get_list(1)->add_int(curr_fa->num_carbon);
        tmp.get_list("fg_pos")->get_list(1)->add_string("");
        
        add_cyclo(node);
        tmp.remove("cyclo");
    }
}


void FattyAcidParserEventHandler::set_fatty_acyl_type(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_double_bond_information(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_double_bond_information(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_double_bond_position(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_cistrans(TreeNode *node) {
}


void FattyAcidParserEventHandler::check_db(TreeNode *node) {
}


void FattyAcidParserEventHandler::reset_length(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_functional_length(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_fatty_length(TreeNode *node) {
}


void FattyAcidParserEventHandler::special_number(TreeNode *node) {
}


void FattyAcidParserEventHandler::last_number(TreeNode *node) {
}


void FattyAcidParserEventHandler::second_number(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_functional_group(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_functional_group(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_functional_pos(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_functional_position(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_functional_type(TreeNode *node) {
}


void FattyAcidParserEventHandler::rearrange_cycle(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_epoxy(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_cycle(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_methylene(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_dioic(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_dial(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_prosta(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_cyclo(TreeNode *node) {
}


void FattyAcidParserEventHandler::reduction(TreeNode *node) {
}


void FattyAcidParserEventHandler::homo(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_recursion(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_recursion(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_recursion_pos(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_yl_ending(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_acetic_acid(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_hydroxyl(TreeNode *node) {
}


void FattyAcidParserEventHandler::setup_hydroxyl(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_hydroxyls(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_wax_ester(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_ate(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_iso(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_coa(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_methyl(TreeNode *node) {
}


void FattyAcidParserEventHandler::set_car(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_car(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_ethanolamine(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_amine(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_amine_name(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_summary(TreeNode *node) {
}


void FattyAcidParserEventHandler::add_func_stereo(TreeNode *node) {
}
