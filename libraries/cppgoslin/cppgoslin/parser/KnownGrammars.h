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



#ifndef KNOWN_GRAMMARS_H
#define KNOWN_GRAMMARS_H

#include <string>
using namespace std;
const string shorthand_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) the authors (listed in global LICENSE file) \n\
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
 * Liebisch et al. Volume 61, Issue 12, December 2020, Pages 1539-1555. \n\
 */ \n\
 \n\
grammar Shorthand2020; \n\
 \n\
 \n\
/* first rule is always start rule */ \n\
lipid : lipid_eof EOF; \n\
lipid_eof : lipid_pure | lipid_pure adduct_info; \n\
lipid_pure : gl | pl | sl | sterol | med; /* glycero lipids, phospho lipids, sphingo lipids, sterol lipids, lipid mediators \n\
 \n\
 \n\
/* adduct information */ \n\
adduct_info : adduct_sep | adduct_separator adduct_sep; \n\
adduct_sep : '[M' adduct ']' charge_sign | '[M' adduct ']' charge charge_sign; \n\
adduct : adduct_set; \n\
adduct_set : adduct_element | adduct_element adduct_set; \n\
adduct_element : element | element number | number element | plus_minus element | plus_minus element number | plus_minus number element; \n\
 \n\
 \n\
/* mediators */ \n\
med : med_species | med_subspecies; \n\
med_species : med_hg_double headgroup_separator fatty_acyl_chain | med_hg_triple headgroup_separator fatty_acyl_chain; \n\
med_subspecies : med_hg_single headgroup_separator fatty_acyl_chain | \n\
                 med_hg_double headgroup_separator fatty_acyl_chain2 | \n\
                 med_hg_triple headgroup_separator fatty_acyl_chain3; \n\
med_hg_single : 'FA' | 'FOH' | 'FAL' | 'CAR' | 'CoA' | 'NAE' | 'NAT' | 'WD' | 'HC' | 'FAHFA'; \n\
med_hg_double : 'WE' | 'NA'; \n\
med_hg_triple : 'WD'; \n\
 \n\
 \n\
 \n\
 \n\
/* fatty acyl chain */ \n\
lcb : fatty_acyl_chain; \n\
fatty_acyl_chain : fatty_acyl | fatty_acyl; \n\
fatty_acyl : fa_pure | ether fa_pure | fatty_acyl_linkage | fatty_alkyl_linkage; \n\
fatty_alkyl_linkage : fatty_linkage_number fatty_acyl_linkage_sign ROB fatty_acyl_chain RCB | fatty_acyl_linkage_sign ROB fatty_acyl_chain RCB; \n\
fatty_acyl_linkage : fatty_linkage_number fatty_acyl_linkage_sign ROB med RCB | fatty_acyl_linkage_sign ROB med RCB; \n\
hydrocarbon_chain : hydrocarbon_number ROB fatty_acyl_chain RCB | ROB fatty_acyl_chain RCB; \n\
fatty_acyl_linkage_sign : 'O' | 'N'; \n\
fatty_linkage_number : number; \n\
hydrocarbon_number : number; \n\
fa_pure : fa_pure_structure | fa_pure_structure sn; \n\
sn : ROB 'sn-' sn_pos RCB; \n\
sn_pos : number; \n\
fa_pure_structure : carbon carbon_db_separator db  | carbon carbon_db_separator db db_funcgroup_separator func_group | carbon carbon_db_separator db stereo_fa | carbon carbon_db_separator db stereo_fa db_funcgroup_separator func_group; \n\
ether : ether_num ether_type | ether_type; \n\
ether_num : 'm' | 'd' | 't' | 'e'; \n\
ether_type: ether_types plasmalogen_separator | ether_types plasmalogen_separator; \n\
ether_types: 'O' | 'P'; \n\
carbon : number; \n\
db : db_count | db_count db_positions; \n\
db_count : number; \n\
db_positions : ROB db_position RCB; \n\
db_position : db_single_position | db_position db_position_separator db_position; \n\
db_single_position : db_position_number | db_position_number cistrans; \n\
db_position_number : number; \n\
cistrans : 'E' | 'Z'; \n\
stereo_fa : SOB stereo_type_fa SCB; \n\
stereo_type_fa : stereo_number stereo_direction | stereo_direction; \n\
 \n\
func_group : func_group_entity; \n\
func_group_entity : func_group_entity funcgroup_separator func_group_entity | func_group_data | func_group_data_repetition; \n\
func_group_data_repetition : func_group_data_repetition func_repetition_separator func_group_data_repetition | func_group_data; \n\
func_group_data : func_group_name | func_group_cycle | func_group_pos func_group_ext_name | func_group_pos func_group_ext_name stereo_fg | func_group_ext_count_name func_group_count | func_group_ext_name func_group_count stereo_fg | molecular_func_group | fatty_acyl_linkage | fatty_alkyl_linkage | hydrocarbon_chain; \n\
func_group_pos : func_group_pos_number | func_group_pos_number ring_stereo; \n\
ring_stereo : 'a' | 'b'; \n\
func_group_pos_number : number; \n\
func_group_count : number; \n\
stereo_fg : SOB stereo_type_fg SCB; \n\
stereo_type_fg : stereo_number stereo_direction | stereo_direction; \n\
stereo_number : number; \n\
stereo_direction : 'R' | 'S'; \n\
molecular_func_group : molecular_func_group_name | molecular_func_group_name func_group_count; \n\
func_group_ext_name : round_open_bracket func_group_name round_close_bracket | func_group_name; \n\
func_group_ext_count_name : round_open_bracket func_group_name round_close_bracket | molecular_func_group_name; \n\
func_group_name : 'Et' | 'Me' | 'Ac' | 'NO2' | 'My' | 'Ep' | 'OO' | 'dMe' | 'OMe' | 'oxy' | 'NH2' | 'OOH' | 'SH' | 'OH' | 'oxo' | 'CN' | 'Ph' | 'Su' | 'COOH' | 'G' | 'T' | 'COG' | 'COT' | carbohydrate | 'H' | 'Cys' | 'Phe' | 'SGlu' | 'SCys' | 'BOO' | 'MMAs' | 'SMe' | 'NH' | 'SCG' | special_elements; \n\
molecular_func_group_name : elements | special_elements; \n\
elements : 'O' | 'N' | 'P' | 'S' | 'As'; \n\
special_elements: 'Br' | 'Cl' | 'F' | 'I'; \n\
func_group_cycle : SOB cycle_base SCB | SOB cycle_base funcgroup_separator cycle_func_group_data SCB; \n\
cycle_base : cycle_def | cycle_def carbon_db_separator cycle_db; \n\
cycle_def : cycle_start cycle_separator cycle_end cycle_token cycle_number | cycle_start cycle_separator cycle_end cycle_bridge cycle_token cycle_number | cycle_bridge cycle_token cycle_number | cycle_token cycle_number; \n\
cycle_bridge : cylce_elements; \n\
cylce_elements : cylce_elements cylce_elements | cylce_element; \n\
cylce_element : elements; \n\
cycle_token : 'cy'; \n\
cycle_number : number; \n\
cycle_start : number; \n\
cycle_end : number; \n\
cycle_func_group_data : func_group_entity; \n\
cycle_db : cycle_db_cnt | cycle_db_cnt ROB cycle_db_positions RCB; \n\
cycle_db_cnt : number; \n\
cycle_db_positions : cycle_db_position; \n\
cycle_db_position : cycle_db_position db_position_separator cycle_db_position | cycle_db_position_number | cycle_db_position_number cycle_db_position_cis_trans; \n\
cycle_db_position_number : number; \n\
cycle_db_position_cis_trans : cistrans; \n\
 \n\
fatty_acyl_chain2 : fa2_sorted | fa2_unsorted; \n\
fa2_unsorted : fatty_acyl_chain unsorted_fa_separator fatty_acyl_chain; \n\
fa2_sorted : fatty_acyl_chain sorted_fa_separator fatty_acyl_chain; \n\
 \n\
fatty_acyl_chain3 : f3_sorted | fa3_unsorted; \n\
f3_sorted : fa2_sorted sorted_fa_separator fatty_acyl_chain; \n\
fa3_unsorted : fa2_unsorted unsorted_fa_separator fatty_acyl_chain; \n\
 \n\
fatty_acyl_chain4 : f4_sorted | fa4_unsorted; \n\
f4_sorted : fa2_sorted sorted_fa_separator fa2_sorted; \n\
fa4_unsorted : fa2_unsorted unsorted_fa_separator fa2_unsorted; \n\
 \n\
 \n\
/* glycero lipids */ \n\
gl : gl_species | gl_subpsecies | gl_molecular_species; \n\
gl_species : gl_hg_dt headgroup_separator fatty_acyl_chain; \n\
gl_molecular_species : gl_hg_double headgroup_separator fa2_unsorted | gl_hg_triple headgroup_separator fa2_unsorted | gl_hg_single headgroup_separator fatty_acyl_chain; \n\
gl_subpsecies : gl_hg headgroup_separator fatty_acyl_chain3; \n\
gl_hg_dt : gl_hg_double | gl_hg_true_double | gl_hg_triple; \n\
gl_hg : gl_hg_single | gl_hg_double | gl_hg_true_double | gl_hg_triple; \n\
gl_hg_single : 'MG' | 'MGMG' | 'DGMG' | 'SQMG'; \n\
gl_hg_double : 'DG'; \n\
gl_hg_true_double : 'MGDG' | 'DGDG' | 'SQDG'; \n\
gl_hg_triple : 'TG'; \n\
 \n\
 \n\
 \n\
pl : pl_species | pl_subspecies | pl_molecular_species; \n\
pl_species : pl_hg headgroup_separator fatty_acyl_chain; \n\
pl_subspecies : pl_single | pl_double | pl_quadro; \n\
pl_molecular_species : pl_hg_quadro headgroup_separator fa2_unsorted | pl_hg_quadro headgroup_separator fa3_unsorted; \n\
pl_single : pl_hg_single headgroup_separator fatty_acyl_chain; \n\
pl_full : pl_hg_single headgroup_separator fatty_acyl_chain2; \n\
pl_double : pl_full | pl_hg_double_all headgroup_separator fatty_acyl_chain2; \n\
pl_quadro : pl_hg_quadro headgroup_separator fatty_acyl_chain4; \n\
pl_hg : pl_hg_double_all | pl_hg_quadro; \n\
pl_hg_single : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | hg_lpim | 'CPA' | 'LCDPDAG' | 'LDMPE' | 'LMMPE' | 'LPIMIP' | 'LPIN' | 'PE-isoLG'; \n\
pl_hg_double_all : pl_hg_double_fa | pl_hg_double | hg_pip; \n\
pl_hg_double_fa : pl_hg_double_fa_hg ROB pl_hg_fa RCB | pl_hg_double_fa_hg ROB pl_hg_alk RCB; \n\
pl_hg_double_fa_hg : 'PS-N' | 'PE-N'; \n\
pl_hg_double : 'CDP-DAG' | 'DMPE' | 'MMPE' | 'PA' | 'PC' | 'PE' | 'PEt' | 'PG' | 'PI' | 'PS' | 'LBPA' | 'PGP' | 'PPA' | 'Glc-GP' | '6-Ac-Glc-GP' | hg_pim | 'PnC' | 'PnE' | 'PT' | 'PE-NMe2' | 'PE-NMe' | 'PIMIP' | 'CDPDAG' | 'PS-CAP' | 'PS-MDA' | 'PE-CAP' | 'PE-Glc' | 'PE-GlcA' | 'PE-GlcK' | 'PE-CM' | 'PE-CE' | 'PE-FA' | 'PE-CA' | 'PE-MDA' | 'PE-HNE' | pl_hg_species; \n\
pl_hg_species : hg_PE_PS ROB hg_PE_PS_type RCB; \n\
hg_PE_PS : 'PE-N' | 'PS-N'; \n\
hg_PE_PS_type : 'Alk' | 'FA';  \n\
pl_hg_quadro : 'BMP' | 'CL' | 'LCL' | 'DLCL'; \n\
hg_pip : hg_pip_pure_m | hg_pip_pure_d | hg_pip_pure_t | hg_pip_pure_m ROB hg_pip_m RCB | hg_pip_pure_d ROB hg_pip_d RCB | hg_pip_pure_t ROB hg_pip_t RCB; \n\
hg_pip_pure_m : 'PIP'; \n\
hg_pip_pure_d : 'PIP2'; \n\
hg_pip_pure_t : 'PIP3'; \n\
hg_pip_m : '3' APOSTROPH | '4' APOSTROPH | '5' APOSTROPH; \n\
hg_pip_d : '3' APOSTROPH COMMA '4' APOSTROPH | '4' APOSTROPH COMMA '5' APOSTROPH | '3' APOSTROPH COMMA '5' APOSTROPH; \n\
hg_pip_t : '3' APOSTROPH COMMA '4' APOSTROPH COMMA '5' APOSTROPH; \n\
hg_pim : 'PIM' hg_pim_number; \n\
hg_pim_number : number; \n\
hg_lpim : 'LPIM' hg_lpim_number; \n\
hg_lpim_number : number; \n\
pl_hg_fa : med; \n\
pl_hg_alk : fatty_acyl_chain; \n\
 \n\
 \n\
carbohydrate : 'Hex' | 'Gal' | 'Glc' | 'Man' | 'Neu' | 'HexNAc' | 'GalNAc' | 'GlcNAc' | 'NeuAc' | 'NeuGc' | 'Kdn' | 'GlcA' | 'Xyl' | 'Fuc' | 'NeuAc2' | 'SHex' | 'S' ROB '3' APOSTROPH RCB 'Hex' | 'NAc' | 'Nac' | 'SGal' | 'S' ROB '3' APOSTROPH RCB 'Gal' | 'HexA' | 'OGlcNAc' | 'OGlc'; \n\
 \n\
 \n\
sl : sl_species | sl_subspecies; \n\
sl_species : sl_hg_double headgroup_separator lcb | acer_species headgroup_separator lcb; \n\
sl_subspecies : sl_hg_single headgroup_separator lcb | sl_hg_single sl_hydroxyl headgroup_separator lcb | sl_double; \n\
sl_double : sl_hg_double headgroup_separator lcb sorted_fa_separator fatty_acyl_chain | sl_hg_double sl_hydroxyl headgroup_separator lcb sorted_fa_separator fatty_acyl_chain; \n\
sl_hydroxyl : ROB sl_hydroxyl_number RCB; \n\
sl_hydroxyl_number : number; \n\
sl_hg_single : 'SPB' | 'SPBP' | 'LIPC' | 'LSM'; \n\
sl_hg_double : acer_hg | sl_hg_double_name | carbohydrate_structural sl_hg_double  | carbohydrate_isomeric sl_hg_double; \n\
carbohydrate_structural : carbohydrate; \n\
carbohydrate_isomeric : carbohydrate carbohydrate_separator; \n\
sl_hg_double_name : 'SM' | 'Cer' | 'CerP' | acer_hg | 'HexCer' | 'GlcCer' | 'GalCer' | 'Hex2Cer' | 'LacCer' | 'SHexCer' | 'IPC' | 'PI-Cer' | 'EPC' | 'PE-Cer' | 'GIPC' | 'MIPC' | 'M(IP)2C' | 'Hex3Cer' | 'S' ROB '3' APOSTROPH RCB 'HexCer' | 'S' ROB '3' APOSTROPH RCB 'GalCer'; \n\
acer_hg : acer_hg_pure ROB med RCB; \n\
acer_hg_pure : 'ACer'; \n\
acer_species : 'ACer(FA)'; \n\
/* \n\
acer_hg : acer_hg_pure | acer_med '-' acer_hg_pure; \n\
acer_med : med; \n\
*/ \n\
 \n\
 \n\
 \n\
/* sterol lipids */ \n\
sterol : st | st_ester; \n\
st : st_hg headgroup_separator sterol_definition; \n\
st_ester : st_hg_ester headgroup_separator sterol_definition sorted_fa_separator fatty_acyl_chain | st_hg_ester headgroup_separator fatty_acyl_chain; \n\
sterol_definition : fatty_acyl_chain; \n\
st_hg : 'ST' | 'BA' | 'FC' | 'SG' | 'ASG'; \n\
st_hg_ester : 'SE' | 'CE'; \n\
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
SOB: '['; \n\
SCB: ']'; \n\
 \n\
APOSTROPH : '\\'' | '′'; \n\
sorted_fa_separator : SLASH; \n\
adduct_separator : SPACE; \n\
unsorted_fa_separator : UNDERSCORE; \n\
plasmalogen_separator : DASH; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
cycle_separator: DASH; \n\
db_funcgroup_separator : SEMICOLON; \n\
db_position_separator : COMMA; \n\
carbohydrate_separator : DASH; \n\
funcgroup_separator : SEMICOLON; \n\
func_repetition_separator : COMMA; \n\
round_open_bracket : ROB; \n\
round_close_bracket : RCB; \n\
 \n\
number :  digit | digit number; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
 \n\
element: 'C' | 'H' | 'N' | 'O' | 'P' | 'S' | 'Br' | 'I' | 'F' | 'Cl' | 'As'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : plus_minus; \n\
plus_minus : '-' | '+'; \n\
";



const string goslin_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) the authors (listed in global LICENSE file) \n\
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
grammar Goslin; \n\
 \n\
 \n\
/* first rule is always start rule */ \n\
lipid : lipid_eof EOF; \n\
lipid_eof : lipid_pure | lipid_pure adduct_info; \n\
lipid_pure : gl | pl | sl | sterol | mediatorc | saccharolipid; \n\
 \n\
 \n\
/* adduct information */ \n\
adduct_info : adduct_sep | adduct_separator adduct_sep; \n\
adduct_sep : '[M' adduct ']' charge_sign | '[M' adduct ']' charge charge_sign; \n\
adduct : adduct_set; \n\
adduct_set : adduct_element | adduct_element adduct_set; \n\
adduct_element : element | element number | number element | plus_minus element | plus_minus element number | plus_minus number element; \n\
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
hg_mgl : 'MAG' | 'MG'; \n\
hg_dgl : 'DAG' | 'DG'; \n\
hg_sgl : 'MGDG' | 'DGDG' | 'SQDG'; \n\
hg_tgl : 'TAG' | 'TG'; \n\
 \n\
 \n\
 \n\
/* phospholipid rules (56 classes) */ \n\
pl : lpl | dpl | tpl | cl | mlcl | dlcl; \n\
lpl : hg_lplc headgroup_separator fa; \n\
dpl : hg_plc headgroup_separator pl_species | hg_plc headgroup_separator pl_subspecies; \n\
tpl : hg_tplc headgroup_separator pl_species | hg_tplc headgroup_separator tpl_subspecies; \n\
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
hg_cl : 'CL' | 'SLBPA' | 'MLCL' | 'BMP'; \n\
hg_mlclc : hg_mlcl | hg_mlcl heavy_hg | hg_mlcl plasmalogen_hg | hg_mlcl heavy_hg plasmalogen_hg; \n\
hg_mlcl : 'MLCL'; \n\
hg_dlclc : hg_dlcl | hg_dlcl heavy_hg | hg_dlcl plasmalogen_hg | hg_dlcl heavy_hg plasmalogen_hg; \n\
hg_dlcl : 'DLCL'; \n\
hg_plc : hg_pl | hg_pl heavy_hg | hg_pl plasmalogen_hg | hg_pl heavy_hg plasmalogen_hg; \n\
hg_pl : 'BMP' | 'CDP-DAG' | 'DMPE' | 'MMPE' | 'PA' | 'PC' | 'PE' | 'PEt' | 'PG' | 'PI' | hg_pip | 'PS' | 'LBPA' | 'PGP' | 'PPA' | 'Glc-GP' | '6-Ac-Glc-GP' | hg_pim | 'PnC' | 'PnE' | 'PT' | 'PE-NMe2' | 'PE-NMe' | 'PIMIP' | 'CDPDAG'; \n\
hg_pim : 'PIM' hg_pim_number; \n\
hg_pim_number : number; \n\
hg_pip : hg_pip_pure | hg_pip_pure hg_pip_m | hg_pip_pure hg_pip_d | hg_pip_pure hg_pip_t; \n\
hg_pip_pure : 'PIP'; \n\
hg_pip_m : '[3\\']' | '[4\\']' | '[5\\']'; \n\
hg_pip_d : '2' | '2[3\\',4\\']' | '2[4\\',5\\']' | '2[3\\',5\\']'; \n\
hg_pip_t : '3' | '3[3\\',4\\',5\\']'; \n\
hg_tplc : hg_tpl | hg_tpl heavy_hg; \n\
hg_tpl : slbpa | 'NAPE'; \n\
slbpa : 'SLBPA'; \n\
hg_lplc : hg_lpl | hg_lpl heavy_hg | hg_lpl plasmalogen_hg | hg_lpl heavy_hg plasmalogen_hg; \n\
hg_lpl : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | hg_lpim | 'CPA' | 'LCDPDAG' | 'LDMPE' | 'LMMPE' | 'LPIMIP' | 'LPIN'; \n\
hg_lpim : 'LPIM' hg_lpim_number; \n\
hg_lpim_number : number; \n\
 \n\
plasmalogen_hg : plasmalogen_separator plasmalogen plasmalogen_separator | plasmalogen_separator plasmalogen | plasmalogen plasmalogen_separator; \n\
plasmalogen : 'O' | 'o' | 'P' | 'p'; \n\
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
hg_lsl : 'LCB' | 'LCBP' | 'LHexCer' | 'LSM' | 'LIPC' | 'So' | 'Sa' | 'SPH' | 'Sph' | 'LCB' | 'S1P' | 'SPH-P' | 'SIP' | 'Sa1P'; \n\
hg_dslc : hg_dsl | hg_dsl heavy_hg; \n\
hg_dsl : 'Cer' | 'CerP' | 'EPC' | 'GB4' | 'GD3' | 'GB3' | 'GM1' | 'GM3' | 'GM4' | 'Hex3Cer' | 'Hex2Cer' | 'HexCer' | 'IPC' | 'MIP2C' | 'M(IP)2C' | 'MIPC' | 'SHexCer' | 'SM' | 'FMC-5' | 'FMC-6' | 'CerPE' | 'PE-Cer'; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* sterol lipids (2 classes) */ \n\
sterol : stc | ste | stes; \n\
stc : st | st heavy_hg; \n\
st : 'Ch' | 'Cholesterol' | 'ST 27:1;1' | 'ST 27:2;1' | 'ST 28:3;1' | 'ST 30:2;1' | 'ST 29:2;1' | 'ST 28:2;1' | 'Desmosterol' | 'Stigmasterol' | 'Ergosterol' | 'Lanosterol' | 'Ergostadienol' | 'Campesterol' | 'Sitosterol' | 'Chol'; \n\
ste : hg_stc sorted_fa_separator fa | hg_stc unsorted_fa_separator fa; \n\
stes : hg_stcs headgroup_separator fa; \n\
hg_stc : hg_ste | hg_ste heavy_hg; \n\
hg_ste : 'SE 27:1' | 'SE 27:2' | 'SE 28:3' | 'SE 30:2' | 'SE 29:2' | 'SE 28:2'; \n\
hg_stcs : hg_stes | hg_stes heavy_hg; \n\
hg_stes : 'ChE' | 'CE' | 'ChoE'; \n\
 \n\
 \n\
/* mediator lipids (1 class) */ \n\
mediatorc : mediator | mediator heavy_hg; \n\
mediator : unstructured_mediator | trivial_mediator | mediator_functional_group mediator_fa mediator_suffix | mediator_functional_group mediator_fa; \n\
mediator_fa : mediator_carbon mediator_db; \n\
mediator_carbon : 'H' | 'O' | 'E' | 'Do'; \n\
mediator_db : 'M' | 'D' | 'Tr' | 'T' | 'P' | 'H'; \n\
mediator_suffix: 'E'; \n\
mediator_functional_group : mediator_functional_group_clear | mediator_tetranor mediator_functional_group_clear; \n\
mediator_tetranor : 'tetranor-'; \n\
mediator_functional_group_clear: mediator_full_function | mediator_function_unknown_pos; \n\
mediator_function_unknown_pos : mediator_functions; \n\
mediator_functions : mediator_mono_functions | mediator_di_functions; \n\
mediator_mono_functions: 'H' | 'Oxo'; \n\
mediator_di_functions: 'E' | 'Ep' | 'DH' | 'DiH'; \n\
mediator_mono_pos: mediator_position; \n\
mediator_di_pos: mediator_position ',' mediator_position | mediator_position '_' mediator_position | mediator_position '(' mediator_position ')'; \n\
mediator_full_function : mediator_mono_pos '-' mediator_mono_functions | mediator_di_pos '-' mediator_di_functions; \n\
mediator_position : number; \n\
 \n\
trivial_mediator : 'AA' | 'ALA' | 'DHA' | 'EPA' | 'Linoleic acid' | 'TXB1' | 'TXB2' | 'TXB3' | 'Resolvin D1' | 'Resolvin D2' | 'Resolvin D3' | 'Resolvin D5' | 'LTB4' | 'Maresin 1' | 'Palmitic acid' | 'PGB2' | 'PGD2' | 'PGE2' | 'PGF2alpha'; \n\
 \n\
unstructured_mediator : 'alpha-LA' | 'LTC4' | 'LTD4' | 'PGI2'; \n\
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
fa : fa_ester | fa_ether; \n\
fa_ester : fa_pure | fa_pure heavy_fa; \n\
fa_ether : plasmalogen_hg fa_pure | plasmalogen_hg fa_pure ether | fa_pure ether | plasmalogen_hg fa_pure heavy_fa | plasmalogen_hg fa_pure ether heavy_fa | fa_pure ether heavy_fa; \n\
heavy_fa : heavy; \n\
fa_pure : carbon carbon_db_separator db | carbon carbon_db_separator db db_hydroxyl_separator hydroxyl; \n\
ether : 'a' | 'p'; \n\
lcb : lcb_pure | lcb_pure heavy_lcb; \n\
heavy_lcb : heavy; \n\
lcb_pure : carbon carbon_db_separator db db_hydroxyl_separator hydroxyl | old_hydroxyl carbon carbon_db_separator db | old_hydroxyl DASH carbon carbon_db_separator db; \n\
carbon : number; \n\
db : db_count | db_count db_positions; \n\
db_count : number; \n\
db_positions : ROB db_position RCB; \n\
db_position : db_single_position | db_position db_position_separator db_position; \n\
db_single_position : db_position_number | db_position_number cistrans; \n\
db_position_number : number; \n\
cistrans : 'E' | 'Z'; \n\
 \n\
hydroxyl : number; \n\
old_hydroxyl : 'd' | 't'; \n\
number :  digit | digit number; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
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
plasmalogen_separator : headgroup_separator | DASH | DASH SPACE; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
db_hydroxyl_separator : SEMICOLON; \n\
db_position_separator : COMMA; \n\
round_open_bracket : ROB; \n\
round_close_bracket : RCB; \n\
 \n\
element: 'C' | 'H' | 'N' | 'O' | 'P' | 'S' | 'Br' | 'I' | 'F' | 'Cl' | 'As'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : plus_minus; \n\
plus_minus : '-' | '+'; \n\
";



const string lipid_maps_grammar = "//////////////////////////////////////////////////////////////////////////////// \n\
// MIT License \n\
//  \n\
// Copyright (c) the authors (listed in global LICENSE file) \n\
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
lipid : lipid_rule EOF | lipid_rule adduct_info EOF; \n\
lipid_rule: lipid_mono | lipid_mono isotope; \n\
lipid_mono: lipid_pure | lipid_pure isoform; \n\
lipid_pure: pure_fa | gl | pl | sl | pk | sterol | mediator; \n\
isoform: square_open_bracket isoform_inner square_close_bracket; \n\
isoform_inner : 'rac' | 'iso' | 'iso' number | 'R'; \n\
isotope: SPACE round_open_bracket isotope_element number round_close_bracket | DASH round_open_bracket isotope_element number round_close_bracket | DASH isotope_element number; \n\
isotope_element: 'd'; \n\
 \n\
 \n\
/* adduct information */ \n\
adduct_info : adduct_sep | adduct_separator adduct_sep; \n\
adduct_sep : '[M' adduct ']' charge_sign | '[M' adduct ']' charge charge_sign; \n\
adduct : adduct_set; \n\
adduct_set : adduct_element | adduct_element adduct_set; \n\
adduct_element : element | element number | number element | plus_minus element | plus_minus element number | plus_minus number element; \n\
 \n\
 \n\
 \n\
/* pure fatty acid */ \n\
pure_fa: hg_fa pure_fa_species | fa_no_hg; \n\
fa_no_hg: fa; \n\
pure_fa_species: round_open_bracket fa round_close_bracket | fa | round_open_bracket fa2 round_close_bracket; \n\
hg_fa: 'FA' | 'WE' | 'CoA' | 'CAR' | 'FAHFA' | 'CoA'; \n\
 \n\
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
species_fa : fa; \n\
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
hg_sgl: 'MGDG' | 'DGDG' | 'SQDG' | 'SQMG' | hg_dg | 'DGCC' | 'PE-GlcDG'; \n\
hg_dg : 'DG'; \n\
hg_glc: hg_gl | hg_gl headgroup_separator; \n\
hg_gl: 'MG' | 'DG' | 'TG'; \n\
 \n\
 \n\
 \n\
 \n\
/* phospholipid rules */ \n\
pl: lpl | dpl | cl | fourpl | threepl | cpa; \n\
lpl: hg_lplc round_open_bracket fa_lpl round_close_bracket | hg_lplc fa_lpl; \n\
cpa : hg_cpa round_open_bracket fa round_close_bracket; \n\
fa_lpl: fa_lpl_molecular | fa2; \n\
fa_lpl_molecular: fa; \n\
dpl: hg_ddpl dpl_species | hg_ddpl dpl_subspecies; \n\
dpl_species: round_open_bracket fa round_close_bracket | fa; \n\
dpl_subspecies: round_open_bracket fa2 round_close_bracket | fa2; \n\
cl: hg_clc cl_species | hg_clc cl_subspecies; \n\
cl_species: round_open_bracket fa round_close_bracket | fa; \n\
cl_subspecies: round_open_bracket '1\\'-' square_open_bracket fa2 square_close_bracket ',3\\'-' square_open_bracket fa2 square_close_bracket round_close_bracket | hg_clc '1\\'-' square_open_bracket fa2 square_close_bracket ',3\\'-' square_open_bracket fa2 square_close_bracket; \n\
fourpl: hg_fourplc round_open_bracket fa4 round_close_bracket | hg_fourplc fa4 | hg_fourplc round_open_bracket species_fa round_close_bracket | hg_fourplc species_fa; \n\
threepl: hg_threeplc round_open_bracket fa3 round_close_bracket | hg_threeplc fa3 | hg_threeplc round_open_bracket species_fa round_close_bracket | hg_threeplc species_fa; \n\
 \n\
hg_ddpl: hg_dplc pip_position | hg_dplc; \n\
 \n\
hg_clc: hg_cl | hg_cl headgroup_separator; \n\
hg_cl: 'CL'; \n\
hg_dplc: hg_dpl | hg_dpl headgroup_separator; \n\
hg_cpa: 'CPA'; \n\
hg_dpl: hg_lbpa | 'CDP-DG' | 'DMPE' | 'MMPE' | 'PA' | 'PC' | 'PE' | 'PEt' | 'PG' | 'PI' | 'PIP' | 'PIP2' | 'PIP3' | 'PS' | 'PIM1' | 'PIM2' | 'PIM3' | 'PIM4' | 'PIM5' | 'PIM6' | 'Glc-DG' | 'PGP' | 'PE-NMe2' | 'AC2SGL' | 'DAT' | 'PE-NMe' | 'PT' | 'Glc-GP' | 'PPA' | 'PnC' | 'PnE' | '6-Ac-Glc-GP' | 'GPA' | 'GPCho' | 'GPEtn' | 'GPGro' | 'GPIns' | 'GPSer' | 'GPC' | 'GPE' | 'GPG' | 'GPI' | 'GPS' | 'PlsA' | 'PlsCho' | 'PlsEtn' | 'PlsGro' | 'PlsIns' | 'PlsSer'; \n\
hg_lbpa : 'LBPA'; \n\
hg_lplc: hg_lpl | hg_lpl headgroup_separator; \n\
hg_lpl: 'lysoPC' | 'LysoPC' | 'LPC' | 'lysoPE' | 'LysoPE' | 'LPE' | 'lysoPI' | 'LysoPI' | 'LPI' | 'lysoPG' | 'LysoPG' | 'LPG' | 'lysoPS' | 'LysoPS' | 'LPS' | 'LPIM1' | 'LPIM2' | 'LPIM3' | 'LPIM4' | 'LPIM5' | 'LPIM6' | 'lysoPA' | 'LysoPA' | 'LPA'; \n\
hg_fourplc: hg_fourpl | hg_fourpl headgroup_separator; \n\
hg_fourpl: 'PAT16' | 'PAT18'; \n\
pip_position: square_open_bracket pip_pos square_close_bracket; \n\
pip_pos: pip_pos COMMA pip_pos | number '\\''; \n\
hg_threeplc: hg_threepl | hg_threepl headgroup_separator; \n\
hg_threepl: 'SLBPA' | 'PS-NAc' | 'NAPE'; \n\
 \n\
 \n\
 \n\
/* sphingolipid rules */ \n\
sl: lsl | dsl; \n\
lsl: hg_lslc round_open_bracket lcb round_close_bracket | hg_lslc lcb; \n\
dsl: hg_dslc dsl_species | hg_dslc dsl_subspecies; \n\
dsl_species: round_open_bracket lcb round_close_bracket | lcb; \n\
dsl_subspecies: round_open_bracket lcb_fa_sorted round_close_bracket | lcb_fa_sorted; \n\
 \n\
 \n\
hg_dslc: hg_dsl_global | hg_dsl_global headgroup_separator; \n\
hg_dsl_global : hg_dsl | special_cer | special_glyco; \n\
hg_dsl: 'Cer' | 'CerP' | 'EPC' | 'GB3' | 'GB4' | 'GD3' | 'GM3' | 'GM4' | 'Hex3Cer' | 'Hex2Cer' | 'HexCer' | 'IPC' | 'M(IP)2C' | 'MIPC' | 'SHexCer' | 'SulfoHexCer' | 'SM' | 'PE-Cer' | 'PI-Cer' | 'GlcCer' | 'FMC-5' | 'FMC-6' | 'LacCer' | 'GalCer' | 'C1P' | '(3\\'-sulfo)Galbeta-Cer' | omega_linoleoyloxy_Cer; \n\
omega_linoleoyloxy_Cer : 'omega-linoleoyloxy-' special_cer_hg; \n\
special_cer : special_cer_prefix '-Cer'; \n\
special_cer_hg : 'Cer'; \n\
special_cer_prefix : '1-O-' special_cer_prefix_1_O; \n\
special_glyco : glyco_cer '-' special_cer_hg; \n\
special_cer_prefix_1_O : 'myristoyl' | 'palmitoyl' | 'stearoyl' | 'eicosanoyl' | 'behenoyl' | 'lignoceroyl' | 'cerotoyl' | 'pentacosanoyl' | 'tricosanoyl' | 'carboceroyl' | 'lignoceroyl-omega-linoleoyloxy' | 'stearoyl-omega-linoleoyloxy'; \n\
glyco_cer : glyco_entity | glyco_entity '-' glyco_cer | number glyco_branch glyco_cer; \n\
glyco_branch : '(' glyco_cer '-' number ')' | '(' glyco_cer '-' number ')' glyco_branch; \n\
glyco_entity : glyco_struct | glyco_number glyco_struct | glyco_number glyco_struct greek | glyco_number glyco_struct greek number | glyco_number glyco_struct number | glyco_struct greek | glyco_struct greek number; \n\
glyco_number : number | number '-'; \n\
glyco_struct : 'Hex' | 'Gal' | 'Glc' | 'Man' | 'Neu' | 'HexNAc' | 'GalNAc' | 'GlcNAc' | 'NeuAc' | 'NeuGc' | 'Kdn' | 'GlcA' | 'Xyl' | 'Fuc' | 'KDN' | 'OAc-NeuAc'; \n\
greek : 'alpha' | 'beta' | 'α' | 'β'; \n\
 \n\
 \n\
hg_lslc: hg_lsl | hg_lsl headgroup_separator; \n\
hg_lsl: 'SPH' | 'Sph' | 'S1P' | 'HexSph' | 'SPC' | 'SPH-P' | 'LysoSM' | 'SIP'; \n\
 \n\
 \n\
 \n\
/* polyketides */ \n\
pk : pk_hg pk_fa; \n\
pk_hg : 'RESORCINOL' | 'ANACARD' | 'PHENOL' | 'CATECHOL'; \n\
pk_fa : round_open_bracket fa round_close_bracket; \n\
 \n\
 \n\
 \n\
/* sterol lipids */ \n\
sterol: chc | chec; \n\
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
fa: fa_unmod | fa_unmod fa_mod | fa_unmod fa_mod_separator fa_mod; \n\
fa_unmod: round_open_bracket ether fa_pure round_close_bracket | round_open_bracket fa_pure round_close_bracket | ether fa_pure | fa_pure; \n\
fa_mod: round_open_bracket modification round_close_bracket; \n\
modification: modification ',' modification | single_mod; \n\
single_mod : isomeric_mod | isomeric_mod square_open_bracket stereo square_close_bracket | structural_mod | structural_mod square_open_bracket stereo square_close_bracket; \n\
isomeric_mod : mod_pos mod_text; \n\
structural_mod : mod_text | mod_text mod_num; \n\
mod_pos : number; \n\
mod_num : number; \n\
mod_text: 'OH' | 'Ke' | 'OOH' | 'My' | 'Me' | 'Br' | 'CHO' | 'COOH' | 'Cp' | 'Ep' | 'KE' | 'NH'; \n\
ether : 'P-' | 'O-'; \n\
stereo : 'R' | 'S'; \n\
fa_pure: carbon carbon_db_separator db | carbon carbon_db_separator db db_hydroxyl_separator hydroxyl; \n\
lcb_fa: lcb_fa_unmod | lcb_fa_unmod lcb_fa_mod; \n\
lcb_fa_unmod: carbon carbon_db_separator db; \n\
lcb_fa_mod: round_open_bracket modification round_close_bracket; \n\
lcb: hydroxyl_lcb lcb_fa | lcb_fa; \n\
carbon: number; \n\
db : db_count | db_count db_positions; \n\
db_count : number; \n\
db_positions : ROB db_position RCB; \n\
db_position : db_single_position | db_position db_position_separator db_position; \n\
db_single_position : db_position_number | db_position_number cistrans; \n\
db_position_number : number; \n\
cistrans : 'E' | 'Z'; \n\
hydroxyl: number; \n\
hydroxyl_lcb: 'm' | 'd' | 't'; \n\
number: digit | digit number; \n\
digit: '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
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
adduct_separator : SPACE; \n\
headgroup_separator: SPACE; \n\
fa_mod_separator: SPACE; \n\
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
element: 'C' | 'H' | 'N' | 'O' | 'P' | 'S' | 'Br' | 'I' | 'F' | 'Cl' | 'As'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : plus_minus; \n\
plus_minus : '-' | '+'; \n\
";



const string swiss_lipids_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) the authors (listed in global LICENSE file) \n\
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
lipid : lipid_pure EOF | lipid_pure adduct_info EOF; \n\
lipid_pure : fatty_acid | gl | pl | sl | st; \n\
 \n\
 \n\
/* adduct information */ \n\
adduct_info : adduct_sep | adduct_separator adduct_sep; \n\
adduct_sep : '[M' adduct ']' charge_sign | '[M' adduct ']' charge charge_sign; \n\
adduct : adduct_set; \n\
adduct_set : adduct_element | adduct_element adduct_set; \n\
adduct_element : element | element number | number element | plus_minus element | plus_minus element number | plus_minus number element; \n\
 \n\
 \n\
/* fatty acyl rules */ \n\
fa : fa_core | fa_lcb_prefix fa_core | fa_core fa_lcb_suffix | fa_lcb_prefix fa_core fa_lcb_suffix; \n\
fa_core : carbon carbon_db_separator db | ether carbon carbon_db_separator db; \n\
 \n\
lcb : lcb_core | fa_lcb_prefix lcb_core | lcb_core fa_lcb_suffix | fa_lcb_prefix lcb_core fa_lcb_suffix; \n\
lcb_core : hydroxyl carbon carbon_db_separator db; \n\
 \n\
carbon : number; \n\
db : db_count | db_count db_positions; \n\
db_count : number; \n\
db_positions : ROB db_position RCB; \n\
db_position : db_single_position | db_position db_position_separator db_position; \n\
db_single_position : db_position_number | db_position_number cistrans; \n\
db_position_number : number; \n\
cistrans : 'E' | 'Z'; \n\
ether : 'O-' | 'P-'; \n\
hydroxyl : 'm' | 'd' | 't'; \n\
fa_lcb_suffix : fa_lcb_suffix_core | fa_lcb_suffix_separator fa_lcb_suffix_core | ROB fa_lcb_suffix_core RCB; \n\
fa_lcb_suffix_core : fa_lcb_suffix_number fa_lcb_suffix_type | fa_lcb_suffix_number fa_lcb_suffix_separator fa_lcb_suffix_type; \n\
fa_lcb_suffix_type : 'OH' | 'me'; \n\
fa_lcb_suffix_number : number; \n\
fa_lcb_prefix : fa_lcb_prefix_type | fa_lcb_prefix_type fa_lcb_prefix_separator; \n\
fa_lcb_prefix_type : 'iso'; \n\
 \n\
/* different fatty acyl types */ \n\
fa_species : fa; \n\
 \n\
fa2 : fa2_sorted | fa2_unsorted; \n\
fa2_sorted : fa sorted_fa_separator fa; \n\
fa2_unsorted : fa unsorted_fa_separator fa; \n\
 \n\
fa3 : fa3_sorted | fa3_unsorted; \n\
fa3_sorted : fa sorted_fa_separator fa sorted_fa_separator fa; \n\
fa3_unsorted : fa unsorted_fa_separator fa unsorted_fa_separator fa; \n\
 \n\
fa4 : fa4_sorted | fa4_unsorted; \n\
fa4_sorted : fa sorted_fa_separator fa sorted_fa_separator fa sorted_fa_separator fa; \n\
fa4_unsorted : fa unsorted_fa_separator fa unsorted_fa_separator fa unsorted_fa_separator fa; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* fatty acid rules */ \n\
fatty_acid : fa_hg fa_fa | fa_hg headgroup_separator fa_fa | mediator; \n\
fa_hg : 'FA' | 'fatty acid' | 'fatty alcohol' | 'NAE' | 'GP-NAE'; \n\
fa_fa : ROB fa RCB; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* mediator rules */ \n\
mediator : mediator_single | mediator_single headgroup_separator med_positions; \n\
mediator_single : mediator_single mediator_single | mediator_single '-' mediator_single | db_positions | med_positions | 'KETE' | 'keto' | 'oxo' | 'Hp' | 'EPE' | 'ETE' | 'ODE' | 'EPT' | 'H' | 'LXA4' | 'hydroxy' | 'Di' | 'RvE1' | 'glyceryl' | 'EpETrE' | 'DHE' | 'ETrE' | 'DHA' | 'DoHA' | 'LTB4' | 'PGE2' | ' PGE2' | 'PGF2alpha' | 'trihydroxy' | 'TriH' | 'OTrE' | 'dihydroxy' | 'Ep' | 'LXB4' | 'Tri' | 'PAHSA' | 'MUFA' | 'GPGP' | 'GPIP' | 'PGE1' | 'PGG2' | 'SFA' | 'PUFA' | 'PGI2' | 'TXA2' | 'CoA' | 'FAOH' | 'EA' | 'beta' | 'PGH2' | 'LTD4' | 'kete' | 'DPE' | ' PGD2' | 'PGD2' | 'PGF2' | 'LTC4' | 'LTA4' | 'PGD1' | 'PGA1' | 'LTDE4' | 'epoxyoleate' | 'epoxystearate' | 'carboxy' | 'PGH1' | 'EtrE' | 'HXA3' | 'HxA3' | 'HXA3-C'; \n\
 \n\
 \n\
 \n\
med_positions : med_position | ROB med_position RCB; \n\
med_position : med_position med_position_separator med_position | number med_suffix | number; \n\
med_suffix : 'S' | 'R'; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* glycerolipid rules */ \n\
gl : gl_regular | gl_mono | gl_molecular; \n\
 \n\
gl_regular : gl_hg gl_fa | gl_hg headgroup_separator gl_fa; \n\
gl_fa : ROB fa_species RCB | ROB fa3 RCB; \n\
gl_hg : 'MG' | 'DG' | 'TG' |  'MAG' | 'DAG' | 'TAG'; \n\
 \n\
gl_molecular : gl_molecular_hg gl_molecular_fa | gl_molecular_hg headgroup_separator gl_molecular_fa; \n\
gl_molecular_fa : ROB fa2 RCB; \n\
gl_molecular_hg : 'DG' | 'DAG'; \n\
 \n\
 \n\
gl_mono : gl_mono_hg gl_mono_fa | gl_mono_hg headgroup_separator gl_mono_fa; \n\
gl_mono_fa : ROB fa_species RCB | ROB fa2 RCB; \n\
gl_mono_hg : 'MHDG' | 'DHDG' | 'MGDG' | 'DGDG'; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* phospholipid rules */ \n\
pl : pl_regular | pl_three | pl_four; \n\
 \n\
pl_regular : pl_hg pl_fa | pl_hg headgroup_separator pl_fa; \n\
pl_fa : ROB fa_species RCB | ROB fa2 RCB; \n\
pl_hg : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | 'PA' | 'PC' | 'PE' | 'PG' | 'PI' | 'PS' | 'PGP' | 'PIP' | 'PIP[3]' | 'PIP[4]' | 'PIP[5]' | 'PIP2' | 'PIP2[3,4]' | 'PIP2[3,5]' | 'PIP2[4,5]' | 'PIP3' | 'PIP3[3,4,5]' | 'CDP-DAG'; \n\
 \n\
pl_three : pl_three_hg pl_three_fa | pl_three_hg headgroup_separator pl_three_fa; \n\
pl_three_fa : ROB fa_species RCB | ROB fa3 RCB; \n\
pl_three_hg : 'NAPE'; \n\
 \n\
pl_four : pl_four_hg pl_four_fa | pl_four_hg headgroup_separator pl_four_fa; \n\
pl_four_fa : ROB fa_species RCB | ROB fa2 RCB | ROB fa4 RCB; \n\
pl_four_hg : 'BMP' | 'LBPA' | 'Lysobisphosphatidate' | 'CL' | 'MLCL' | 'DLCL'; \n\
 \n\
 \n\
 \n\
 \n\
/* sphingolipid rules */ \n\
sl : sl_hg sl_lcb | sl_hg headgroup_separator sl_lcb; \n\
sl_hg : sl_hg_names | sl_hg_prefix sl_hg_names | sl_hg_names sl_hg_suffix | sl_hg_prefix sl_hg_names sl_hg_suffix; \n\
sl_hg_names : 'HexCer' | 'Hex2Cer' | 'SM' | 'PE-Cer' | 'Cer' | 'CerP' | 'Gb3' | 'GA2' | 'GA1' | 'GM3' | 'GM2' | 'GM1' | 'GD3' | 'GT3' | 'GD1' | 'GT1' | 'GQ1' | 'GM4' | 'GD2' | 'GT2' | 'GP1' | 'GD1a' | 'GM1b' | 'GT1b' | 'GQ1b' | 'GT1a' | 'GQ1c' | 'GP1c' | 'GD1c' | 'GD1b' | 'GT1c' | 'IPC' | 'MIPC' | 'M(IP)2C' | 'Gb3Cer' | 'Gb4Cer' | 'Forssman'  | 'MSGG' | 'DSGG' | 'NOR1' | 'NORint' | 'NOR2' | 'Globo-H' | 'Globo-A' | 'SB1a' | 'SM1b' | 'SM1a' | 'Branched-Forssman' | 'Globo-B' | 'Para-Forssman' | 'Globo-Lex-9'; \n\
 \n\
sl_hg_prefix : sl_hg_prefix '-' | sl_hg_prefix sl_hg_prefix | ROB sl_hg_prefix RCB | 'Glc' | 'NAc' | 'Gal' | 'Fuc' | 'SO3' | 'NeuGc' | 'i' | 'NeuAc' | 'Lac' | 'Lex' | '(3\\'-sulfo)' | 'Ac-O-9' | '(alpha2-8)' | '(alpha2-6)' | 'NeuAc' | 'Sulfo'; \n\
sl_hg_suffix : sl_hg_suffix sl_hg_suffix | sl_hg_suffix '/' | ROB sl_hg_suffix RCB | 'NeuAc' | 'NeuGc' | ' alpha'; \n\
 \n\
sl_lcb : sl_lcb_species | sl_lcb_subspecies; \n\
sl_lcb_species : ROB lcb RCB; \n\
sl_lcb_subspecies : ROB lcb sorted_fa_separator fa RCB; \n\
 \n\
 \n\
 \n\
 \n\
/* sterol rules */ \n\
st : st_species | st_sub1 | st_sub2; \n\
 \n\
st_species : st_species_hg st_species_fa | st_species_hg headgroup_separator st_species_fa; \n\
st_species_hg : 'SE'; \n\
st_species_fa : ROB fa_species RCB; \n\
 \n\
st_sub1 : st_sub1_hg st_sub1_fa | st_sub1_hg headgroup_separator st_sub1_fa; \n\
st_sub1_hg : 'CE'; \n\
st_sub1_fa : ROB fa RCB; \n\
 \n\
st_sub2 : st_sub2_hg sorted_fa_separator fa RCB; \n\
st_sub2_hg : 'SE' ROB number COLON number; \n\
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
unsorted_fa_separator : UNDERSCORE; \n\
adduct_separator : SPACE; \n\
sorted_fa_separator : SLASH; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
db_position_separator : COMMA; \n\
med_position_separator : COMMA; \n\
fa_lcb_suffix_separator : DASH; \n\
fa_lcb_prefix_separator : DASH; \n\
 \n\
number :  digit | digit number; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
 \n\
element: 'C' | 'H' | 'N' | 'O' | 'P' | 'S' | 'Br' | 'I' | 'F' | 'Cl' | 'As'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : plus_minus; \n\
plus_minus : '-' | '+'; \n\
";



const string hmdb_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) the authors (listed in global LICENSE file) \n\
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
grammar HMDB; \n\
 \n\
 \n\
/* first rule is always start rule */ \n\
lipid : lipid_pure EOF | lipid_pure adduct_info EOF; \n\
lipid_pure : lipid_class | lipid_class lipid_suffix; \n\
lipid_class : fatty_acid | gl | pl | sl | st; \n\
 \n\
lipid_suffix : '[rac]'; \n\
 \n\
/* adduct information */ \n\
adduct_info : adduct_sep | adduct_separator adduct_sep; \n\
adduct_sep : '[M' adduct ']' charge_sign | '[M' adduct ']' charge charge_sign; \n\
adduct : adduct_set; \n\
adduct_set : adduct_element | adduct_element adduct_set; \n\
adduct_element : element | element number | number element | plus_minus element | plus_minus element number | plus_minus number element; \n\
 \n\
 \n\
 \n\
 \n\
/* fatty acyl rules */ \n\
fa : fa_core | furan_fa | fa_lcb_prefix fa_core | fa_core fa_lcb_suffix | fa_lcb_prefix fa_core fa_lcb_suffix; \n\
fa_core : carbon carbon_db_separator db | ether carbon carbon_db_separator db | methyl carbon carbon_db_separator db; \n\
 \n\
furan_fa : furan_fa_mono | furan_fa_di; \n\
furan_fa_mono : furan_first_number 'M' furan_second_number | 'MonoMe(' furan_first_number ',' furan_second_number ')'; \n\
furan_fa_di : furan_first_number 'D' furan_second_number | 'DiMe(' furan_first_number ',' furan_second_number ')'; \n\
furan_first_number : number; \n\
furan_second_number : number; \n\
 \n\
lcb : lcb_core | fa_lcb_prefix lcb_core | lcb_core fa_lcb_suffix | fa_lcb_prefix lcb_core fa_lcb_suffix; \n\
lcb_core : hydroxyl carbon carbon_db_separator db; \n\
 \n\
carbon : number; \n\
db : db_count | db_count db_positions | db_count db_suffix | db_count db_positions db_suffix; \n\
db_count : number; \n\
db_suffix : 'e' | 'n' db_suffix_number; \n\
db_suffix_number : number; \n\
db_positions : ROB db_position RCB; \n\
db_position : db_single_position | db_position db_position_separator db_position; \n\
db_single_position : db_position_number | db_position_number cistrans; \n\
db_position_number : number; \n\
cistrans : 'E' | 'Z'; \n\
ether : ether_type | ether_link_pos ether_type; \n\
ether_link_pos : number '-'; \n\
ether_type : 'o-' | 'O-' | 'P-'; \n\
methyl : 'i-' | 'a-'; \n\
hydroxyl : 'm' | 'd' | 't'; \n\
fa_lcb_suffix : fa_lcb_suffix_core | fa_lcb_suffix_separator fa_lcb_suffix_core | ROB fa_lcb_suffix_core RCB; \n\
fa_lcb_suffix_core : fa_lcb_suffix_type | fa_lcb_suffix_number fa_lcb_suffix_type | fa_lcb_suffix_number fa_lcb_suffix_separator fa_lcb_suffix_type; \n\
fa_lcb_suffix_type : 'OH' | 'me'; \n\
fa_lcb_suffix_number : number; \n\
fa_lcb_prefix : fa_lcb_prefix_type | fa_lcb_prefix_type fa_lcb_prefix_separator; \n\
fa_lcb_prefix_type : 'iso'; \n\
 \n\
/* different fatty acyl types */ \n\
fa_species : fa; \n\
 \n\
fa2 : fa2_sorted | fa2_unsorted; \n\
fa2_sorted : fa sorted_fa_separator fa; \n\
fa2_unsorted : fa unsorted_fa_separator fa; \n\
 \n\
fa3 : fa3_sorted | fa3_unsorted; \n\
fa3_sorted : fa sorted_fa_separator fa sorted_fa_separator fa; \n\
fa3_unsorted : fa unsorted_fa_separator fa unsorted_fa_separator fa; \n\
 \n\
fa4 : fa4_sorted | fa4_unsorted; \n\
fa4_sorted : fa sorted_fa_separator fa sorted_fa_separator fa sorted_fa_separator fa; \n\
fa4_unsorted : fa unsorted_fa_separator fa unsorted_fa_separator fa unsorted_fa_separator fa; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* fatty acid rules */ \n\
fatty_acid : fa_hg fa_fa | fa_hg interlink_fa | fa_hg headgroup_separator fa_fa | mediator; \n\
fa_hg : 'FA' | 'fatty acid' | 'fatty alcohol' | 'NAE' | 'GP-NAE' | 'FAHFA'; \n\
fa_fa : ROB fa RCB; \n\
interlink_fa : ROB fa sorted_fa_separator fa RCB; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* mediator rules */ \n\
mediator : mediator_single | mediator_single headgroup_separator med_positions; \n\
mediator_single : mediator_single mediator_single | mediator_single '-' mediator_single | db_positions | med_positions | 'KETE' | 'keto' | 'oxo' | 'Hp' | 'EPE' | 'ETE' | 'ODE' | 'EPT' | 'H' | 'LXA4' | 'hydroxy' | 'Di' | 'RvE1' | 'glyceryl' | 'EpETrE' | 'DHE' | 'ETrE' | 'DHA' | 'DoHA' | 'LTB4' | 'PGE2' | ' PGE2' | 'PGF2alpha' | 'trihydroxy' | 'TriH' | 'OTrE' | 'dihydroxy' | 'Ep' | 'LXB4' | 'Tri' | 'PAHSA' | 'MUFA' | 'GPGP' | 'GPIP' | 'PGE1' | 'PGG2' | 'SFA' | 'PUFA' | 'PGI2' | 'TXA2' | 'CoA' | 'FAOH' | 'EA' | 'beta' | 'PGH2' | 'LTD4' | 'kete' | 'DPE' | ' PGD2' | 'PGD2' | 'PGF2' | 'LTC4' | 'LTA4' | 'PGD1' | 'PGA1' | 'LTDE4' | 'epoxyoleate' | 'epoxystearate' | 'carboxy' | 'PGH1' | 'EtrE' | 'HXA3' | 'HxA3' | 'HXA3-C'; \n\
 \n\
 \n\
 \n\
med_positions : med_position | ROB med_position RCB; \n\
med_position : med_position med_position_separator med_position | number med_suffix | number; \n\
med_suffix : 'S' | 'R'; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* glycerolipid rules */ \n\
gl : gl_regular | gl_mono | gl_molecular; \n\
 \n\
gl_regular : gl_hg gl_fa | gl_hg headgroup_separator gl_fa; \n\
gl_fa : ROB fa_species RCB | ROB fa3 RCB; \n\
gl_hg : 'MG' | 'DG' | 'TG' |  'MAG' | 'DAG' | 'TAG'; \n\
 \n\
gl_molecular : gl_molecular_hg gl_molecular_fa | gl_molecular_hg headgroup_separator gl_molecular_fa; \n\
gl_molecular_fa : ROB fa2 RCB; \n\
gl_molecular_hg : 'DG' | 'DAG'; \n\
 \n\
 \n\
gl_mono : gl_mono_hg gl_mono_fa | gl_mono_hg headgroup_separator gl_mono_fa; \n\
gl_mono_fa : ROB fa_species RCB | ROB fa2 RCB; \n\
gl_mono_hg : 'MHDG' | 'DHDG' | 'MGDG' | 'DGDG'; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
/* phospholipid rules */ \n\
pl : pl_regular | pl_three | pl_four; \n\
 \n\
pl_regular : pl_hg pl_fa | pl_hg headgroup_separator pl_fa; \n\
pl_fa : ROB fa_species RCB | ROB fa2 RCB; \n\
pl_hg : 'LPA' | 'LPC' | 'LPE' | 'LPG' | 'LPI' | 'LPS' | 'PA' | 'PC' | 'PE' | 'PG' | 'PI' | 'PS' | 'PGP' | 'PIP' | 'PIP[3]' | 'PIP[4]' | 'PIP[5]' | 'PIP2' | 'PIP2[3,4]' | 'PIP2[3,5]' | 'PIP2[4,5]' | 'PIP3' | 'PIP3[3,4,5]' | 'CDP-DAG' | 'LysoPA' | 'LysoPC' | 'LysoPE' | 'LysoPG' | 'LysoPI' | 'LysoPS' | 'PE-NMe' | 'PE-NMe2' | 'CDP-DG'; \n\
 \n\
pl_three : pl_three_hg pl_three_fa | pl_three_hg headgroup_separator pl_three_fa; \n\
pl_three_fa : ROB fa_species RCB | ROB fa3 RCB; \n\
pl_three_hg : 'NAPE'; \n\
 \n\
pl_four : pl_four_hg pl_four_fa | pl_four_hg headgroup_separator pl_four_fa; \n\
pl_four_fa : ROB fa_species RCB | ROB fa2 RCB | ROB fa4 RCB; \n\
pl_four_hg : 'BMP' | 'LBPA' | 'Lysobisphosphatidate' | 'CL' | 'MLCL' | 'DLCL'; \n\
 \n\
 \n\
 \n\
 \n\
/* sphingolipid rules */ \n\
sl : sl_hg sl_lcb | sl_hg headgroup_separator sl_lcb; \n\
sl_hg : all_sl_hg_names | sl_hg_prefix all_sl_hg_names | all_sl_hg_names sl_hg_suffix | sl_hg_prefix all_sl_hg_names sl_hg_suffix; \n\
all_sl_hg_names : sl_hg_names | ganglioside; \n\
sl_hg_names : 'HexCer' | 'Hex2Cer' | 'SM' | 'PE-Cer' | 'Cer' | 'CerP' | 'IPC' | 'MIPC' | 'M(IP)2C' | 'Gb3Cer' | 'Gb4Cer' | 'Forssman'  | 'MSGG' | 'DSGG' | 'NOR1' | 'NORint' | 'NOR2' | 'Globo-H' | 'Globo-A' | 'SB1a' | 'SM1b' | 'SM1a' | 'Branched-Forssman' | 'Globo-B' | 'Para-Forssman' | 'Globo-Lex-9' | 'LysoSM' | 'Glucosylceramide' | 'Ceramide' | 'Tetrahexosylceramide'; \n\
ganglioside : 'Ganglioside' headgroup_separator ganglioside_names | ganglioside_names; \n\
ganglioside_names : 'Gb3' | 'GA2' | 'GA1' | 'GM3' | 'GM2' | 'GM1' | 'GD3' | 'GT3' | 'GD1' | 'GT1' | 'GQ1' | 'GM4' | 'GD2' | 'GT2' | 'GP1' | 'GD1a' | 'GM1b' | 'GT1b' | 'GQ1b' | 'GT1a' | 'GQ1c' | 'GP1c' | 'GD1c' | 'GD1b' | 'GT1c'; \n\
 \n\
sl_hg_prefix : sl_hg_prefix '-' | sl_hg_prefix sl_hg_prefix | ROB sl_hg_prefix RCB | 'Glc' | 'NAc' | 'Gal' | 'Fuc' | 'SO3' | 'NeuGc' | 'i' | 'NeuAc' | 'Lac' | 'Lex' | '(3\\'-sulfo)' | 'Ac-O-9' | '(alpha2-8)' | '(alpha2-6)' | 'NeuAc' | 'Sulfo'; \n\
sl_hg_suffix : sl_hg_suffix sl_hg_suffix | sl_hg_suffix '/' | ROB sl_hg_suffix RCB | 'NeuAc' | 'NeuGc' | ' alpha'; \n\
 \n\
sl_lcb : sl_lcb_species | sl_lcb_subspecies; \n\
sl_lcb_species : ROB lcb RCB; \n\
sl_lcb_subspecies : ROB lcb sorted_fa_separator fa RCB; \n\
 \n\
 \n\
 \n\
 \n\
/* sterol rules */ \n\
st : st_species | st_sub1 | st_sub2; \n\
 \n\
st_species : st_species_hg st_species_fa | st_species_hg headgroup_separator st_species_fa; \n\
st_species_hg : 'SE'; \n\
st_species_fa : ROB fa_species RCB; \n\
 \n\
st_sub1 : st_sub1_hg st_sub1_fa | st_sub1_hg headgroup_separator st_sub1_fa; \n\
st_sub1_hg : 'CE'; \n\
st_sub1_fa : ROB fa RCB; \n\
 \n\
st_sub2 : st_sub2_hg st_sub2_fa | st_sub2_hg headgroup_separator st_sub2_fa; \n\
st_sub2_hg : 'SE'; \n\
st_sub2_fa : ROB fa2 RCB; \n\
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
unsorted_fa_separator : UNDERSCORE; \n\
sorted_fa_separator : SLASH; \n\
adduct_separator : SPACE; \n\
headgroup_separator : SPACE; \n\
carbon_db_separator : COLON; \n\
db_position_separator : COMMA; \n\
med_position_separator : COMMA; \n\
fa_lcb_suffix_separator : DASH; \n\
fa_lcb_prefix_separator : DASH; \n\
 \n\
number :  digit | digit number; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
 \n\
element: 'C' | 'H' | 'N' | 'O' | 'P' | 'S' | 'Br' | 'I' | 'F' | 'Cl' | 'As'; \n\
charge : '1' | '2' | '3' | '4'; \n\
charge_sign : plus_minus; \n\
plus_minus : '-' | '+'; \n\
";



const string sum_formula_grammar = "//////////////////////////////////////////////////////////////////////////////// \n\
// MIT License \n\
//  \n\
// Copyright (c) the authors (listed in global LICENSE file) \n\
// \n\
// Permission is hereby granted, free of charge, to any person obtaining a copy \n\
// of this software and associated documentation files (the \"Software\"), to deal \n\
// in the Software without restriction, including without limitation the rights \n\
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell \n\
// copies of the Software, and to permit persons to whom the Software is \n\
// furnished to do so, subject to the following conditions: \n\
//  \n\
// The above copyright notice and this permission notice shall be included in all \n\
// copies or substantial portions of the Software. \n\
//  \n\
// THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n\
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
grammar SumFormula; \n\
 \n\
/* first rule is always start rule */ \n\
molecule: molecule_rule EOF; \n\
molecule_rule: molecule_group; \n\
molecule_group: element_group | single_element | molecule_group molecule_group; \n\
element_group: element count; \n\
single_element: element; \n\
element: 'C' | 'H' | 'N' | 'O' | 'P' | 'S' | 'Br' | 'I' | 'F' | 'Cl' | 'As'; \n\
count: number | '-' number; \n\
number : digit | digit number; \n\
digit:  '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
 \n\
";



const string fatty_acid_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) the authors (listed in global LICENSE file) \n\
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
grammar FattyAcids; \n\
 \n\
 \n\
/* first rule is always start rule, EOF = end of file */ \n\
lipid : fatty_acid EOF; \n\
 \n\
fatty_acid: regular_fatty_acid | wax | car | ethanolamine | amine | acetic_acid; \n\
fatty_acid_recursion: regular_fatty_acid; \n\
wax : wax_ester regular_fatty_acid; \n\
wax_ester : fatty_acid_recursion SPACE | ROB fatty_acid_recursion RCB SPACE | methyl SPACE | methyl DASH; \n\
methyl : 'methyl'; \n\
car : car_positions DASH car_fa '-4-(' car_spec ')butanoate'; \n\
car_fa : SOB regular_fatty_acid SCB | COB regular_fatty_acid CCB; \n\
car_spec : 'trimethylammonio' | 'trimethylazaniumyl'; \n\
 \n\
car_positions : functional_position | ROB car_position RCB DASH functional_position; \n\
car_position : number; \n\
ethanolamine : amine_prefix ROB fatty_acid_recursion RCB DASH 'ethanolamine'; \n\
amine : amine_prefix amine_n DASH regular_fatty_acid SPACE 'amine'; \n\
amine_prefix : 'n-' | '(+/-)n-'; \n\
amine_n : fatty_acid_recursion | ROB fatty_acid_recursion RCB | methyl; \n\
acetic_acid : acetic_recursion 'acetic acid'; \n\
acetic_recursion : ROB fatty_acid_recursion RCB | SOB fatty_acid_recursion SCB | COB fatty_acid_recursion CCB; \n\
 \n\
regular_fatty_acid : ate_type | \n\
                     ol_position_description |  \n\
                     additional_len acid_type_regular | \n\
                     additional_len acid_type_double | \n\
                     additional_descriptions double_bond_positions fatty_length acid_type_double | \n\
                     sum_add fatty_length acid_type_regular |  \n\
                     sum_add fatty_length acid_type_double |  \n\
                     fg_pos_summary fatty_length acid_type_double | \n\
                     /* double_bond_positions fatty_length acid_type_double | */ \n\
                     fg_pos_summary double_bond_positions fatty_length acid_type_double |   \n\
                     sum_add double_bond_positions fatty_length acid_type_double; \n\
 \n\
additional_len : additional_descriptions fatty_length | fatty_length; \n\
sum_add : fg_pos_summary additional_descriptions; \n\
ol_position_description : ol_position DASH fatty_length ol_ending | \n\
         fg_pos_summary ol_position DASH fatty_length ol_ending |  \n\
         fg_pos_summary DASH ol_position DASH fatty_length ol_ending |  \n\
         sum_add ol_position DASH fatty_length ol_ending |  \n\
         sum_add DASH ol_position DASH fatty_length ol_ending | \n\
         additional_descriptions ol_position DASH fatty_length ol_ending | \n\
         additional_descriptions DASH ol_position DASH fatty_length ol_ending; \n\
ol_ending : 'ol' | 'nol'; \n\
ol_position : ol_pos | ol_pos PRIME | ol_pos cistrans_b | ol_pos PRIME cistrans_b; \n\
ol_pos : number; \n\
 \n\
 \n\
fatty_length : notation_specials | notation_regular | cycle notation_specials | cycle notation_regular | tetrahydrofuran notation_specials; \n\
notation_regular : notation_last_digit | notation_last_digit notation_second_digit | notation_second_digit; \n\
/* 1, 2, 2, 3, 4, 4, 4, 5, 6, 7, 8, 9 */ \n\
notation_last_digit : 'un' | 'hen' | 'do' | 'di' | 'tri' | 'buta' | 'but' | 'tetra' | 'penta' | 'pent' | 'hexa' | 'hex' | 'hepta' | 'hept' | 'octa' | 'oct' | 'nona' | 'non'; \n\
/* 0, 10, 10, 20, 20, 30 */ \n\
notation_second_digit: 'deca' | 'dec' | 'cosa' | 'cos' | 'eicosa' | 'eicos' | 'triaconta' | 'triacont' | 'tetraconta'  | 'tetracont' | 'pentaconta' | 'pantacont' | 'hexaconta' | 'hexacont' | 'heptaconta' | 'heptacont' | 'octaconta' | 'octacont' | 'nonaconta' | 'nonacont'; \n\
/* 4, 10, 20, 21, 21, 30, 30 */ \n\
notation_specials: 'etha' | 'eth' | 'buta' | 'but' | 'butr' | 'valer' | 'propa' | 'propi' | 'propio' | 'prop' | 'eicosa' | 'eicos' | 'icosa' | 'icos' | prosta | isoprop | furan; \n\
isoprop: 'isoprop'; \n\
prosta : 'prosta' | 'prost' | 'prostan'; \n\
tetrahydrofuran : 'tetrahydro'; \n\
furan : 'furan'; \n\
 \n\
acid_type_regular: acid_single_type | acid_single_type cyclo_position; \n\
acid_type_double: db_num acid_type_regular; \n\
acid_single_type: 'noic acid' | 'nic acid' | 'nal' | dioic | 'noyloxy' | 'noyl' | ol | dial | 'noate' | 'nate' | coa | yl | 'ne' | 'yloxy'; \n\
coa : 'noyl' coa_ending | 'yl' coa_ending | 'nyl' coa_ending; \n\
coa_ending : 'coa' | '-coa'; \n\
yl : 'yl' | 'nyl' | 'n' DASH yl_ending DASH 'yl' | DASH yl_ending DASH 'yl'; \n\
yl_ending: number; \n\
 \n\
db_num: DASH double_bond_positions DASH db_length db_suffix | DASH double_bond_positions DASH db_suffix | db_length db_suffix | db_suffix; \n\
db_suffix : 'e' | 'ne' | 'ene' | 'en' | 'n'; \n\
dial : 'dial'; \n\
db_length: notation_regular; \n\
dioic : 'n' DASH functional_positions DASH dioic_acid | DASH functional_positions DASH dioic_acid | 'ne' dioic_acid; \n\
dioic_acid : 'dioic acid'; \n\
ol : 'nol' | db_suffix DASH hydroxyl_positions DASH notation_regular 'ol' | db_suffix DASH hydroxyl_position DASH 'ol' | DASH hydroxyl_positions DASH notation_regular 'ol' | DASH hydroxyl_position DASH 'ol'; \n\
ate_type : ate | additional_descriptions ate; \n\
ate : 'formate' | 'acetate' | 'butyrate' | 'propionate' | 'valerate' | isobut; \n\
isobut : 'isobutyrate'; \n\
 \n\
 \n\
hydroxyl_positions : hydroxyl_positions pos_separator hydroxyl_positions | hydroxyl_position; \n\
hydroxyl_position : hydroxyl_number | hydroxyl_number cistrans_b | hydroxyl_number PRIME cistrans_b; \n\
hydroxyl_number : number; \n\
 \n\
additional_descriptions : additional_descriptions_m | additional_descriptions_m DASH; \n\
additional_descriptions_m : additional_descriptions_m additional_descriptions_m | additional_description; \n\
additional_description : functional_group | functional_group DASH | pos_neg | reduction | reduction DASH; \n\
functional_group : multi_functional_group | single_functional_group | epoxy | methylene_group; \n\
pos_neg : '(+/-)-' | '(+)-' | '(-)-'; \n\
 \n\
double_bond_positions : double_bond_positions_pure DASH | ROB double_bond_positions_pure RCB DASH | double_bond_positions_pure | ROB double_bond_positions_pure RCB; \n\
double_bond_positions_pure : double_bond_positions_pure pos_separator double_bond_positions_pure | double_bond_position; \n\
double_bond_position : db_number | db_number cistrans_b | db_number PRIME | db_number PRIME cistrans_b | cistrans_b; \n\
cistrans_b : cistrans | ROB cistrans RCB; \n\
cistrans : 'e' | 'z' | 'r' | 's' | 'a' | 'b' | 'c'; \n\
db_number : number; \n\
fg_pos_summary : functional_positions DASH; \n\
 \n\
multi_functional_group : functional_positions DASH functional_length functional_group_type | functional_positions DASH functional_group_type; \n\
functional_length : notation_last_digit | notation_second_digit | notation_last_digit notation_second_digit; \n\
functional_positions : functional_positions_pure | ROB functional_positions_pure RCB; \n\
functional_positions_pure : functional_positions_pure pos_separator functional_positions_pure | functional_position; \n\
single_functional_group : functional_position DASH functional_group_type_name | functional_position functional_group_type_name | recursion_description | recursion_description DASH; \n\
functional_group_type_name : functional_group_type | ROB functional_group_type RCB; \n\
functional_group_type : 'hydroxy' | 'oxo' | 'bromo' | 'thio' | 'keto' | 'methyl' | 'hydroperoxy' | homo | 'phospho' | 'fluro' | 'fluoro' | 'chloro' | methylene | 'sulfooxy' | 'amino' | 'sulfanyl' | 'methoxy' | 'iodo' | 'cyano' | 'nitro' | 'oh' | 'thio' | 'mercapto' | 'carboxy' | 'acetoxy' | 'cysteinyl' | 'phenyl' | 's-glutathionyl' | 's-cysteinyl' | 'butylperoxy' | 'dimethylarsinoyl' | 'methylsulfanyl' | 'imino' | 's-cysteinylglycinyl'; \n\
epoxy : functional_position pos_separator functional_position DASH 'epoxy' | functional_position ROB functional_position RCB DASH 'epoxy' | ROB functional_position pos_separator functional_position RCB DASH 'epoxy'; \n\
methylene_group : functional_positions DASH methylene; \n\
methylene : 'methylene'; \n\
/* acetoxy : 'acetoxy'; */ \n\
 \n\
functional_position : functional_position_pure | ROB functional_position_pure RCB; \n\
functional_position_pure : functional_pos | functional_pos PRIME | functional_pos func_stereo | functional_pos PRIME func_stereo | func_stereo; \n\
functional_pos : number; \n\
func_stereo : cistrans_b; \n\
reduction : functional_position DASH 'nor' | functional_positions DASH functional_length 'nor'; \n\
homo : 'homo'; \n\
 \n\
cycle : 'cyclo'; \n\
cyclo_position : '-cyclo' SOB functional_position pos_separator functional_position SCB | ' cyclo' SOB functional_position pos_separator functional_position SCB | ' cyclo ' SOB functional_position pos_separator functional_position SCB | '-cyclo-' SOB functional_position pos_separator functional_position SCB | ' cyclo-' SOB functional_position pos_separator functional_position SCB; \n\
 \n\
recursion_description : recursion_position DASH recursion; \n\
recursion : fatty_acid | ROB fatty_acid RCB | SOB fatty_acid SCB | COB fatty_acid CCB; \n\
recursion_position : ROB functional_positions RCB | recursion_pos | recursion_pos cistrans_b; \n\
recursion_pos : number; \n\
 \n\
 \n\
 \n\
 \n\
/* separators */ \n\
SPACE : ' '; \n\
COLON : ':'; \n\
SEMICOLON : ';'; \n\
DASH : '-' | '‐'; \n\
UNDERSCORE : '_'; \n\
SLASH : '/'; \n\
BACKSLASH : '\\\\'; \n\
COMMA: ','; \n\
ROB: '('; \n\
RCB: ')'; \n\
SOB: '['; \n\
SCB: ']'; \n\
COB: '{'; \n\
CCB: '}'; \n\
PRIME: '\\''; \n\
pos_separator : COMMA; \n\
 \n\
 \n\
 \n\
number :  digit | digit number; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
 \n\
 \n\
";



const string systematic_grammar = "/* \n\
 * MIT License \n\
 *  \n\
 * Copyright (c) the authors (listed in global LICENSE file) \n\
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
grammar Systematic; \n\
 \n\
 \n\
/* first rule is always start rule, EOF = end of file */ \n\
lipid : lipid_category EOF; \n\
 \n\
lipid_category : FA | GL | GP | SL | ST; \n\
 \n\
/* fatty acids */ \n\
FA : fa; \n\
 \n\
/* glycero lipids */ \n\
GL : gl | xdg | xmg; \n\
gl : generic_fa DASH gl_ending | generic_fa2 DASH gl_ending | generic_fa3 DASH gl_ending; \n\
gl_ending : 'glycerol' | sn_rac DASH 'glycerol'; \n\
xdg : generic_fa2 DASH xdg_xmg_ending; \n\
xmg : generic_fa DASH xdg_xmg_ending; \n\
xdg_xmg_ending : '3-o-β-d-galactosyl-sn-glycerol' | '3-o-(6\\'-o-α-d-galactosyl-β-d-galactosyl)-sn-glycerol' | '3-o-(α-d-galactosyl1-6)-β-d-galactosyl-sn-glycerol' | '3-(6\\'-sulfo-α-d-quinovosyl)-sn-glycerol'; \n\
sn_rac : 'rac' | 'sn'; \n\
         \n\
generic_fa : generic_position DASH generic_fa_rob | generic_position generic_fa_rob_s | generic_position DASH ROB number COLON number generic_fa_rob; \n\
generic_fa2 : generic_position COMMA generic_position DASH 'di' generic_fa_rob | generic_position COMMA generic_position DASH 'di' DASH generic_fa_rob | generic_fa DASH generic_fa; \n\
generic_fa3 : generic_position COMMA generic_position COMMA generic_position DASH 'tri' generic_fa_rob | generic_fa DASH generic_fa2 | generic_fa2 DASH generic_fa; \n\
generic_fa_rob : ROB fa RCB | fa; \n\
generic_fa_rob_s : ROB fa RCB; \n\
generic_position : generic_pos | generic_pos DASH 'o'; \n\
generic_pos : number; \n\
 \n\
 \n\
/* phospho glycero lipids */ \n\
GP : gp | cl; \n\
gp : generic_fa DASH gp_ending | generic_fa2 DASH gp_ending; \n\
 \n\
/* PC, PE, PS */ \n\
gp_ending : 'sn-glycero-3-phosphocholine' | 'sn-glycero-3-phosphoethanolamine' | 'sn-glycero-3-phosphoserine'; \n\
 \n\
 \n\
 \n\
 \n\
 \n\
 \n\
fa : fatty_acid | acetyl; \n\
acetyl : 'acetyl'; \n\
fatty_acid: regular_fatty_acid | wax | CAR | ethanolamine | amine | acetic_acid; \n\
wax : wax_ester fatty_acid_type | wax_ester regular_fatty_acid; \n\
wax_ester : fatty_acid SPACE | ROB fatty_acid RCB SPACE | methyl SPACE | methyl DASH; \n\
methyl : 'methyl'; \n\
CAR : car_positions DASH CAR_fa '-4-(' CAR_spec ')butanoate'; \n\
CAR_fa : SOB regular_fatty_acid SCB | COB regular_fatty_acid CCB; \n\
CAR_spec : 'trimethylammonio' | 'trimethylazaniumyl'; \n\
 \n\
car_positions : functional_position | ROB car_position RCB DASH functional_position; \n\
ethanolamine : amine_prefix ROB fatty_acid RCB DASH 'ethanolamine'; \n\
amine : amine_prefix amine_n DASH regular_fatty_acid SPACE 'amine'; \n\
amine_prefix : 'n-' | '(+/-)n-'; \n\
amine_n : fatty_acid | ROB fatty_acid RCB | methyl; \n\
acetic_acid : acetic_recursion 'acetic acid'; \n\
acetic_recursion : fatty_acid | ROB fatty_acid RCB | SOB fatty_acid SCB | COB fatty_acid CCB; \n\
 \n\
regular_fatty_acid : ate_type | \n\
                     ol_position_description |  \n\
                     furan | \n\
                     additional_descriptions furan |  \n\
                     sum_add furan |  \n\
                     additional_len acid_type_regular | \n\
                     additional_len acid_type_double | \n\
                     additional_descriptions double_bond_positions fatty_length acid_type_double | \n\
                     sum_add fatty_length acid_type_regular |  \n\
                     sum_add fatty_length acid_type_double |  \n\
                     fg_pos_summary fatty_length acid_type_double |   \n\
                     fg_pos_summary double_bond_positions fatty_length acid_type_double |   \n\
                     sum_add double_bond_positions fatty_length acid_type_double; \n\
 \n\
additional_len : additional_descriptions fatty_length | fatty_length; \n\
sum_add : fg_pos_summary additional_descriptions; \n\
ol_position_description : ol_position DASH fatty_length ol_ending | \n\
         fg_pos_summary ol_position DASH fatty_length ol_ending |  \n\
         fg_pos_summary DASH ol_position DASH fatty_length ol_ending |  \n\
         sum_add ol_position DASH fatty_length ol_ending |  \n\
         sum_add DASH ol_position DASH fatty_length ol_ending | \n\
         additional_descriptions ol_position DASH fatty_length ol_ending | \n\
         additional_descriptions DASH ol_position DASH fatty_length ol_ending; \n\
ol_ending : 'ol' | 'nol'; \n\
ol_position : ol_pos | ol_pos PRIME | ol_pos cistrans_b | ol_pos PRIME cistrans_b; \n\
ol_pos : number; \n\
 \n\
 \n\
fatty_length : notation_specials | notation_regular | cycle notation_specials | cycle notation_regular; \n\
notation_regular : notation_last_digit | notation_last_digit notation_second_digit | notation_second_digit; \n\
/* 1, 2, 2, 3, 4, 4, 4, 5, 6, 7, 8, 9 */ \n\
notation_last_digit : 'un' | 'hen' | 'do' | 'di' | 'tri' | 'buta' | 'but' | 'tetra' | 'penta' | 'pent' | 'hexa' | 'hex' | 'hepta' | 'hept' | 'octa' | 'oct' | 'nona' | 'non'; \n\
/* 0, 10, 10, 20, 20, 30 */ \n\
notation_second_digit: 'deca' | 'dec' | 'cosa' | 'cos' | 'eicosa' | 'eicos' | 'triaconta' | 'triacont' | 'tetraconta'  | 'tetracont' | 'pentaconta' | 'pantacont' | 'hexaconta' | 'hexacont' | 'heptaconta' | 'heptacont' | 'octaconta' | 'octacont' | 'nonaconta' | 'nonacont'; \n\
/* 4, 10, 20, 21, 21, 30, 30 */ \n\
notation_specials: 'etha' | 'eth' | 'buta' | 'but' | 'butr' | 'valer' | 'propa' | 'propi' | 'propio' | 'prop' | 'eicosa' | 'eicos' | 'icosa' | 'icos' | prosta | isoprop; \n\
isoprop: 'isoprop'; \n\
prosta : 'prosta' | 'prost' | 'prostan'; \n\
 \n\
acid_type_regular: acid_single_type | acid_single_type cyclo_position; \n\
acid_type_double: db_num acid_type_regular; \n\
acid_single_type: 'noic acid' | 'nic acid' | 'nal' | dioic | 'noyloxy' | 'noyl' | ol | dial | 'noate' | 'nate' | CoA | yl | 'ne' | 'nyloxy'; \n\
CoA : 'noyl' coa | 'yl' coa | 'nyl' coa; \n\
furan : 'furan' DASH furan_pos DASH 'yl'; \n\
furan_pos : number; \n\
coa : 'coa' | '-coa'; \n\
yl : 'yl' | 'nyl' | 'n' DASH yl_ending DASH 'yl' | DASH yl_ending DASH 'yl'; \n\
yl_ending: number; \n\
 \n\
db_num: DASH double_bond_positions DASH db_length db_suffix | DASH double_bond_positions DASH db_suffix | db_length db_suffix | db_suffix; \n\
db_suffix : 'e' | 'ne' | 'ene' | 'en' | 'n'; \n\
dial : 'dial'; \n\
db_length: notation_regular; \n\
dioic : 'n' DASH functional_positions DASH dioic_acid | DASH functional_positions DASH dioic_acid | 'ne' dioic_acid; \n\
dioic_acid : 'dioic acid'; \n\
ol : 'nol' | db_suffix DASH hydroxyl_positions DASH notation_regular 'ol' | db_suffix DASH hydroxyl_position DASH 'ol' | DASH hydroxyl_positions DASH notation_regular 'ol' | DASH hydroxyl_position DASH 'ol'; \n\
ate_type : ate | additional_descriptions ate; \n\
ate : 'formate' | 'acetate' | 'butyrate' | 'propionate' | 'valerate' | isobut; \n\
isobut : 'isobutyrate'; \n\
 \n\
 \n\
hydroxyl_positions : hydroxyl_positions pos_separator hydroxyl_positions | hydroxyl_position; \n\
hydroxyl_position : hydroxyl_number | hydroxyl_number cistrans_b | hydroxyl_number PRIME cistrans_b; \n\
hydroxyl_number : number; \n\
 \n\
additional_descriptions : additional_descriptions_m | additional_descriptions_m DASH; \n\
additional_descriptions_m : additional_descriptions_m additional_descriptions_m | additional_description; \n\
additional_description : functional_group | functional_group DASH | pos_neg | reduction | reduction DASH; \n\
functional_group : multi_functional_group | single_functional_group | epoxy | methylene_group; \n\
pos_neg : '(+/-)-' | '(+)-' | '(-)-'; \n\
 \n\
double_bond_positions : double_bond_positions_pure DASH | ROB double_bond_positions_pure RCB DASH | double_bond_positions_pure | ROB double_bond_positions_pure RCB; \n\
double_bond_positions_pure : double_bond_positions_pure pos_separator double_bond_positions_pure | double_bond_position; \n\
double_bond_position : db_number | db_number cistrans_b | db_number PRIME | db_number PRIME cistrans_b | cistrans_b; \n\
cistrans_b : cistrans | ROB cistrans RCB; \n\
cistrans : 'e' | 'z' | 'r' | 's' | 'a' | 'b' | 'c'; \n\
db_number : number; \n\
fg_pos_summary : functional_positions DASH; \n\
 \n\
multi_functional_group : functional_positions DASH functional_length functional_group_type | functional_positions DASH functional_group_type; \n\
functional_length : notation_last_digit | notation_second_digit | notation_last_digit notation_second_digit; \n\
functional_positions : functional_positions_pure | ROB functional_positions_pure RCB; \n\
functional_positions_pure : functional_positions_pure pos_separator functional_positions_pure | functional_position; \n\
single_functional_group : functional_position DASH functional_group_type_name | functional_position functional_group_type_name | recursion_description | recursion_description DASH; \n\
functional_group_type_name : functional_group_type | ROB functional_group_type RCB; \n\
functional_group_type : 'hydroxy' | 'oxo' | 'bromo' | 'thio' | 'keto' | 'methyl' | 'hydroperoxy' | homo | 'phospho' | 'fluro' | 'fluoro' | 'chloro' | methylene | 'sulfooxy' | 'amino' | 'sulfanyl' | 'methoxy' | 'iodo' | 'cyano' | 'nitro' | 'oh' | 'thio' | 'mercapto' | 'carboxy' | 'acetoxy' | 'cysteinyl' | 'phenyl' | 's-glutathionyl' | 's-cysteinyl' | 'butylperoxy' | 'dimethylarsinoyl' | 'methylsulfanyl' | 'imino' | 's-cysteinylglycinyl'; \n\
epoxy : functional_position pos_separator functional_position DASH 'epoxy' | functional_position ROB functional_position RCB DASH 'epoxy' | ROB functional_position pos_separator functional_position RCB DASH 'epoxy'; \n\
methylene_group : functional_positions DASH methylene; \n\
methylene : 'methylene'; \n\
/* acetoxy : 'acetoxy'; */ \n\
 \n\
functional_position : functional_position_pure | ROB functional_position_pure RCB; \n\
functional_position_pure : functional_pos | functional_pos PRIME | functional_pos func_stereo | functional_pos PRIME func_stereo | func_stereo; \n\
functional_pos : number; \n\
func_stereo : cistrans_b; \n\
reduction : functional_position DASH 'nor' | functional_positions DASH functional_length 'nor'; \n\
homo : 'homo'; \n\
 \n\
cycle : 'cyclo'; \n\
cyclo_position : '-cyclo' SOB functional_position pos_separator functional_position SCB | ' cyclo' SOB functional_position pos_separator functional_position SCB | ' cyclo ' SOB functional_position pos_separator functional_position SCB | '-cyclo-' SOB functional_position pos_separator functional_position SCB | ' cyclo-' SOB functional_position pos_separator functional_position SCB; \n\
 \n\
recursion_description : recursion_position DASH recursion; \n\
recursion : fatty_acid | ROB fatty_acid RCB | SOB fatty_acid SCB | COB fatty_acid CCB; \n\
recursion_position : ROB functional_positions RCB | recursion_pos | recursion_pos cistrans_b; \n\
recursion_pos : number; \n\
 \n\
 \n\
 \n\
 \n\
/* separators */ \n\
SPACE : ' '; \n\
COLON : ':'; \n\
SEMICOLON : ';'; \n\
DASH : '-' | '‐'; \n\
UNDERSCORE : '_'; \n\
SLASH : '/'; \n\
BACKSLASH : '\\\\'; \n\
COMMA: ','; \n\
ROB: '('; \n\
RCB: ')'; \n\
SOB: '['; \n\
SCB: ']'; \n\
COB: '{'; \n\
CCB: '}'; \n\
PRIME: '\\''; \n\
pos_separator : COMMA; \n\
 \n\
 \n\
 \n\
number :  digit | digit number; \n\
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'; \n\
 \n\
 \n\
";



#endif /* KNOWN_GRAMMARS_H */
