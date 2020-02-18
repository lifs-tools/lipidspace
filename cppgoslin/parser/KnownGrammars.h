#ifndef KNOWN_GRAMMARS_H
#define KNOWN_GRAMMARS_H

#include <string>
#include "cppgoslin/parser/Parser.h"
using namespace std;
static const string goslin_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) 2017 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de \n\
 *                    Bing Peng   -   bing.peng {at} isas.de \n\
 *                    Nils Hoffmann  -  nils.hoffmann {at} isas.de \n\
 * \n\
 * Permission is hereby granted, free of charge, to any person obtaining a copy \n\
 * of this software and associated documentation files (the 'Software'), to deal \n\
 * in the Software without restriction, including without limitation the rights \n\
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell \n\
 * copies of the Software, and to permit persons to whom the Software is \n\
 * furnished to do so, subject to the following conditions:; \n\
 *  \n\
 * The above copyright notice and this permission notice shall be included in all \n\
 * copies or substantial portions of the Software. \n\
 *  \n\
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n\
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \n\
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE \n\
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER \n\
 * LIABILITY, WHether IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \n\
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE \n\
 * SOFTWARE. \n\
*/ \n\
 \n\
/* This is a BNF / ANTLR4 grammar for lipid subspecies identifiers following \n\
 * J.K. Pauling et al. 2017, PLoS One, 12(11):e0188394. \n\
 */ \n\
 \n\
grammar Goslin; \n\
 \n\
 \n\
/* first rule is always start rule */ \n\
lipid : lipid_eof EOF; \n\
lipid_eof : lipid_pure | lipid_pure adduct_info; \n\
lipid_pure : gl | pl | sl | cholesterol | mediatorc | saccharolipid; \n\
 \n\
 \n\
/* adduct information */ \n\
adduct_info : '[M' adduct ']' charge charge_sign | adduct_separator '[M' adduct ']' charge charge_sign; \n\
adduct : '+H' | '+2H' | '+NH4' | '-H' | '-2H' | '+HCOO' | '+CH3COO' | charge_sign arbitrary_adduct; \n\
arbitrary_adduct : adduct4 | adduct4 adduct4; \n\
adduct4 : adduct2 | adduct2 adduct2; \n\
adduct2 : character | character character; \n\
 \n\
fa2 : fa2_unsorted | fa2_sorted; \n\
fa2_unsorted: fa DASH fa | fa UNDERSCORE fa; \n\
fa2_sorted: fa SLASH fa | fa BACKSLASH fa; \n\
fa3 : fa3_unsorted | fa3_sorted; \n\
fa3_unsorted: fa DASH fa DASH fa | fa UNDERSCORE fa UNDERSCORE fa; \n\
fa3_sorted: fa SLASH fa SLASH fa | fa BACKSLASH fa BACKSLASH fa; \n\
fa4 : fa4_unsorted | fa4_sorted; \n\
fa4_unsorted: fa DASH fa DASH fa DASH fa | fa UNDERSCORE fa UNDERSCORE fa UNDERSCORE fa; \n\
fa4_sorted: fa SLASH fa SLASH fa SLASH fa | fa BACKSLASH fa BACKSLASH fa BACKSLASH fa; \n\
 \n\
/* glycerolipid rules (7 classes) */ \n\
gl : sqml | mgl | dgl | sgl | tgl; \n\
sqml : hg_sqml_full headgroup_separator fa; \n\
mgl : hg_mgl_full headgroup_separator fa; \n\
dgl : hg_dgl_full headgroup_separator gl_species | hg_dgl_full headgroup_separator dgl_subspecies; \n\
sgl : hg_sgl_full headgroup_separator gl_species | hg_sgl_full headgroup_separator dgl_subspecies; \n\
tgl : hg_tgl_full headgroup_separator gl_species | hg_tgl_full headgroup_separator tgl_subspecies; \n\
gl_species : fa; \n\
dgl_subspecies : fa2; \n\
tgl_subspecies : fa3; \n\
 \n\
hg_sqml_full : hg_sqml | hg_mgl heavy_hg; \n\
hg_mgl_full : hg_mgl | hg_mgl heavy_hg; \n\
hg_dgl_full : hg_dgl | hg_dgl heavy_hg; \n\
hg_sgl_full : hg_sgl | hg_sgl heavy_hg; \n\
hg_tgl_full : hg_tgl | hg_tgl heavy_hg; \n\
 \n\
hg_sqml : 'SQMG'; \n\
hg_mgl : 'MAG'; \n\
hg_dgl : 'DAG'; \n\
hg_sgl : 'MGDG' | 'DGDG' | 'SQDG'; \n\
hg_tgl : 'TAG'; \n\
 \n\
 \n\
 \n\
/* phospholipid rules (56 classes) */ \n\
pl : lpl | dpl | tpl | pl_o | cl | mlcl | dlcl; \n\
pl_o : lpl_o | dpl_o; \n\
lpl : hg_lplc headgroup_separator fa; \n\
lpl_o : hg_lpl_oc plasmalogen_separator fa; \n\
dpl : hg_plc headgroup_separator pl_species | hg_plc headgroup_separator pl_subspecies; \n\
tpl : hg_tplc headgroup_separator pl_species | hg_tplc headgroup_separator tpl_subspecies; \n\
dpl_o : hg_pl_oc plasmalogen_separator pl_species | hg_pl_oc plasmalogen_separator pl_subspecies; \n\
dlcl : hg_dlclc headgroup_separator pl_species | hg_dlclc headgroup_separator dlcl_subspecies; \n\
mlcl : hg_mlclc headgroup_separator pl_species | hg_mlclc headgroup_separator mlcl_subspecies; \n\
cl : hg_clc headgroup_separator pl_species | hg_clc headgroup_separator cl_subspecies; \n\
 \n\
pl_species : fa; \n\
pl_subspecies : fa2; \n\
tpl_subspecies : fa3; \n\
dlcl_subspecies : fa2; \n\
mlcl_subspecies : fa3; \n\
cl_subspecies : fa4; \n\
 \n\
heavy_hg : heavy; \n\
 \n\
 \n\
hg_clc : hg_cl | hg_cl heavy_hg; \n\
hg_cl : 'CL'; \n\
hg_mlclc : hg_mlcl | hg_mlcl heavy_hg; \n\
hg_mlcl : 'MLCL'; \n\
hg_dlclc : hg_dlcl | hg_dlcl heavy_hg; \n\
hg_dlcl : 'DLCL'; \n\
hg_plc : hg_pl | hg_pl heavy_hg; \n\
hg_pl : 'BMP' | 'CDP-DAG' | 'DMPE' | 'MMPE' | 'PA' | 'PC' | 'PE' | 'PEt' | 'PG' | 'PI' | hg_pip | 'PS' | 'LBPA' | 'PGP' | 'PPA' | 'Glc-GP' | '6-Ac-Glc-GP' | hg_pim | 'PnC' | 'PnE' | 'PT' | 'PE-NMe2' | 'PE-NMe' | 'PIMIP'; \n\
hg_pim : 'PIM' hg_pim_number; \n\
hg_pim_number : number; \n\
hg_pip : hg_pip_pure | hg_pip_pure hg_pip_m | hg_pip_pure hg_pip_d | hg_pip_pure hg_pip_t; \n\
hg_pip_pure : 'PIP'; \n\
hg_pip_m : '[3\\']' | '[4\\']' | '[5\\']'; \n\
hg_pip_d : '2' | '2[3\\',4\\']' | '2[4\\',5\\']' | '2[3\\',5\\']'; \n\
hg_pip_t : '3' | '3[3\\',4\\',5\\']'; \n\
hg_tplc : hg_tpl | hg_tpl heavy_hg; \n\
hg_tpl : 'SLBPA' | 'NAPE'; \n\
hg_lplc : hg_lpl | hg_lpl heavy_hg; \n\
hg_lpl : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | hg_lpim | 'CPA' | 'LCDPDAG' | 'LDMPE' | 'LMMPE' | 'LPIMIP' | 'LPIN'; \n\
hg_lpim : 'LPIM' hg_lpim_number; \n\
hg_lpim_number : number; \n\
hg_lpl_oc : hg_lpl_o ' O' | hg_lpl_o heavy_hg ' O'; \n\
hg_lpl_o : 'LPC' | 'LPE'; \n\
hg_pl_oc : hg_pl_o ' O' | hg_pl_o heavy_hg ' O'; \n\
hg_pl_o : 'PA' | 'PC' | 'PE' | 'PG' | 'PI' | 'PS'; \n\
 \n\
 \n\
 \n\
/* sphingolipid rules (21) */ \n\
sl : lsl | dsl; \n\
lsl : hg_lslc headgroup_separator lcb; \n\
dsl : hg_dslc headgroup_separator sl_species | hg_dslc headgroup_separator sl_subspecies; \n\
 \n\
sl_species : lcb; \n\
sl_subspecies : lcb sorted_fa_separator fa; \n\
 \n\
hg_lslc : hg_lsl | hg_lsl heavy_hg; \n\
hg_lsl : 'LCB' | 'LCBP' | 'LHexCer' | 'LSM'; \n\
hg_dslc : hg_dsl | hg_dsl heavy_hg; \n\
hg_dsl : 'Cer' | 'CerP' | 'EPC' | 'GB4' | 'GD3' | 'GM3' | 'GM4' | 'Hex3Cer' | 'Hex2Cer' | 'HexCer' | 'IPC' | 'M(IP)2C' | 'MIPC' | 'SHexCer' | 'SM' | 'FMC-5' | 'FMC-6' ; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* cholesterol lipids (2 classes) */ \n\
cholesterol : chc | che; \n\
chc : ch | ch heavy_hg; \n\
ch : 'Ch' | 'Cholesterol'; \n\
che : hg_chec headgroup_separator fa; \n\
hg_chec : hg_che | hg_che heavy_hg; \n\
hg_che : 'ChE' | 'CE'; \n\
 \n\
 \n\
/* mediator lipids (1 class) */ \n\
mediatorc : mediator | mediator heavy_hg; \n\
mediator : '10-HDoHE' | '11-HDoHE' | '11-HETE' | '11,12-DHET' | '11(12)-EET'| '12-HEPE' | '12-HETE' | '12-HHTrE' | '12-OxoETE' | '12(13)-EpOME' | '13-HODE' | '13-HOTrE' | '14,15-DHET' | '14(15)-EET' | '14(15)-EpETE' | '15-HEPE' | '15-HETE' | '15d-PGJ2' | '16-HDoHE' | '16-HETE' | '18-HEPE' | '5-HEPE' | '5-HETE' | '5-HpETE' | '5-OxoETE' | '5,12-DiHETE' | '5,6-DiHETE' | '5,6,15-LXA4' | '5(6)-EET' | '8-HDoHE' | '8-HETE' | '8,9-DHET' | '8(9)-EET' | '9-HEPE' | '9-HETE' | '9-HODE' | '9-HOTrE' | '9(10)-EpOME' | 'AA' | 'alpha-LA' | 'DHA' | 'EPA' | 'Linoleic acid' | 'LTB4' | 'LTC4' | 'LTD4' | 'Maresin 1' | 'Palmitic acid' | 'PGB2' | 'PGD2' | 'PGE2' | 'PGF2alpha' | 'PGI2' | 'Resolvin D1' | 'Resolvin D2' | 'Resolvin D3' | 'Resolvin D5' | 'tetranor-12-HETE' | 'TXB1' | 'TXB2' | 'TXB3'; \n\
 \n\
 \n\
 \n\
 \n\
/* saccharolipids rules (3 classes) */ \n\
saccharolipid : sac_di | sac_f; \n\
sac_di : hg_sac_di_c headgroup_separator sac_species | hg_sac_di_c headgroup_separator sac_di_subspecies; \n\
hg_sac_di_c : hg_sac_di | hg_sac_di heavy_hg; \n\
hg_sac_di : 'DAT' | 'AC2SGL'; \n\
sac_f : hg_sac_f_c headgroup_separator sac_species | hg_sac_f_c headgroup_separator sac_f_subspecies; \n\
hg_sac_f_c : hg_sac_f | hg_sac_f heavy_hg; \n\
hg_sac_f : 'PAT16' | 'PAT18'; \n\
 \n\
sac_species : fa; \n\
sac_di_subspecies : fa2; \n\
sac_f_subspecies : fa4; \n\
 \n\
 \n\
 \n\
/* generic rules */ \n\
fa : fa_pure | fa_pure heavy_fa | fa_pure ether | fa_pure ether heavy_fa; \n\
heavy_fa : heavy; \n\
fa_pure : carbon carbon_db_separator db | carbon carbon_db_separator db db_hydroxyl_separator hydroxyl; \n\
ether : 'a' | 'p'; \n\
lcb : lcb_pure | lcb_pure heavy_lcb; \n\
heavy_lcb : heavy; \n\
lcb_pure : carbon carbon_db_separator db db_hydroxyl_separator hydroxyl | old_hydroxyl carbon carbon_db_separator db; \n\
carbon : number; \n\
db : db_count | db_count round_open_bracket db_position round_close_bracket; \n\
db_count : number; \n\
db_position : number cistrans | number cistrans | db_position db_position_separator db_position; \n\
cistrans : 'E' | 'Z'; \n\
hydroxyl : number; \n\
old_hydroxyl : 'd' | 't'; \n\
number :  digit; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | digit digit; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
heavy : '(+' isotopes  ')'; \n\
isotopes : isotopes isotopes | isotope; \n\
isotope : '[' isotope_number ']' isotope_element isotope_count | '[' isotope_number ']' isotope_element; \n\
isotope_number : number; \n\
isotope_element : element; \n\
isotope_count : number; \n\
element : 'C' | 'H' | 'O' | 'N' | 'P' | 'S'; \n\
 \n\
/* separators */ \n\
SPACE : ' '; \n\
COLON : ':'; \n\
SEMICOLON : ';'; \n\
DASH : '-'; \n\
UNDERSCORE : '_'; \n\
SLASH : '/'; \n\
BACKSLASH : '\\\\'; \n\
COMMA: ','; \n\
ROB: '('; \n\
RCB: ')'; \n\
FRAGMENT_SEPARATOR : ' - '; \n\
 \n\
sorted_fa_separator : SLASH | BACKSLASH; \n\
adduct_separator : SPACE; \n\
unsorted_fa_separator : DASH | UNDERSCORE; \n\
plasmalogen_separator : headgroup_separator | DASH; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
db_hydroxyl_separator : SEMICOLON; \n\
db_position_separator : COMMA; \n\
round_open_bracket : ROB; \n\
round_close_bracket : RCB; \n\
 \n\
character : 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' |'0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : '-' | '+'; \n\
 \n\
";



static const string goslin_fragment_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) 2017 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de \n\
 *                    Bing Peng   -   bing.peng {at} isas.de \n\
 *                    Nils Hoffmann  -  nils.hoffmann {at} isas.de \n\
 * \n\
 * Permission is hereby granted, free of charge, to any person obtaining a copy \n\
 * of this software and associated documentation files (the 'Software'), to deal \n\
 * in the Software without restriction, including without limitation the rights \n\
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell \n\
 * copies of the Software, and to permit persons to whom the Software is \n\
 * furnished to do so, subject to the following conditions:; \n\
 *  \n\
 * The above copyright notice and this permission notice shall be included in all \n\
 * copies or substantial portions of the Software. \n\
 *  \n\
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n\
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \n\
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE \n\
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER \n\
 * LIABILITY, WHether IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \n\
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE \n\
 * SOFTWARE. \n\
*/ \n\
 \n\
/* This is a BNF / ANTLR4 grammar for lipid subspecies identifiers following \n\
 * J.K. Pauling et al. 2017, PLoS One, 12(11):e0188394. \n\
 */ \n\
 \n\
grammar GoslinFragments; \n\
 \n\
/* first rule is always start rule */ \n\
lipid : lipid_eof EOF; \n\
lipid_eof : just_lipid | just_lipid FRAGMENT_SEPARATOR fragment_name; \n\
just_lipid : lipid_pure | lipid_pure adduct_info; \n\
lipid_pure : gl | pl | sl | cholesterol | mediatorc | saccharolipid; \n\
 \n\
/* rules for fragments */ \n\
fragment_name : frag_char; \n\
frag_char : frag_char frag_char | character | sign; \n\
sign : '-' | '+' | ' ' | ',' | '(' | ')' | '[' | ']' | ':' | '*' | ';'; \n\
 \n\
/* adduct information */ \n\
adduct_info : '[M' adduct ']' charge charge_sign | adduct_separator '[M' adduct ']' charge charge_sign; \n\
adduct : '+H' | '+2H' | '+NH4' | '-H' | '-2H' | '+HCOO' | '+CH3COO' | charge_sign arbitrary_adduct; \n\
arbitrary_adduct : adduct4 | adduct4 adduct4; \n\
adduct4 : adduct2 | adduct2 adduct2; \n\
adduct2 : character | character character; \n\
 \n\
fa2 : fa2_unsorted | fa2_sorted; \n\
fa2_unsorted: fa DASH fa | fa UNDERSCORE fa; \n\
fa2_sorted: fa SLASH fa | fa BACKSLASH fa; \n\
fa3 : fa3_unsorted | fa3_sorted; \n\
fa3_unsorted: fa DASH fa DASH fa | fa UNDERSCORE fa UNDERSCORE fa; \n\
fa3_sorted: fa SLASH fa SLASH fa | fa BACKSLASH fa BACKSLASH fa; \n\
fa4 : fa4_unsorted | fa4_sorted; \n\
fa4_unsorted: fa DASH fa DASH fa DASH fa | fa UNDERSCORE fa UNDERSCORE fa UNDERSCORE fa; \n\
fa4_sorted: fa SLASH fa SLASH fa SLASH fa | fa BACKSLASH fa BACKSLASH fa BACKSLASH fa; \n\
 \n\
/* glycerolipid rules */ \n\
gl : sqml | mgl | dgl | sgl | tgl; \n\
sqml : hg_sqml_full headgroup_separator fa; \n\
mgl : hg_mgl_full headgroup_separator fa; \n\
dgl : hg_dgl_full headgroup_separator gl_species | hg_dgl_full headgroup_separator dgl_subspecies; \n\
sgl : hg_sgl_full headgroup_separator gl_species | hg_sgl_full headgroup_separator dgl_subspecies; \n\
tgl : hg_tgl_full headgroup_separator gl_species | hg_tgl_full headgroup_separator tgl_subspecies; \n\
gl_species : fa; \n\
dgl_subspecies : fa2; \n\
tgl_subspecies : fa3; \n\
 \n\
hg_sqml_full : hg_sqml | hg_mgl heavy_hg; \n\
hg_mgl_full : hg_mgl | hg_mgl heavy_hg; \n\
hg_dgl_full : hg_dgl | hg_dgl heavy_hg; \n\
hg_sgl_full : hg_sgl | hg_sgl heavy_hg; \n\
hg_tgl_full : hg_tgl | hg_tgl heavy_hg; \n\
 \n\
hg_sqml : 'SQMG'; \n\
hg_mgl : 'MAG'; \n\
hg_dgl : 'DAG'; \n\
hg_sgl : 'MGDG' | 'DGDG' | 'SQDG'; \n\
hg_tgl : 'TAG'; \n\
 \n\
 \n\
 \n\
/* phospholipid rules */ \n\
pl : lpl | dpl | tpl | pl_o | cl | mlcl | dlcl; \n\
pl_o : lpl_o | dpl_o; \n\
lpl : hg_lplc headgroup_separator fa; \n\
lpl_o : hg_lpl_oc plasmalogen_separator fa; \n\
dpl : hg_plc headgroup_separator pl_species | hg_plc headgroup_separator pl_subspecies; \n\
tpl : hg_tplc headgroup_separator pl_species | hg_tplc headgroup_separator tpl_subspecies; \n\
dpl_o : hg_pl_oc plasmalogen_separator pl_species | hg_pl_oc plasmalogen_separator pl_subspecies; \n\
dlcl : hg_dlclc headgroup_separator pl_species | hg_dlclc headgroup_separator dlcl_subspecies; \n\
mlcl : hg_mlclc headgroup_separator pl_species | hg_mlclc headgroup_separator mlcl_subspecies; \n\
cl : hg_clc headgroup_separator pl_species | hg_clc headgroup_separator cl_subspecies; \n\
 \n\
pl_species : fa; \n\
pl_subspecies : fa2; \n\
tpl_subspecies : fa3; \n\
dlcl_subspecies : fa2; \n\
mlcl_subspecies : fa3; \n\
cl_subspecies : fa4; \n\
 \n\
heavy_hg : heavy; \n\
 \n\
 \n\
hg_clc : hg_cl | hg_cl heavy_hg; \n\
hg_cl : 'CL'; \n\
hg_mlclc : hg_mlcl | hg_mlcl heavy_hg; \n\
hg_mlcl : 'MLCL'; \n\
hg_dlclc : hg_dlcl | hg_dlcl heavy_hg; \n\
hg_dlcl : 'DLCL'; \n\
hg_plc : hg_pl | hg_pl heavy_hg; \n\
hg_pl : 'BMP' | 'CDP-DAG' | 'DMPE' | 'MMPE' | 'PA' | 'PC' | 'PE' | 'PEt' | 'PG' | 'PI' | hg_pip | 'PS' | 'LBPA' | 'PGP' | 'PPA' | 'Glc-GP' | '6-Ac-Glc-GP' | hg_pim | 'PnC' | 'PnE' | 'PT' | 'PE-NMe2' | 'PE-NMe' | 'PIMIP'; \n\
hg_pim : 'PIM' hg_pim_number; \n\
hg_pim_number : number; \n\
hg_pip : hg_pip_pure | hg_pip_pure hg_pip_m | hg_pip_pure hg_pip_d | hg_pip_pure hg_pip_t; \n\
hg_pip_pure : 'PIP'; \n\
hg_pip_m : '[3\\']' | '[4\\']' | '[5\\']'; \n\
hg_pip_d : '2' | '2[3\\',4\\']' | '2[4\\',5\\']' | '2[3\\',5\\']'; \n\
hg_pip_t : '3' | '3[3\\',4\\',5\\']'; \n\
hg_tplc : hg_tpl | hg_tpl heavy_hg; \n\
hg_tpl : 'SLBPA' | 'NAPE'; \n\
hg_lplc : hg_lpl | hg_lpl heavy_hg; \n\
hg_lpl : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | hg_lpim | 'CPA' | 'LCDPDAG' | 'LDMPE' | 'LMMPE' | 'LPIMIP' | 'LPIN'; \n\
hg_lpim : 'LPIM' hg_lpim_number; \n\
hg_lpim_number : number; \n\
hg_lpl_oc : hg_lpl_o ' O' | hg_lpl_o heavy_hg ' O'; \n\
hg_lpl_o : 'LPC' | 'LPE'; \n\
hg_pl_oc : hg_pl_o ' O' | hg_pl_o heavy_hg ' O'; \n\
hg_pl_o : 'PA' | 'PC' | 'PE' | 'PG' | 'PI' | 'PS'; \n\
 \n\
 \n\
 \n\
/* sphingolipid rules */ \n\
sl : lsl | dsl; \n\
lsl : hg_lslc headgroup_separator lcb; \n\
dsl : hg_dslc headgroup_separator sl_species | hg_dslc headgroup_separator sl_subspecies; \n\
 \n\
sl_species : lcb; \n\
sl_subspecies : lcb sorted_fa_separator fa; \n\
 \n\
hg_lslc : hg_lsl | hg_lsl heavy_hg; \n\
hg_lsl : 'LCB' | 'LCBP' | 'LHexCer' | 'LSM'; \n\
hg_dslc : hg_dsl | hg_dsl heavy_hg; \n\
hg_dsl : 'Cer' | 'CerP' | 'EPC' | 'GB4' | 'GD3' | 'GM3' | 'GM4' | 'Hex3Cer' | 'Hex2Cer' | 'HexCer' | 'IPC' | 'M(IP)2C' | 'MIPC' | 'SHexCer' | 'SM' | 'FMC-5' | 'FMC-6' ; \n\
 \n\
 \n\
 \n\
/* cholesterol lipids */ \n\
cholesterol : chc | che; \n\
chc : ch | ch heavy_hg; \n\
ch : 'Ch' | 'Cholesterol'; \n\
che : hg_chec headgroup_separator fa; \n\
hg_chec : hg_che | hg_che heavy_hg; \n\
hg_che : 'ChE' | 'CE'; \n\
 \n\
 \n\
/* mediator lipids */ \n\
mediatorc : mediator | mediator heavy_hg; \n\
mediator : '10-HDoHE' | '11-HDoHE' | '11-HETE' | '11,12-DHET' | '11(12)-EET'| '12-HEPE' | '12-HETE' | '12-HHTrE' | '12-OxoETE' | '12(13)-EpOME' | '13-HODE' | '13-HOTrE' | '14,15-DHET' | '14(15)-EET' | '14(15)-EpETE' | '15-HEPE' | '15-HETE' | '15d-PGJ2' | '16-HDoHE' | '16-HETE' | '18-HEPE' | '5-HEPE' | '5-HETE' | '5-HpETE' | '5-OxoETE' | '5,12-DiHETE' | '5,6-DiHETE' | '5,6,15-LXA4' | '5(6)-EET' | '8-HDoHE' | '8-HETE' | '8,9-DHET' | '8(9)-EET' | '9-HEPE' | '9-HETE' | '9-HODE' | '9-HOTrE' | '9(10)-EpOME' | 'AA' | 'alpha-LA' | 'DHA' | 'EPA' | 'Linoleic acid' | 'LTB4' | 'LTC4' | 'LTD4' | 'Maresin 1' | 'Palmitic acid' | 'PGB2' | 'PGD2' | 'PGE2' | 'PGF2alpha' | 'PGI2' | 'Resolvin D1' | 'Resolvin D2' | 'Resolvin D3' | 'Resolvin D5' | 'tetranor-12-HETE' | 'TXB1' | 'TXB2' | 'TXB3'; \n\
 \n\
 \n\
 \n\
 \n\
/* saccharolipids rules */ \n\
saccharolipid : sac_di | sac_f; \n\
sac_di : hg_sac_di_c headgroup_separator sac_species | hg_sac_di_c headgroup_separator sac_di_subspecies; \n\
hg_sac_di_c : hg_sac_di | hg_sac_di heavy_hg; \n\
hg_sac_di : 'DAT' | 'AC2SGL'; \n\
sac_f : hg_sac_f_c headgroup_separator sac_species | hg_sac_f_c headgroup_separator sac_f_subspecies; \n\
hg_sac_f_c : hg_sac_f | hg_sac_f heavy_hg; \n\
hg_sac_f : 'PAT16' | 'PAT18'; \n\
 \n\
sac_species : fa; \n\
sac_di_subspecies : fa2; \n\
sac_f_subspecies : fa4; \n\
 \n\
 \n\
 \n\
/* generic rules */ \n\
fa : fa_pure | fa_pure heavy_fa | fa_pure ether | fa_pure ether heavy_fa; \n\
heavy_fa : heavy; \n\
fa_pure : carbon carbon_db_separator db | carbon carbon_db_separator db db_hydroxyl_separator hydroxyl; \n\
ether : 'a' | 'p'; \n\
lcb : lcb_pure | lcb_pure heavy_lcb; \n\
heavy_lcb : heavy; \n\
lcb_pure : carbon carbon_db_separator db db_hydroxyl_separator hydroxyl | old_hydroxyl carbon carbon_db_separator db; \n\
carbon : number; \n\
db : db_count | db_count round_open_bracket db_position round_close_bracket; \n\
db_count : number; \n\
db_position : number cistrans | number cistrans | db_position db_position_separator db_position; \n\
cistrans : 'E' | 'Z'; \n\
hydroxyl : number; \n\
old_hydroxyl : 'd' | 't'; \n\
number :  digit; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | digit digit; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
heavy : '(+' isotopes  ')'; \n\
isotopes : isotopes isotopes | isotope; \n\
isotope : '[' isotope_number ']' isotope_element isotope_count | '[' isotope_number ']' isotope_element; \n\
isotope_number : number; \n\
isotope_element : element; \n\
isotope_count : number; \n\
element : 'C' | 'H' | 'O' | 'N' | 'P' | 'S'; \n\
 \n\
/* separators */ \n\
SPACE : ' '; \n\
COLON : ':'; \n\
SEMICOLON : ';'; \n\
DASH : '-'; \n\
UNDERSCORE : '_'; \n\
SLASH : '/'; \n\
BACKSLASH : '\\\\'; \n\
COMMA: ','; \n\
ROB: '('; \n\
RCB: ')'; \n\
FRAGMENT_SEPARATOR : ' - '; \n\
 \n\
sorted_fa_separator : SLASH | BACKSLASH; \n\
adduct_separator : SPACE; \n\
unsorted_fa_separator : DASH | UNDERSCORE; \n\
plasmalogen_separator : headgroup_separator | DASH; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
db_hydroxyl_separator : SEMICOLON; \n\
db_position_separator : COMMA; \n\
round_open_bracket : ROB; \n\
round_close_bracket : RCB; \n\
 \n\
character : 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' |'0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : '-' | '+'; \n\
";



static const string lipid_maps_grammar = "//////////////////////////////////////////////////////////////////////////////// \n\
// MIT License \n\
//  \n\
// Copyright (c) 2017 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de \n\
//                    Bing Peng   -   bing.peng {at} isas.de \n\
//                    Nils Hoffmann  -  nils.hoffmann {at} isas.de \n\
// \n\
// Permission is hereby granted, free of charge, to any person obtaining a copy \n\
// of this software and associated documentation files (the 'Software'), to deal \n\
// in the Software without restriction, including without limitation the rights \n\
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell \n\
// copies of the Software, and to permit persons to whom the Software is \n\
// furnished to do so, subject to the following conditions: \n\
//  \n\
// The above copyright notice and this permission notice shall be included in all \n\
// copies or substantial portions of the Software. \n\
//  \n\
// THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n\
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \n\
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE \n\
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER \n\
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \n\
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE \n\
// SOFTWARE. \n\
//////////////////////////////////////////////////////////////////////////////// \n\
 \n\
//// This is a BNF grammer for lipid subspecies identifiers followed by \n\
//// J.K. Pauling et al. 2017, PLoS One, 12(11):e0188394.  \n\
 \n\
grammar LipidMaps; \n\
 \n\
/* first rule is always start rule */ \n\
lipid: lipid_rule EOF; \n\
lipid_rule: lipid_mono | lipid_mono isotope; \n\
lipid_mono: lipid_pure | lipid_pure isoform; \n\
lipid_pure: pure_fa | gl | pl | sl | cholesterol | mediator; \n\
isoform: square_open_bracket 'rac' square_close_bracket | square_open_bracket 'iso' number square_close_bracket; \n\
isotope: SPACE round_open_bracket element number round_close_bracket | DASH round_open_bracket element number round_close_bracket | DASH element number; \n\
element: 'd'; \n\
 \n\
 \n\
/* pure fatty acid */ \n\
pure_fa: hg_fa pure_fa_species | fa; \n\
pure_fa_species: round_open_bracket fa round_close_bracket; \n\
hg_fa: 'FA' | 'WE'; \n\
 \n\
fa2 : fa2_unsorted | fa2_sorted; \n\
fa2_unsorted: fa DASH fa | fa UNDERSCORE fa; \n\
fa2_sorted: fa SLASH fa | fa BACKSLASH fa; \n\
fa3 : fa3_unsorted | fa3_sorted; \n\
fa3_unsorted: fa DASH fa DASH fa | fa UNDERSCORE fa UNDERSCORE fa; \n\
fa3_sorted: fa SLASH fa SLASH fa | fa BACKSLASH fa BACKSLASH fa; \n\
fa4 : fa4_unsorted | fa4_sorted; \n\
fa4_unsorted: fa DASH fa DASH fa DASH fa | fa UNDERSCORE fa UNDERSCORE fa UNDERSCORE fa; \n\
fa4_sorted: fa SLASH fa SLASH fa SLASH fa | fa BACKSLASH fa BACKSLASH fa BACKSLASH fa; \n\
lcb_fa_sorted: lcb SLASH fa | lcb BACKSLASH fa; \n\
 \n\
/* glycerolipid rules */ \n\
gl: sgl | tgl; \n\
sgl: hg_sglc sgl_species | hg_sglc sgl_subspecies; \n\
sgl_species: round_open_bracket fa round_close_bracket | fa; \n\
sgl_subspecies: round_open_bracket fa2 round_close_bracket | fa2; \n\
tgl: hg_glc tgl_species | hg_glc tgl_subspecies; \n\
tgl_species: round_open_bracket fa round_close_bracket | fa; \n\
tgl_subspecies: round_open_bracket fa3 round_close_bracket | fa3; \n\
 \n\
hg_sglc: hg_sgl | hg_sgl headgroup_separator; \n\
hg_sgl: 'MGDG' | 'DGDG' | 'SQDG' | 'SQMG' | 'DG'; \n\
hg_glc: hg_gl | hg_gl headgroup_separator; \n\
hg_gl: 'MG' | 'DG' | 'TG'; \n\
 \n\
 \n\
 \n\
 \n\
/* phospholipid rules */ \n\
pl: lpl | dpl | cl | fourpl; \n\
lpl: hg_lplc round_open_bracket fa_lpl round_close_bracket | hg_lplc fa_lpl; \n\
fa_lpl: fa | fa2; \n\
dpl: hg_ddpl dpl_species | hg_ddpl dpl_subspecies; \n\
dpl_species: round_open_bracket fa round_close_bracket | fa; \n\
dpl_subspecies: round_open_bracket fa2 round_close_bracket | fa2; \n\
cl: hg_clc cl_species | hg_clc cl_subspecies; \n\
cl_species: round_open_bracket fa round_close_bracket | fa; \n\
cl_subspecies: round_open_bracket '1\\'-' square_open_bracket fa2 square_close_bracket ',3\\'-' square_open_bracket fa2 square_close_bracket round_close_bracket | hg_clc '1\\'-' square_open_bracket fa2 square_close_bracket ',3\\'-' square_open_bracket fa2 square_close_bracket; \n\
fourpl: hg_fourplc round_open_bracket fa4 round_close_bracket | hg_fourplc fa4; \n\
 \n\
hg_ddpl: hg_dplc pip_position | hg_dplc; \n\
 \n\
hg_clc: hg_cl | hg_cl headgroup_separator; \n\
hg_cl: 'CL'; \n\
hg_dplc: hg_dpl | hg_dpl headgroup_separator; \n\
hg_dpl: 'LBPA' | 'CDP-DG' | 'DMPE' | 'MMPE' | 'PA' | 'PC' | 'PE' | 'PEt' | 'PG' | 'PI' | 'PIP' | 'PIP2' | 'PIP3' | 'PS' | 'PIM1' | 'PIM2' | 'PIM3' | 'PIM4' | 'PIM5' | 'PIM6' | 'Glc-DG' | 'PGP' | 'PE-NMe2' | 'AC2SGL' | 'DAT' | 'PE-NMe' | 'PT' | 'Glc-GP' | 'NAPE'; \n\
hg_lplc: hg_lpl | hg_lpl headgroup_separator; \n\
hg_lpl: 'LysoPC' | 'LPC' | 'LysoPE' | 'LPE' | 'LPIM1' | 'LPIM2' | 'LPIM3' | 'LPIM4' | 'LPIM5' | 'LPIM6' | 'CPA' | 'LPA'; \n\
hg_fourplc: hg_fourpl | hg_fourpl headgroup_separator; \n\
hg_fourpl: 'PAT16' | 'PAT18'; \n\
pip_position: square_open_bracket pip_pos square_close_bracket; \n\
pip_pos: pip_pos COMMA pip_pos | number '\\''; \n\
 \n\
 \n\
 \n\
/* sphingolipid rules */ \n\
sl: lsl | dsl | sphingoxine; \n\
lsl: hg_lslc round_open_bracket lcb round_close_bracket | hg_lslc lcb; \n\
dsl: hg_dslc dsl_species | hg_dslc dsl_subspecies; \n\
dsl_species: round_open_bracket lcb round_close_bracket | lcb; \n\
dsl_subspecies: round_open_bracket lcb_fa_sorted round_close_bracket | lcb_fa_sorted; \n\
 \n\
sphingoxine: sphingoxine_pure | sphingoxine_var; \n\
sphingoxine_pure: sphingosine_name | sphinganine_name; \n\
sphingoxine_var: ctype headgroup_separator sphingosine_name | ctype headgroup_separator sphinganine_name; \n\
sphingosine_name: 'Sphingosine' | 'So' | 'Sphingosine-1-phosphate'; \n\
sphinganine_name: 'Sphinganine' | 'Sa' | 'Sphinganine-1-phosphate'; \n\
ctype: 'C' number; \n\
 \n\
hg_dslc: hg_dsl | hg_dsl headgroup_separator; \n\
hg_dsl: 'Cer' | 'CerP' | 'EPC' | 'GB3' | 'GB4' | 'GD3' | 'GM3' | 'GM4' | 'Hex3Cer' | 'Hex2Cer' | 'HexCer' | 'IPC' | 'M(IP)2C' | 'MIPC' | 'SHexCer' | 'SM' | 'PE-Cer' | 'PI-Cer' | 'GlcCer' | 'FMC-5' | 'FMC-6' | 'LacCer' | 'GalCer' | '(3\\'-sulfo)Galbeta-Cer'; \n\
 \n\
hg_lslc: hg_lsl | hg_lsl headgroup_separator; \n\
hg_lsl: 'SPH' | 'S1P' | 'HexSph' | 'SPC' | 'SPH-P'; \n\
 \n\
 \n\
 \n\
/* cholesterol lipids */ \n\
cholesterol: chc | chec; \n\
chc: ch | ch headgroup_separator; \n\
ch: 'Cholesterol'; \n\
chec: che | che headgroup_separator | che_fa; \n\
che: fa headgroup_separator hg_che; \n\
che_fa: hg_che round_open_bracket fa round_close_bracket; \n\
hg_che: 'Cholesteryl ester' | 'Cholesterol ester' | 'CE'; \n\
 \n\
 \n\
/* mediator lipids */ \n\
mediator: mediator_var | mediator_const; \n\
mediator_var: mediator_prefix mediator_name_separator mediator_var_names | '(+/-)-' mediator_prefix mediator_name_separator mediator_var_names; \n\
mediator_prefix: mediator_numbers | mediator_prefix mediator_prefix; \n\
mediator_numbers: mediator_numbers mediator_separator mediator_numbers | mediator_number_pure | mediator_number_pure mediator_pos; \n\
mediator_number_pure: number | round_open_bracket number round_close_bracket; \n\
mediator_pos: 'R' | 'S'; \n\
mediator_var_names: mediator_var_name | mediator_oxo '-' mediator_var_name; \n\
mediator_var_name: 'HHTrE' | 'EpOME' | 'HODE' | 'HOTrE' | 'DHET' | 'EET' | 'EpETE' | 'HEPE' | 'HETE' | 'PGJ2' | 'HDoHE' | 'HpETE' | 'ETE' | 'DiHETE' | 'LXA4'; \n\
mediator_const: 'Arachidonic acid' | 'Arachidonic Acid' | 'alpha-LA' | 'DHA' | 'EPA' | 'Linoleic acid' | 'LTB4' | 'LTC4' | 'LTD4' | 'Maresin 1' | 'Palmitic acid' | 'PGB2' | 'PGD2' | 'PGE2' | 'PGF2alpha' | 'PGI2' | 'Resolvin D1' | 'Resolvin D2' | 'Resolvin D3' | 'Resolvin D5' | 'tetranor-12-HETE' | 'TXB1' | 'TXB2' | 'TXB3'; \n\
mediator_oxo: 'Oxo' | 'oxo'; \n\
 \n\
 \n\
 \n\
/* generic rules */ \n\
fa: fa_unmod | fa_unmod fa_mod; \n\
fa_unmod: ether fa_pure | fa_pure; \n\
fa_mod: round_open_bracket modification round_close_bracket; \n\
modification: modification ',' modification | number mod_text | mod_text; \n\
mod_text: mod_text mod_text | square_open_bracket mod_text square_close_bracket | 'OH' | 'Ke' | 'S' | 'OOH' | 'My' | 'Me' | 'R' | 'Br' | 'CHO' | 'COOH' | 'Cp' | '(R)'; \n\
ether: 'P-' | 'O-'; \n\
fa_pure: carbon carbon_db_separator db | carbon carbon_db_separator db db_hydroxyl_separator hydroxyl; \n\
lcb_fa: carbon carbon_db_separator db; \n\
lcb: hydroxyl_lcb lcb_fa | lcb_fa; \n\
carbon: number; \n\
db: db_count | db_count round_open_bracket db_position round_close_bracket; \n\
db_count: number; \n\
db_position: number | number cistrans | db_position db_position_separator db_position; \n\
cistrans: 'E' | 'Z'; \n\
hydroxyl: number; \n\
hydroxyl_lcb: 'm' | 'd' | 't'; \n\
number:  '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | number number; \n\
 \n\
/* separators */ \n\
SPACE : ' '; \n\
COLON : ':'; \n\
SEMICOLON : ';'; \n\
DASH : '-'; \n\
UNDERSCORE : '_'; \n\
SLASH : '/'; \n\
BACKSLASH : '\\\\'; \n\
COMMA: ','; \n\
ROB: '('; \n\
RCB: ')'; \n\
SOB: '['; \n\
SCB: ']'; \n\
 \n\
fa_separator: UNDERSCORE | SLASH | BACKSLASH | DASH; \n\
headgroup_separator: SPACE; \n\
carbon_db_separator: COLON; \n\
db_hydroxyl_separator: SEMICOLON; \n\
db_position_separator: COMMA; \n\
mediator_separator: COMMA; \n\
mediator_name_separator: DASH; \n\
round_open_bracket: ROB; \n\
round_close_bracket: RCB; \n\
square_open_bracket: SOB; \n\
square_close_bracket: SCB; \n\
 \n\
";



static const string swiss_lipids_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de \n\
 *                    Bing Peng   -   bing.peng {at} isas.de \n\
 *                    Nils Hoffmann  -  nils.hoffmann {at} isas.de \n\
 * \n\
 * Permission is hereby granted, free of charge, to any person obtaining a copy \n\
 * of this software and associated documentation files (the 'Software'), to deal \n\
 * in the Software without restriction, including without limitation the rights \n\
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell \n\
 * copies of the Software, and to permit persons to whom the Software is \n\
 * furnished to do so, subject to the following conditions:; \n\
 *  \n\
 * The above copyright notice and this permission notice shall be included in all \n\
 * copies or substantial portions of the Software. \n\
 *  \n\
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n\
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \n\
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE \n\
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER \n\
 * LIABILITY, WHether IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \n\
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE \n\
 * SOFTWARE. \n\
*/ \n\
 \n\
 \n\
 \n\
grammar SwissLipids; \n\
 \n\
 \n\
/* first rule is always start rule */ \n\
lipid : lipid_pure EOF; \n\
lipid_pure : fatty_acid | gl | pl | sl | st; \n\
 \n\
 \n\
 \n\
 \n\
/* fatty acyl rules */ \n\
fa : carbon carbon_db_separator db | ether carbon carbon_db_separator db; \n\
lcb : hydroxyl carbon carbon_db_separator db; \n\
carbon : number; \n\
db : number; \n\
ether : 'O-' | 'P-'; \n\
hydroxyl : 'm' | 'd' | 't'; \n\
 \n\
 \n\
fa2 : fa sorted_fa_separator fa; \n\
fa3 : fa sorted_fa_separator fa sorted_fa_separator fa; \n\
fa4 : fa sorted_fa_separator fa sorted_fa_separator fa sorted_fa_separator fa; \n\
fa_species : fa; \n\
 \n\
 \n\
 \n\
/* fatty acid rules */ \n\
fatty_acid : fa_hg fa_fa; \n\
fa_hg : fa_hg_pure | fa_hg_pure headgroup_separator; \n\
fa_hg_pure : 'FA' | 'fatty acid'  | 'fatty alcohol'; \n\
fa_fa : ROB fa RCB; \n\
 \n\
 \n\
 \n\
/* glycerolipid rules */ \n\
gl : gl_regular | gl_mono; \n\
 \n\
gl_regular : gl_hg gl_fa; \n\
gl_fa : ROB fa_species RCB | ROB fa3 RCB; \n\
gl_hg : 'MG' | 'DG' | 'TG'; \n\
 \n\
gl_mono : gl_mono_hg gl_mono_fa; \n\
gl_mono_fa : ROB fa_species RCB | ROB fa2 RCB; \n\
gl_mono_hg : gl_mono_hg_pure | gl_mono_hg_pure headgroup_separator; \n\
gl_mono_hg_pure : 'MHDG' | 'DHDG'; \n\
 \n\
 \n\
 \n\
 \n\
/* phospholipid rules */ \n\
pl : pl_regular | pl_four; \n\
 \n\
pl_regular : pl_hg pl_fa; \n\
pl_fa : ROB fa_species RCB | ROB fa2 RCB; \n\
pl_hg : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | 'PA' | 'PC' | 'PE' | 'PG' | 'PI' | 'PS' | 'PGP' | 'PIP2' | 'PIP' | 'PIP3'; \n\
 \n\
pl_four : pl_four_hg pl_four_fa; \n\
pl_four_fa : ROB fa_species RCB | ROB fa2 RCB | ROB fa4 RCB; \n\
pl_four_hg : pl_four_hg_pure | pl_four_hg_pure headgroup_separator; \n\
pl_four_hg_pure : 'BMP' | 'LBPA' | 'Lysobisphosphatidate'; \n\
 \n\
 \n\
 \n\
/* sphingolipid rules */ \n\
sl : sl_hg sl_lcb; \n\
sl_hg : 'HexCer' | 'Hex2Cer' | 'SM' | 'PE-Cer' | 'Cer' | 'CerP' | 'SulfoHexCer' | 'SulfoHex2Cer' | 'Gb3' | 'GA2' | 'GA1' | 'GM3' | 'GM2' | 'GM1' | 'GD3' | 'GT3' | 'GD1' | 'GT1' | 'GQ1' | 'GM4' | 'GD2' | 'GT2' | 'GP1'; \n\
sl_lcb : ROB lcb RCB; \n\
 \n\
 \n\
 \n\
 \n\
/* sterol rules */ \n\
st : st_hg st_fa; \n\
st_hg : 'SE'; \n\
st_fa : ROB fa RCB; \n\
 \n\
 \n\
 \n\
 \n\
/* separators */ \n\
SPACE : ' '; \n\
COLON : ':'; \n\
SEMICOLON : ';'; \n\
DASH : '-'; \n\
UNDERSCORE : '_'; \n\
SLASH : '/'; \n\
BACKSLASH : '\\\\'; \n\
COMMA: ','; \n\
ROB: '('; \n\
RCB: ')'; \n\
 \n\
sorted_fa_separator : SLASH; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
number :  digit; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | digit digit; \n\
";



#endif /* KNOWN_GRAMMARS_H */
