#ifndef LIPID_ENUMS_H
#define LIPID_ENUMS_H

#include <vector>
#include <map>

using namespace std;

enum LipidCategory {NO_CATEGORY,
    UNDEFINED_CATEGORY, 
    GL, // SLM:000117142 Glycerolipids
    GP, // SLM:000001193 Glycerophospholipids
    SP, // SLM:000000525 Sphingolipids
    ST, // SLM:000500463 Steroids and derivatives
    FA, // SLM:000390054 Fatty acyls and derivatives
    SL // Saccharo lipids
};



static const map<LipidCategory, string> CategoryString = {
    {GL, "GL"},
    {GP, "GP"},
    {SP, "SP"},
    {ST, "ST"},
    {FA, "FA"},
    {SL, "SL"}};

    
    


enum LipidLevel {NO_LEVEL,
    UNDEFINED_LEVEL,
    CATEGORY, // Mediators, Glycerolipids, Glycerophospholipids, Sphingolipids, Steroids, Prenols
    CLASS, // Glyerophospholipids -> Glycerophosphoinositols (PI)
    SPECIES, // Phosphatidylinositol (16:0) or PI(16:0)
    MOLECULAR_SUBSPECIES, // Phosphatidylinositol (8:0-8:0) or PI(8:0-8:0)
    STRUCTURAL_SUBSPECIES, // Phosphatidylinositol (8:0/8:0) or PI(8:0/8:0)
    ISOMERIC_SUBSPECIES // e.g. Phosphatidylethanolamine (P-18:0/22:6(4Z,7Z,10Z,13Z,16Z,19Z))
};



struct LipidClassMeta {
    LipidCategory lipid_category;
    string class_name;
    vector<string> synonyms;
};




enum LipidClass {NO_CLASS, UNDEFINED_CLASS, FA1 , FA2 , FA3 , FA4 , FA5 , FA6 , FA7 , FA8 , FA9 , FA10 , FA11 , FA12 , FA13 , FA14 , FA15 , FA16 , FA17 , FA18 , FA19 , FA20 , FA21 , FA22 , FA23 , FA24 , FA25 , FA26 , FA27 , FA28 , FA29 , FA30 , FA31 , FA32 , FA33 , FA34 , FA35 , FA36 , FA37 , FA38 , FA39 , FA40 , FA41 , FA42 , FA43 , FA44 , FA45 , FA46 , FA47 , FA48 , FA49 , FA50 , FA51 , FA52 , FA53 , FA54 , FA55 , FA56 , FA57 , FA58 , FA59 , FA60 , FA61 , FA62 , MG , DG , TG , MGDG , DGDG , SQMG , SQDG , BMP , CDPDAG , CL , MLCL , PA , LPA , PC , LPC , PE , PET , LPE , PG , LPG , PGP , PI , LPI , PIP , PIP_3p , PIP_4p , PIP_5p , PIP2 , PIP2_3p_4p , PIP2_3p_5p , PIP3 , PIP3_3p_4p_5p , PS , LPS , PIM1 , PIM2 , PIM3 , PIM4 , PIM5 , PIM6 , GLCDG , PENME2 , AC2SGL , DAT , PENME , PT , GLCGP , NAPE , LPIM1 , LPIM2 , LPIM3 , LPIM4 , LPIM5 , LPIM6 , CPA , CER , CERP , C1P , SM , HEXCER , HEX2CER , HEX3CER , FMC5 , FMC6 , LACCER , SHEXCER , LCB , LCBP , LHexCer , EPC , GB4 , GD3 , GM3 , GM4 , IPC , LSM , MIP2C , MIPC , STT , SE , CH , CHE , PAT16 , PAT18, SLBPA, LBPA, PPA, GL_6_AC_GlC_GP, PNC, PNE, DLCL};



typedef map<LipidClass, LipidClassMeta> ClassMap;
enum LipidFaBondType { NO_FA, UNDEFINED_FA, ESTER, ETHER_PLASMANYL, ETHER_PLASMENYL};


static const ClassMap lipid_classes = {
//{NO_CLASS, {NO_CATEGORY, "no class", {"no lipid class"} } },
{UNDEFINED_CLASS, {UNDEFINED_CATEGORY, "UNDEFINED", {"Undefined lipid class"} } },
{FA1, {FA, "Fatty acids and conjugates [FA01]", {"FA"} } },
{FA2, {FA, "Fatty acids and conjugates [FA01]", {"10-HDoHE"} } },
{FA3, {FA, "Fatty acids and conjugates [FA01]", {"11-HDoHE"} } },
{FA4, {FA, "Fatty acids and conjugates [FA01]", {"11-HETE"} } },
{FA5, {FA, "Fatty acids and conjugates [FA01]", {"11,12-DHET"} } },
{FA6, {FA, "Fatty acids and conjugates [FA01]", {"11(12)-EET"} } },
{FA7, {FA, "Fatty acids and conjugates [FA01]",  {"12-HEPE"} } },
{FA8, {FA, "Fatty acids and conjugates [FA01]", {"12-HETE"} } },
{FA9, {FA, "Fatty acids and conjugates [FA01]", {"12-HHTrE"} } },
{FA10, {FA, "Fatty acids and conjugates [FA01]", {"12-OxoETE"} } },
{FA11, {FA, "Fatty acids and conjugates [FA01]", {"12(13)-EpOME"} } },
{FA12, {FA, "Fatty acids and conjugates [FA01]", {"13-HODE"} } },
{FA13, {FA, "Fatty acids and conjugates [FA01]", {"13-HOTrE"} } },
{FA14, {FA, "Fatty acids and conjugates [FA01]", {"14,15-DHET"} } },
{FA15, {FA, "Fatty acids and conjugates [FA01]", {"14(15)-EET"} } },
{FA16, {FA, "Fatty acids and conjugates [FA01]", {"14(15)-EpETE"} } },
{FA17, {FA, "Fatty acids and conjugates [FA01]", {"15-HEPE"} } },
{FA18, {FA, "Fatty acids and conjugates [FA01]", {"15-HETE"} } },
{FA19, {FA, "Fatty acids and conjugates [FA01]", {"15d-PGJ2"} } },
{FA20, {FA, "Fatty acids and conjugates [FA01]", {"16-HDoHE"} } },
{FA21, {FA, "Fatty acids and conjugates [FA01]", {"16-HETE"} } },
{FA22, {FA, "Fatty acids and conjugates [FA01]", {"18-HEPE"} } },
{FA23, {FA, "Fatty acids and conjugates [FA01]", {"5-HEPE"} } },
{FA24, {FA, "Fatty acids and conjugates [FA01]", {"5-HETE"} } },
{FA25, {FA, "Fatty acids and conjugates [FA01]", {"5-HpETE"} } },
{FA26, {FA, "Fatty acids and conjugates [FA01]", {"5-OxoETE"} } },
{FA27, {FA, "Fatty acids and conjugates [FA01]", {"5,12-DiHETE"} } },
{FA28, {FA, "Fatty acids and conjugates [FA01]", {"5,6-DiHETE"} } },
{FA29, {FA, "Fatty acids and conjugates [FA01]", {"5,6,15-LXA4"} } },
{FA30, {FA, "Fatty acids and conjugates [FA01]", {"5(6)-EET"} } },
{FA31, {FA, "Fatty acids and conjugates [FA01]", {"8-HDoHE"} } },
{FA32, {FA, "Fatty acids and conjugates [FA01]", {"8-HETE"} } },
{FA33, {FA, "Fatty acids and conjugates [FA01]", {"8,9-DHET"} } },
{FA34, {FA, "Fatty acids and conjugates [FA01]", {"8(9)-EET"} } },
{FA35, {FA, "Fatty acids and conjugates [FA01]", {"9-HEPE"} } },
{FA36, {FA, "Fatty acids and conjugates [FA01]", {"9-HETE"} } },
{FA37, {FA, "Fatty acids and conjugates [FA01]", {"9-HODE"} } },
{FA38, {FA, "Fatty acids and conjugates [FA01]", {"9-HOTrE"} } },
{FA39, {FA, "Fatty acids and conjugates [FA01]", {"9(10)-EpOME"} } },
{FA40, {FA, "Fatty acids and conjugates [FA01]", {"alpha-LA"} } },
{FA41, {FA, "Fatty acids and conjugates [FA01]", {"DHA"} } },
{FA42, {FA, "Fatty acids and conjugates [FA01]", {"EPA"} } },
{FA43, {FA, "Fatty acids and conjugates [FA01]", {"Linoleic acid"} } },
{FA44, {FA, "Fatty acids and conjugates [FA01]", {"LTB4"} } },
{FA45, {FA, "Fatty acids and conjugates [FA01]", {"LTC4"} } },
{FA46, {FA, "Fatty acids and conjugates [FA01]", {"LTD4"} } },
{FA47, {FA, "Fatty acids and conjugates [FA01]", {"Maresin 1"} } },
{FA48, {FA, "Fatty acids and conjugates [FA01]", {"Palmitic acid"} } },
{FA49, {FA, "Fatty acids and conjugates [FA01]", {"PGB2"} } },
{FA50, {FA, "Fatty acids and conjugates [FA01]", {"PGD2"} } },
{FA51, {FA, "Fatty acids and conjugates [FA01]", {"PGE2"} } },
{FA52, {FA, "Fatty acids and conjugates [FA01]", {"PGF2alpha"} } },
{FA53, {FA, "Fatty acids and conjugates [FA01]", {"PGI2"} } },
{FA54, {FA, "Fatty acids and conjugates [FA01]", {"Resolvin D1"} } },
{FA55, {FA, "Fatty acids and conjugates [FA01]", {"Resolvin D2"} } },
{FA56, {FA, "Fatty acids and conjugates [FA01]", {"Resolvin D3"} } },
{FA57, {FA, "Fatty acids and conjugates [FA01]", {"Resolvin D5"} } },
{FA58, {FA, "Fatty acids and conjugates [FA01]", {"tetranor-12-HETE"} } },
{FA59, {FA, "Fatty acids and conjugates [FA01]", {"TXB1"} } },
{FA60, {FA, "Fatty acids and conjugates [FA01]", {"TXB2"} } },
{FA61, {FA, "Fatty acids and conjugates [FA01]", {"TXB3"} } },
{FA62, {FA, "Fatty acids and conjugates [FA01]", {"AA", "Arachidonic acid", "Arachidonic acid"} } },
{MG, {GL, "Monoradylglycerols [GL01]", {"MAG", "MG"} } },
{DG, {GL, "Diradylglycerols [GL02]", {"DAG", "DG"} } },
{TG, {GL, "Triradylglycerols [GL03]", {"TAG", "TG"} } },
{MGDG, {GL, "Glycosyldiradylglycerols [GL05]", {"MGDG"} } },
{DGDG, {GL, "Glycosyldiradylglycerols [GL05]", {"DGDG"} } },
{SQMG, {GL, "Glycosylmonoradylglycerols [GL04]", {"SQMG"} } },
{SQDG, {GL, "Glycosyldiradylglycerols [GL05]", {"SQDG"} } },
{BMP, {GP, "Monoacylglycerophosphomonoradylglycerols [GP0410]", {"BMP", "LBPA"} } },
{CDPDAG, {GP, "CDP-Glycerols [GP13]", {"CDPDAG", "CDP-DG"} } },
{CL, {GP, "Glycerophosphoglycerophosphoglycerols [GP12]", {"CL"} } },
{MLCL, {GP, "Glycerophosphoglycerophosphoglycerols [GP12]", {"MLCL"} } },
{DLCL, {GP, "Glycerophosphoglycerophosphoglycerols [GP12]", {"DLCL"} } },
{LPA, {GP, "Glycerophosphates [GP10]", {"LPA"} } },
{PC, {GP, "Glycerophosphocholines [GP01]", {"PC"} } },
{LPC, {GP, "Glycerophosphocholines [GP01]", {"LPC", "LysoPC"} } },
{PE, {GP, "Glycerophosphoethanolamines [GP02]", {"PE"} } },
{PET, {GP, "Glycerophosphoethanolamines [GP02]", {"PEt"} } },
{LPE, {GP, "Glycerophosphoethanolamines [GP02]", {"LPE", "LysoPE"} } },
{PG, {GP, "Glycerophosphoglycerols [GP04]", {"PG"} } },
{LPG, {GP, "Glycerophosphoglycerols [GP04]", {"LPG"} } },
{PGP, {GP, "Glycerophosphoglycerophosphates [GP05]", {"PGP"} } },
{PA, {GP, "Glycerophosphates [GP10]", {"PA"} } },
{PI, {GP, "Glycerophosphoinositols [GP06]", {"PI"} } },
{LPI, {GP, "Glycerophosphoinositols [GP06]", {"LPI"} } },
{PIP, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP"} } },
{PIP_3p, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP[3']"} } },
{PIP_4p, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP[4']"} } },
{PIP_5p, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP[5']"} } },
{PIP2, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2"} } },
{PIP2_3p_4p, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2[3',4']"} } },
{PIP2_3p_5p, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2[3',5']"} } },
{PIP3, {GP, "Glycerophosphoinositol trisphosphates [GP09]", {"PIP3"} } },
{PIP3_3p_4p_5p, {GP, "Glycerophosphoinositol trisphosphates [GP09]", {"PIP3[3',4',5']"} } },
{PS, {GP, "Glycerophosphoserines [GP03]", {"PS"} } },
{LPS, {GP, "Glycerophosphoserines [GP03]", {"LPS"} } },
{PIM1, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM1"} } },
{PIM2, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM2"} } },
{PIM3, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM3"} } },
{PIM4, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM4"} } },
{PIM5, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM5"} } },
{PIM6, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM6"} } },
{LBPA, {GP, "Glycerophosphoglycerols [GP04]", {"LBPA"} } },
{PPA, {GP, "Glyceropyrophosphates [GP11]", {"PPA"} } },
{GL_6_AC_GlC_GP, {GP, "Glycosylglycerophospholipids [GP14]", {"6-Ac-Glc-GP"} } },
{PNC, {GP, "Glycerophosphonocholines [GP16]", {"PnC"} } },
{PNE, {GP, "Glycerophosphoinositolglycans [GP15]", {"PnE"} } },
{PT, {GP, "Glycerophosphoinositolglycans [GP15]", {"PT"} } },
{GLCDG, {GP, "Glycosyldiradylglycerols [GL05]", {"Glc-DG"} } },
{PENME2, {GP, "Glycerophosphoethanolamines [GP02]", {"PE-NMe2"} } },
{PENME, {GP, "Glycerophosphoethanolamines [GP02]", {"PE-NMe"} } },
{GLCGP, {GP, "Glycosylglycerophospholipids [GP14]", {"Glc-GP"} } },
{NAPE, {GP, "Glycerophosphoethanolamines [GP02]", {"NAPE"} } },
{LPIM1, {GP, "Glycerophosphoinositolglycans [GP15]", {"LPIM1"} } },
{LPIM2, {GP, "Glycerophosphoinositolglycans [GP15]", {"LPIM2"} } },
{LPIM3, {GP, "Glycerophosphoinositolglycans [GP15]", {"LPIM3"} } },
{LPIM4, {GP, "Glycerophosphoinositolglycans [GP15]", {"LPIM4"} } },
{LPIM5, {GP, "Glycerophosphoinositolglycans [GP15]", {"LPIM5"} } },
{LPIM6, {GP, "Glycerophosphoinositolglycans [GP15]", {"LPIM6"} } },
{CPA, {GP, "Glycerophosphoinositolglycans [GP15]", {"CPA"} } },
{SLBPA, {GP, "Glycerophosphoglycerols [GP04]", {"SLBPA"} } },
{CER, {SP, "Ceramides [SP02]", {"Cer"} } },
{CERP, {SP, "Ceramides [SP02]", {"CerP"} } },
{C1P, {SP, "Ceramide-1-phosphates [SP0205]", {"C1P"} } },
{SM, {SP, "Phosphosphingolipids [SP03]", {"SM"} } },
{HEXCER, {SP, "Neutral glycosphingolipids [SP05]", {"HexCer", "GalCer", "GlcCer"} } },
{HEX2CER, {SP, "Neutral glycosphingolipids [SP05]", {"Hex2Cer", "LacCer"} } },
{HEX3CER, {SP, "Neutral glycosphingolipids [SP05]", {"Hex3Cer", "GB3"} } },
{FMC5, {SP, "Neutral glycosphingolipids [SP05]", {"FMC-5"} } },
{FMC6, {SP, "Neutral glycosphingolipids [SP05]", {"FMC-6"} } },
{LACCER, {SP, "Neutral glycosphingolipids [SP05]", {"LacCer"} } },
{SHEXCER, {SP, "Acidic glycosphingolipids [SP06]", {"SHexCer", "(3'-sulfo)Galbeta-Cer"} } },
{LCB, {SP, "Sphingoid bases [SP01]", {"LCB", "Sphingosine", "So", "Sphingosine-1-phosphate", "SPH"} } },
{LCBP, {SP, "Sphingoid bases [SP01]", {"LCBP", "Sphinganine", "Sa", "Sphingosine-1-phosphate", "S1P", "SPH-P"} } },
{LHexCer, {SP, "Hexosylsphingosine", {"LHexCer", "HexSph"} } },
{EPC, {SP, "Phosphosphingolipids [SP03]", {"EPC", "PE-Cer"} } },
{GB4, {SP, "Neutral glycosphingolipids [SP05]", {"GB4"} } },
{GD3, {SP, "Acidic glycosphingolipids [SP06]", {"GD3"} } },
{GM3, {SP, "Acidic glycosphingolipids [SP06]", {"GM3"} } },
{GM4, {SP, "Acidic glycosphingolipids [SP06]", {"GM4"} } },
{IPC, {SP, "Phosphosphingolipids [SP03]", {"IPC", "PI-Cer"} } },
{LSM, {SP, "Ceramides [SP02]", {"LSM", "SPC"} } },
{MIP2C, {SP, "Phosphosphingolipids [SP03]", {"M(IP)2C"} } },
{MIPC, {SP, "Phosphosphingolipids [SP03]", {"MIPC"} } },
{STT, {ST, "Sterols [ST01]", {"ST"} } },
{SE, {ST, "Steryl esters [ST0102]", {"SE"} } },
{CH, {ST, "Cholesterol [LMST01010001]", {"CH", "FC", "Cholesterol"} } },
{CHE, {ST, "Cholesteryl esters [ST0102]", {"ChE", "CE", "Cholesteryl ester", "Cholesterol ester"} } },
{AC2SGL, {SL, "Acyltrehaloses [SL03]", {"AC2SGL"} } },
{PAT16, {SL, "Acyltrehaloses [SL03]", {"PAT16"} } },
{PAT18, {SL, "Acyltrehaloses [SL03]", {"PAT18"} } },
{DAT, {SL, "Acyltrehaloses [SL03]", {"DAT"} } }
};



static map<LipidClass, string> ClassString;
static map<string, LipidClass> StringClass;
static map<string, LipidCategory> StringCategory;
    

#endif /* LIPID_ENUMS_H */
