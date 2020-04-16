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


#include "cppgoslin/cppgoslin.h"
#include <set>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <fstream>


using namespace std;
using namespace goslin;

int main(int argc, char** argv){
    char PARSER_QUOTE = '\'';
    
    
    LipidAdduct* lipid;
    string lipid_name;
    
    GoslinParser goslin_parser;
    LipidParser lipid_parser;
    LipidMapsParser lipid_maps_parser;
    GoslinFragmentParser goslin_fragment_parser;
    SwissLipidsParser swiss_lipids_parser;
    
    
    // test bitfield
    srand(time(0));
    int num_numbers = 1000 + (rand() % 1000);
    int max_number = num_numbers * 100;
    set<int> check;
    Bitfield b(max_number);
    
    for (int i = 0; i < num_numbers; ++i){
        int num = rand() % max_number;
        b.insert(num);
        check.insert(num);
    }
    
    uint32_t cnt = 0;
    for (int i : b){
        assert (check.find(i) != check.end());
        ++cnt;
    }
    assert (cnt == check.size());
    
    
    
    
    // check lipids maps parser for headless FA
    lipid_name = "15:6(2Z,4E,6Z,8E,12E,14)(6Me,8Me,10Me[S],13Me)";
    lipid = lipid_maps_parser.parse(lipid_name);
    assert (lipid_maps_parser.word_in_grammar);
    assert (lipid);
    delete lipid;
    
    
    
    // Pure Parser test
    GoslinParserEventHandler goslin_parser_event_handler;
    Parser<LipidAdduct*> goslin_parser_pure(&goslin_parser_event_handler, "data/goslin/Goslin.g4", PARSER_QUOTE);
    
    // check goslin pure parser with illegal lipid name
    string failLipid = "TAG 16::1-18:1-24:0";
    try {
        lipid = goslin_parser_pure.parse(failLipid);
        assert (false);
    }
    catch (LipidException &e){ }

    // glycerophospholipid
    for (auto the_lipid_name : { "TAG 16:1-18:1-24:0", "PE 16:1/12:0", "DAG 16:1-12:0", "12-HETE", "HexCer 18:1;2/16:0"}){
        LipidAdduct* lipid = goslin_parser_pure.parse(the_lipid_name);
        assert (lipid);
        delete lipid;
    }
    
    
    
    
    
    // check lipid maps and swiss lipids parser with illegal lipid name
    string failLipidSL = "TG(16::1_18:1_24:0)";
    try {
        lipid = lipid_maps_parser.parse(failLipidSL);
        assert (false);
    }
    catch (LipidException &e){ }
    try {
        lipid = swiss_lipids_parser.parse(failLipidSL);
        assert (false);
    }
    catch (LipidException &e){ }
    
    

    failLipidSL = "TG 16::1-18:1-24:0";
    // check goslin fragment parser with illegal lipid name 
    try {
        lipid = goslin_fragment_parser.parse(failLipidSL);
        assert (false);
    }
    catch (LipidException &e){ }
    
    
    // check if goslin fragment parser parses correctly lipid name with fragment
    lipid_name = "PE 16:1-12:0 - -(H20)";
    lipid = goslin_fragment_parser.parse(lipid_name);
    assert(lipid);
    assert (lipid->fragment);
    assert (lipid->fragment->name == "-(H20)");
    delete lipid;
    
    
    
    
    
    lipid_name = "PG(22:1(5Z)/12:0)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->lipid->info.level == ISOMERIC_SUBSPECIES);
    assert (lipid->get_lipid_string() == "PG 22:1(5Z)/12:0");
    delete lipid;
    
    lipid_name = "PG(22:1/12:0)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->lipid->info.level == STRUCTURAL_SUBSPECIES);
    assert (lipid->get_lipid_string() == "PG 22:1/12:0");
    delete lipid;
    
    lipid_name = "PG(22:1_12:0)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->lipid->info.level == MOLECULAR_SUBSPECIES);
    assert (lipid->get_lipid_string() == "PG 22:1-12:0");
    delete lipid;
    
    lipid_name = "LPG(O-22:1)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->lipid->info.level == SPECIES);
    assert (lipid->get_lipid_string() == "LPG 22:1a");
    delete lipid;
    
    
    
    lipid_name = "fail";
    try {
        lipid = lipid_parser.parse(lipid_name);
        assert (false);
    }
    catch(LipidException &e){ }
    
    
    
    lipid_name = "LP 19:1p";
    try {
        lipid = lipid_parser.parse(lipid_name);
        assert (false);
    }
    catch(LipidException &e){ }
    
    
    
    // test SwissLipids parser
    lipid_name = "TG(O-16:0/18:3(6Z,9Z,12Z)/18:1(11Z))";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "TAG 16:0a/18:3(6Z,9Z,12Z)/18:1(11Z)");
    delete lipid;
    
    lipid_name = "PIP2[4,5](21:0/24:4(9Z,12Z,15Z,18Z))";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PIP2[4',5'] 21:0/24:4(9Z,12Z,15Z,18Z)");
    delete lipid;
    
    lipid_name = "GalGb3Cer(d18:0/14:0)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "GalGb3Cer 18:0;2/14:0");
    delete lipid;
    
    lipid_name = "PI(34:5(19Z,22Z,25Z,28Z,31Z)/18:1(6Z))";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PI 34:5(19Z,22Z,25Z,28Z,31Z)/18:1(6Z)");
    delete lipid;
    
    lipid_name = "PIP(22:6/20:4)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PIP 22:6/20:4");
    delete lipid;
    
    lipid_name = "NAPE (12:0/30:4(15Z,18Z,21Z,24Z)/12:0)";
    lipid = swiss_lipids_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "NAPE 12:0/30:4(15Z,18Z,21Z,24Z)/12:0");
    delete lipid;
    
    /*
    vector<FattyAcid*>* fa_list = new vector<FattyAcid*>;
    LipidSpecies* ls = new LipidStructuralSubspecies("PA", fa_list);
    try {
        cout << ls->get_lipid_string() << endl;
    }
    catch (LipidException &e){
        cout << "Exception:" << endl;
        cout << e.what() << endl;
    }
    
    //delete ls;
    //delete fa_list;
    */
    
    // test lipid parser
    lipid_name = "PE 16:1-12:0";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PE 16:1-12:0");
    delete lipid;
    
    lipid_name = "PA 16:1-12:0 - fragment";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PA 16:1-12:0");
    assert (lipid->get_lipid_fragment_string() == "PA 16:1-12:0 - fragment");
    // check that all FAs have been initialized properly
    assert (lipid->lipid->get_fa_list().size() == 2);
    int faCnt = 1;
    for(FattyAcid* fa:lipid->lipid->get_fa_list()) {
        // cout << (*fa).name << " (lcb: " << ((*fa).lcb?"true":"false") << "): pos=" << (*fa).position << ", #carbon=" << (*fa).num_carbon << ", #hydroxyl=" << (*fa).num_hydroxyl << ", #double-bonds=" << (*fa).num_double_bonds << endl;
        assert ((*fa).position == 0);
        switch(faCnt) {
        case 1:
        assert ((*fa).lcb == false);
        assert ((*fa).num_carbon == 16);
        assert ((*fa).num_hydroxyl == 0);
        assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
        assert ((*fa).num_double_bonds == 1);
        break;
        case 2:
        assert ((*fa).lcb == false);
        assert ((*fa).num_carbon == 12);
        assert ((*fa).num_hydroxyl == 0);
        assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
        assert ((*fa).num_double_bonds == 0);
        break;
        }
        ++faCnt;
    }
    delete lipid;
    
    
    lipid_name = "PE O-16:1p/12:0";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PE O-16:1p/12:0");
    // check that all FAs have been initialized properly
    assert (lipid->lipid->get_fa_list().size() == 2);
    faCnt = 1;
    for(FattyAcid* fa:lipid->lipid->get_fa_list()) {
        // cout << (*fa).name << " (lcb: " << ((*fa).lcb?"true":"false") << "): pos=" << (*fa).position << ", #carbon=" << (*fa).num_carbon << ", #hydroxyl=" << (*fa).num_hydroxyl << ", #double-bonds=" << (*fa).num_double_bonds << endl;
        assert ((*fa).position == faCnt);
        switch(faCnt) {
        case 1:
        assert ((*fa).lcb == false);
        assert ((*fa).num_carbon == 16);
        assert ((*fa).num_hydroxyl == 0);
        assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "p");
        assert ((*fa).num_double_bonds == 1);
        break;
        case 2:
        assert ((*fa).lcb == false);
        assert ((*fa).num_carbon == 12);
        assert ((*fa).num_hydroxyl == 0);
        assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
        assert ((*fa).num_double_bonds == 0);
        break;
        }
        ++faCnt;
    }
    delete lipid;
    
    
    lipid_name = "PAT16 16:1/12:0/14:1/8:0";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PAT16 16:1/12:0/14:1/8:0");
    delete lipid;
    
    lipid_name = "SLBPA 16:1/12:0/14:1";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "SLBPA 16:1/12:0/14:1");
    delete lipid;
    
    lipid_name = "MLCL 16:1/12:0/14:1";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "MLCL 16:1/12:0/14:1");
    delete lipid;
    
    
    lipid_name = "DLCL 14:1/8:0";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "DLCL 14:1/8:0");
    delete lipid;
    
    
    lipid_name = "PIP[3'] 17:0/20:4(5Z,8Z,11Z,14Z)";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PIP[3'] 17:0/20:4(5Z,8Z,11Z,14Z)");
    delete lipid;
    
    lipid_name = "PIMIP 12:0/14:1";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "PIMIP 12:0/14:1");
    delete lipid;
    
    lipid_name = "LCDPDAG 14:1";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "LCDPDAG 14:1");
    delete lipid;
    
    
    lipid_name = "CPA 8:0";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "CPA 8:0");
    delete lipid;
    
    
    lipid_name = "LPIN 20:4(5Z,8Z,11Z,14Z)";
    lipid = lipid_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "LPIN 20:4(5Z,8Z,11Z,14Z)");
    delete lipid;
    
    
    
    
    // testing lipid maps parser
    vector< vector<string> > lmp_data{{"PA(16:1/12:0)", "PA 16:1/12:0"},
                        {"PA(4:0/12:0)", "PA 4:0/12:0"},
                        {"PC(O-14:0/0:0)", "LPC O-14:0a"},
                        {"SQMG(16:1(11Z)/0:0)", "SQMG 16:1(11Z)"},
                        {"TG(13:0/22:3(10Z,13Z,16Z)/22:5(7Z,10Z,13Z,16Z,19Z))[iso6]", "TAG 13:0/22:3(10Z,13Z,16Z)/22:5(7Z,10Z,13Z,16Z,19Z)"},
                        {"13R-HODE", "13R-HODE"},
                        {"CL(1'-[20:0/20:0],3'-[20:4(5Z,8Z,11Z,14Z)/18:2(9Z,12Z)])", "CL 20:0/20:0/20:4(5Z,8Z,11Z,14Z)/18:2(9Z,12Z)"},
                        {"PA(P-20:0/18:3(6Z,9Z,12Z))", "PA 20:0p/18:3(6Z,9Z,12Z)"},
                        {"M(IP)2C(t18:0/20:0(2OH))", "M(IP)2C 18:0;3/20:0;1"},
                        {"Cer(d16:2(4E,6E)/22:0(2OH))", "Cer 16:2(4E,6E);2/22:0;1"},
                        {"MG(18:1(11E)/0:0/0:0)[rac]", "MAG 18:1(11E)"},
                        {"PAT18(24:1(2E)(2Me,4Me[S],6Me[S])/25:1(2E)(2Me,4Me[S],6Me[S])/26:1(2E)(2Me,4Me[S],6Me[S])/24:1(2E)(2Me,4Me[S],6Me[S]))", "PAT18 24:1(2E)/25:1(2E)/26:1(2E)/24:1(2E)"},
                        {"(3'-sulfo)Galbeta-Cer(d18:1/20:0)", "SHexCer 18:1;2/20:0"},
                        {"GlcCer(d15:2(4E,6E)/22:0(2OH))", "HexCer 15:2(4E,6E);2/22:0;1"}};
    
    for (uint32_t i = 0; i < lmp_data.size(); ++i){
        lipid = lipid_maps_parser.parse(lmp_data.at(i)[0]);
        assert (lipid);
        assert (lipid->get_lipid_string() == lmp_data.at(i)[1]);
        delete lipid;
    }

    
    
    // testing lyso lipids
    lipid = lipid_parser.parse("LPA 16:1a");
    assert (lipid != NULL);
    assert (lipid->get_lipid_string() == "LPA 16:1a");
    delete lipid;
    
    lipid = lipid_parser.parse("LPC O-16:1a");
    assert (lipid != NULL);
    assert (lipid->get_lipid_string() == "LPC O-16:1a");
    delete lipid;
    
    lipid = lipid_parser.parse("LPE O-16:1p");
    assert (lipid != NULL);
    assert (lipid->get_lipid_string() == "LPE O-16:1p");
    delete lipid;
    
    lipid = lipid_parser.parse("LCB 18:1;2");
    assert (lipid != NULL);
    assert (lipid->get_lipid_string() == "LCB 18:1;2");
    delete lipid;
    
    
    try {
        lipid = lipid_parser.parse("LPE O-16:1p/12:0");
        assert(false);
    }
    catch (LipidException &e){}
    
    
    
    
    
    // testing some failing situations
    for (string test_lipid_name : {"PE 18:3:1-16:2", "CL 18:1/18:1-18:1-18:1", "MLL 18:1-18:1-18:1", "M(IP )2C 18:0;3/26:0;1", "PET 16:0-18:1", "CDPDAG 18:1-18:1-18:1", "BeMP 18:1-18:1", "PE O18:0a-22:6", "GB3 18:1;2-24:1;12", "DGDGDG 16:0-16:1", "DG  16:0-16:1", "LPC 14:1a", "LPA14:1a", "LCB 17:1;2/6:0", "LP 19:1p", "xx2-34", "Hex2Cer 18:1;2/12:0/2:0", "Cer 18:1;2\\16:0", "MAG 16:0-12:0", "PC 21:0+22:6", "PE 16:2-18:3;1 - GP(153e)", "PE 16:2-18:3;1 - FA 18:3;1(+HO)", "CL 14:1-16:1-18:1-20:1 - FA 18:1(-H)-FA 20:1(+HO) [-2]", "MLCL 14:0-16:1-18:2 - FA 14:0(+O) +FA 16:1(+O) + HG(GP)", "SHexCer 18:0;3/26:0;1 - LCB 18:1;3(-CH3O)", "PEt 16:0-18:1 - -(-NH4)", "CDPDAG 18:1-18:1 - HG(C,2,08)", "BMP 18:1-18:1 - FA 18:2(+O)+ HG(GP,155)", "PE O-18:0a/22:6-GP(136)", "Hex3Cer 18:1;2/24:1 - LCB 24:1(-CH3O2)", "DGDG 16:0-16:1 - HG(MGDG,379)", "LCB 17:1;2 -LCB(60)", "Hex2Cer 18:1;2/12:0 -  -(+CH3COO)", "Cer 18:1;2/16:0 - FA 18:1;2(-C2H3NO)", "MAG 16:0 - FA 17:0", "PIP2 21:0-22:6 - -(2 *H3PO4+H2O,214)", "LPC(+[13]C8) O-12:1a - (C5H13NO,104)(+[13]O)", "LPC(+[18]O7) O-12:1a - (C5H13NO,104)"}){
        try {
            lipid = lipid_maps_parser.parse(test_lipid_name);
            assert(false);
        }
        catch (LipidException &e){}
    }
    
    
    
    
    
    // testing dedicated lipid species
    lipid_name = "Cer 28:1;2";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "Cer 28:1;2");
    delete lipid;
    
    lipid_name = "DAG 38:1";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string() == "DAG 38:1");
    delete lipid;
    
    
    
    
    // testing mediators
    for (auto test_lipid_name : {"10-HDoHE","11-HDoHE","11-HETE","11,12-DHET","11(12)-EET", "12-HEPE","12-HETE","12-HHTrE","12-OxoETE","12(13)-EpOME","13-HODE","13-HOTrE","14,15-DHET","14(15)-EET","14(15)-EpETE","15-HEPE","15-HETE","15d-PGJ2","16-HDoHE","16-HETE","18-HEPE","5-HEPE","5-HETE","5-HpETE","5-OxoETE","5,12-DiHETE","5,6-DiHETE","5,6,15-LXA4","5(6)-EET","8-HDoHE","8-HETE","8,9-DHET","8(9)-EET","9-HEPE","9-HETE","9-HODE","9-HOTrE","9(10)-EpOME","AA","alpha-LA","DHA","EPA","Linoleic acid","LTB4","LTC4","LTD4","Maresin 1","Palmitic acid","PGB2","PGD2","PGE2","PGF2alpha","PGI2","Resolvin D1","Resolvin D2","Resolvin D3","Resolvin D5","tetranor-12-HETE","TXB1","TXB2","TXB3"}){
        
        lipid = goslin_parser.parse(test_lipid_name);
        assert (lipid);
        assert (lipid->get_lipid_string() == test_lipid_name);
        delete lipid;
    }
    
    
    
    
    
    
    
    // check if goslin parser fails correctly on parsing lipid name with fragment
    lipid_name = "PE 16:1-12:0 - -(H20)";
    try {
        lipid = goslin_parser.parse(lipid_name);
        assert(false);
    }
    catch (LipidException &e){}
    
    
    
    
    
    
    
    
        
    // test the down leveling of lipid names
    // glycerophospholipid;
    lipid_name = "PE 16:1(2E)/12:0";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string(ISOMERIC_SUBSPECIES) == "PE 16:1(2E)/12:0");
    assert (lipid->get_lipid_string(STRUCTURAL_SUBSPECIES) == "PE 16:1(2E)/12:0");
    assert (lipid->get_lipid_string(MOLECULAR_SUBSPECIES) == "PE 16:1(2E)-12:0");
    assert (lipid->get_lipid_string(SPECIES) == "PE 28:1");
    assert (lipid->get_lipid_string(CLASS) == "PE");
    assert (lipid->get_lipid_string(CATEGORY) == "GP");
    delete lipid;
    
    
    // sphingolipid;
    lipid_name = "Cer 16:1;2/12:0";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string(STRUCTURAL_SUBSPECIES) == "Cer 16:1;2/12:0");
    assert (lipid->get_lipid_string(MOLECULAR_SUBSPECIES) == "Cer 16:1;2-12:0");
    assert (lipid->get_lipid_string(SPECIES) == "Cer 28:1;2");
    assert (lipid->get_lipid_string(CLASS) == "Cer");
    assert (lipid->get_lipid_string(CATEGORY) == "SP");
    // check that all FAs have been initialized properly
    assert (lipid->lipid->get_fa_list().size() == 2);
    faCnt = 1;
    for(FattyAcid* fa:lipid->lipid->get_fa_list()) {
        // cout << (*fa).name << " (lcb: " << ((*fa).lcb?"true":"false") << "): pos=" << (*fa).position << ", #carbon=" << (*fa).num_carbon << ", #hydroxyl=" << (*fa).num_hydroxyl << ", #double-bonds=" << (*fa).num_double_bonds << endl;
        assert ((*fa).position == faCnt);
        switch(faCnt) {
        case 1:
        assert ((*fa).lcb == true);
        assert ((*fa).num_carbon == 16);
        assert ((*fa).num_hydroxyl == 2);
        assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
        assert ((*fa).num_double_bonds == 1);
        break;
        case 2:
        assert ((*fa).lcb == false);
        assert ((*fa).num_carbon == 12);
        assert ((*fa).num_hydroxyl == 0);
        assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
        assert ((*fa).num_double_bonds == 0);
        break;
        }
        ++faCnt;
    }
    delete lipid;
    
    // glycerolipid;
    lipid_name = "TAG 16:1/12:0/20:2";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string(STRUCTURAL_SUBSPECIES) == "TAG 16:1/12:0/20:2");
    assert (lipid->get_lipid_string(MOLECULAR_SUBSPECIES) == "TAG 16:1-12:0-20:2");
    assert (lipid->get_lipid_string(SPECIES) == "TAG 48:3");
    assert (lipid->get_lipid_string(CLASS) == "TAG");
    assert (lipid->get_lipid_string(CATEGORY) == "GL");
    assert (lipid->lipid);
    
    
    // try to retrieve LipidSpeciesInfo for summary information
    LipidSpeciesInfo lsi = lipid->lipid->info;
    assert (lsi.lcb == false);
    assert (lsi.level == STRUCTURAL_SUBSPECIES);
    assert (lsi.lipid_FA_bond_type == ESTER);
    assert (lsi.num_carbon == 48);
    assert (lsi.num_double_bonds == 3);
    assert (lsi.num_hydroxyl == 0);
    assert (lsi.position == 0);
    // check that all FAs have been initialized properly
    assert (lipid->lipid->get_fa_list().size() == 3);
    faCnt = 1;
    for(FattyAcid* fa:lipid->lipid->get_fa_list()) {
        // cout << (*fa).name << " (lcb: " << ((*fa).lcb?"true":"false") << "): pos=" << (*fa).position << ", #carbon=" << (*fa).num_carbon << ", #hydroxyl=" << (*fa).num_hydroxyl << ", #double-bonds=" << (*fa).num_double_bonds << endl;
        assert ((*fa).position == faCnt);
        switch(faCnt) {
        case 1:
            assert ((*fa).lcb == false);
            assert ((*fa).num_carbon == 16);
            assert ((*fa).num_hydroxyl == 0);
            assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
            assert ((*fa).num_double_bonds == 1);
            break;
        case 2:
            assert ((*fa).lcb == false);
            assert ((*fa).num_carbon == 12);
            assert ((*fa).num_hydroxyl == 0);
            assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
            assert ((*fa).num_double_bonds == 0);
            break;
        case 3:
            assert ((*fa).lcb == false);
            assert ((*fa).num_carbon == 20);
            assert ((*fa).num_hydroxyl == 0);
            assert (FattyAcid::suffix((*fa).lipid_FA_bond_type) == "");
            assert ((*fa).num_double_bonds == 2);
            break;
        }
        ++faCnt;
    }
    delete lipid;
    
    
    
    // sterol;
    lipid_name = "ChE 16:1";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string(STRUCTURAL_SUBSPECIES) == "SE 27:1/16:1");
    assert (lipid->get_lipid_string(MOLECULAR_SUBSPECIES) == "SE 27:1/16:1");
    assert (lipid->get_lipid_string(SPECIES) == "SE 27:1/16:1");
    assert (lipid->get_lipid_string(CLASS) == "SE 27:1");
    assert (lipid->get_lipid_string(CATEGORY) == "ST");
    delete lipid;
    
    
    
    
    // sterol;
    lipid_name = "ChE 16:1";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string(STRUCTURAL_SUBSPECIES) == "SE 27:1/16:1");
    assert (lipid->get_lipid_string(MOLECULAR_SUBSPECIES) == "SE 27:1/16:1");
    assert (lipid->get_lipid_string(SPECIES) == "SE 27:1/16:1");
    assert (lipid->get_lipid_string(CLASS) == "SE 27:1");
    assert (lipid->get_lipid_string(CATEGORY) == "ST");
    delete lipid;
    

    
    // PC;
    lipid_name = "PC O-16:1a/12:0";
    lipid = goslin_parser.parse(lipid_name);
    assert (lipid);
    assert (lipid->get_lipid_string(STRUCTURAL_SUBSPECIES) == "PC O-16:1a/12:0");
    assert (lipid->get_lipid_string(MOLECULAR_SUBSPECIES) == "PC O-16:1a-12:0");
    assert (lipid->get_lipid_string(SPECIES) == "PC O-28:1a");
    assert (lipid->get_lipid_string(CLASS) == "PC");
    assert (lipid->get_lipid_string(CATEGORY) == "GP");
    delete lipid;
    
    
    // testing adducts
    lipid_name = "PE 16:1/12:0[M+H]1+";
    lipid = goslin_parser.parse(lipid_name);
    assert(lipid);
    assert (lipid->get_lipid_string() == "PE 16:1/12:0[M+H]1+");
    delete lipid;

    
    
    
    
    

    // test several more lipid names
    vector<string> lipidnames;
    ifstream infile("cppgoslin/tests/lipidnames.txt");
    string line;
    while (getline(infile, line)){
        line = strip(line, ' ');
        if (line.length() < 2) continue;
        if (line[0] == '#') continue;
        vector<string> *v = split_string(line, ',', '"');
        lipidnames.push_back(strip(v->at(0), '"'));
        delete v;
    }
    infile.close();
    
    for (auto test_lipid_name : lipidnames){
        lipid = lipid_parser.parse(test_lipid_name);
        if (lipid == NULL){
            cout << "Fail: '" << test_lipid_name << "'" << endl;
        }
        assert(lipid);
        delete lipid;
    }
        

    cout << "All tests passed without any problem" << endl;
    
    return EXIT_SUCCESS;
}
