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




enum LipidClass {NO_CLASS, UNDEFINED_CLASS, FA1 , FA2 , FA3 , FA4 , FA5 , FA6 , FA7 , FA8 , FA9 , FA10 , FA11 , FA12 , FA13 , FA14 , FA15 , FA16 , FA17 , FA18 , FA19 , FA20 , FA21 , FA22 , FA23 , FA24 , FA25 , FA26 , FA27 , FA28 , FA29 , FA30 , FA31 , FA32 , FA33 , FA34 , FA35 , FA36 , FA37 , FA38 , FA39 , FA40 , FA41 , FA42 , FA43 , FA44 , FA45 , FA46 , FA47 , FA48 , FA49 , FA50 , FA51 , FA52 , FA53 , FA54 , FA55 , FA56 , FA57 , FA58 , FA59 , FA60 , FA61 , FA62 , MG , DG , TG , MGDG , DGDG , SQMG , SQDG , BMP , CDPDAG , CL , MLCL , PA , LPA , PC , LPC , PE , PET , LPE , PG , LPG , PGP , PI , LPI , PIP , PIP_3p , PIP_4p , PIP_5p , PIP2 , PIP2_3p_4p , PIP2_3p_5p , PIP2_4p_5p , PIP3 , PIP3_3p_4p_5p , PS , LPS , PIM1 , PIM2 , PIM3 , PIM4 , PIM5 , PIM6 , GLCDG , PENME2 , AC2SGL , DAT , PENME , PT , GLCGP , NAPE , LPIM1 , LPIM2 , LPIM3 , LPIM4 , LPIM5 , LPIM6 , CPA , CER , CERP , C1P , SM , HEXCER , HEX2CER , HEX3CER , FMC5 , FMC6 , SHEXCER , LCB , LCBP , LHEXCER , EPC , GB4 , GD3 , GM3 , GM4 , IPC , LSM , MIP2C , MIPC , STT , SE , CH , CHE , PAT16 , PAT18, SLBPA, PPA, GL_6_AC_GlC_GP, PNC, PNE, DLCL, LCDPDAG, LDMPE, LMMPE, LPIMIP, LPIN, PIMIP, DMPE, NAE, GD1A_ALPHA, FUC_GAL_GM1, SULFOGALCER, GD1A, GM1B, GT1B, GQ1B, GT1A, GT1A_ALPHA, GQ1B_ALPHA, T_SULFO_LACCER, GP1C_ALPHA, GQ1C, GP1C, GD1C, GD1B, GT1C, GP_NAE, GB3CER, GB4CER, FORSSMAN, GALGB4CER, MSGG, DSGG, NEUAC_6_MSGG, NEUAC_8_MSGG, NOR1, NORINT, NOR2, GLOBO_H, GLOBO_A, AC_O9_GT3, GT1B_ALPHA_NEUGC, GT1B_ALPHA, AC_O9_GT1B, GM2_NEUGC, GALGALNAC_GM1B_NEUGC, GALNAC_GM1B_NEUGC, GM1B_NEUGC, GALNAC_GM1B, GM1_ALPHA, LEX_GM1, NEUGC_LACNAC_GM1_NEUGC, GALNAC_GM1, SO3_GM1_NEUGC, FUC_GM1_NEUGC, GM1_NEUGC, FUC_GM1, GD1C_NEUGC_NEUGC, GALGAL_GD1B, GAL_FUC_GD1B, GAL_GD1B, FUC_GD1B, GALNAC_GD1A_NEUGC_NEUAC, GALNAC_GD1A, GD1A_NEUGC_NEUGC, GD1A_NEUAC_NEUGC, GD1A_NEUGC_NEUAC, AC_O9_GD1A, SB1A, SM1B, SM1A, GALNACGAL_FUC_GA1, GAL_FUC_GA1, FUC_GA1, FUCIGB3CER, SO3_GAL_IGB4CER, SO3_IGB4CER, FUC_GAL_GAL_IGB4CER, FUC_IGB3CER, NEUACGAL_IGB4CER, GAL_IGB4CER, I_FORSSMAN, IGB4CER,IGB3CER, FUC_BRANCHED_FORSSMAN, BRANCHED_FORSSMAN, NEUGCNEUGC_GALGB4CER, NEUGC_GALGB4CER, SO3_GALGB4CER, SO3_GB4CER, GLCNACGB3CER, GLOBO_B, PARA_FORSSMAN, GALNACGALGB3CER, FUCGALGB3CER, GALGALGALGB3CER, GALGALGB3CER, GALGB3CER, GLOBO_LEX_9, GALGLCNAC_GALGB4CER, GLCNAC_GALGB4CER
    
    
};




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
{NAE, {FA, "X-ethanolamine", {"NAE"} } },
{GP_NAE, {FA, "X-ethanolamine", {"GP-NAE"} } },
{MG, {GL, "Monoradylglycerols [GL01]", {"MAG", "MG"} } },
{DG, {GL, "Diradylglycerols [GL02]", {"DAG", "DG"} } },
{TG, {GL, "Triradylglycerols [GL03]", {"TAG", "TG"} } },
{MGDG, {GL, "Glycosyldiradylglycerols [GL05]", {"MGDG"} } },
{DGDG, {GL, "Glycosyldiradylglycerols [GL05]", {"DGDG"} } },
{SQMG, {GL, "Glycosylmonoradylglycerols [GL04]", {"SQMG"} } },
{SQDG, {GL, "Glycosyldiradylglycerols [GL05]", {"SQDG"} } },
{BMP, {GP, "Monoacylglycerophosphomonoradylglycerols [GP0410]", {"BMP", "LBPA"} } },
{CDPDAG, {GP, "CDP-Glycerols [GP13]", {"CDPDAG", "CDPDG", "CDP-DG", "CDP-DAG"} } },
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
{PIP_3p, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP[3']", "PIP[3]"} } },
{PIP_4p, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP[4']", "PIP[4]"} } },
{PIP_5p, {GP, "Glycerophosphoinositol monophosphates [GP07]", {"PIP[5']", "PIP[5]"} } },
{PIP2, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2"} } },
{PIP2_3p_4p, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2[3',4']", "PIP[3,4]"} } },
{PIP2_3p_5p, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2[3',5']", "PIP[3,5]"} } },
{PIP2_4p_5p, {GP, "Glycerophosphoinositol bisphosphates [GP08]", {"PIP2[4',5']", "PIP[4,5]"} } },
{PIP3, {GP, "Glycerophosphoinositol trisphosphates [GP09]", {"PIP3"} } },
{PIP3_3p_4p_5p, {GP, "Glycerophosphoinositol trisphosphates [GP09]", {"PIP3[3',4',5']", "PIP3[3,4,5]"} } },
{PS, {GP, "Glycerophosphoserines [GP03]", {"PS"} } },
{LPS, {GP, "Glycerophosphoserines [GP03]", {"LPS"} } },
{PIM1, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM1"} } },
{PIM2, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM2"} } },
{PIM3, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM3"} } },
{PIM4, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM4"} } },
{PIM5, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM5"} } },
{PIM6, {GP, "Glycerophosphoinositolglycans [GP15]", {"PIM6"} } },
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
{HEXCER, {SP, "Neutral glycosphingolipids [SP05]", {"HexCer", "GalCer", "GlcCer", "(3'-sulfo)GalCer"} } },
{HEX2CER, {SP, "Neutral glycosphingolipids [SP05]", {"Hex2Cer", "LacCer"} } },
{T_SULFO_LACCER, {SP, "Neutral glycosphingolipids [SP05]", {"(3'-sulfo)LacCer"} } },
{HEX3CER, {SP, "Neutral glycosphingolipids [SP05]", {"Hex3Cer", "GB3"} } },
{FMC5, {SP, "Neutral glycosphingolipids [SP05]", {"FMC-5"} } },
{FMC6, {SP, "Neutral glycosphingolipids [SP05]", {"FMC-6"} } },
{SHEXCER, {SP, "Acidic glycosphingolipids [SP06]", {"SHexCer", "(3'-sulfo)Galbeta-Cer"} } },
{LCB, {SP, "Sphingoid bases [SP01]", {"LCB", "Sphingosine", "So", "Sphingosine-1-phosphate", "SPH"} } },
{LCBP, {SP, "Sphingoid bases [SP01]", {"LCBP", "Sphinganine", "Sa", "Sphingosine-1-phosphate", "S1P", "SPH-P"} } },
{LHEXCER, {SP, "Hexosylsphingosine", {"LHexCer", "HexSph"} } },
{EPC, {SP, "Phosphosphingolipids [SP03]", {"EPC", "PE-Cer"} } },
{GB4, {SP, "Neutral glycosphingolipids [SP05]", {"GB4"} } },
{GD3, {SP, "Acidic glycosphingolipids [SP06]", {"GD3"} } },
{GD1A_ALPHA, {SP, "Acidic glycosphingolipids", {"GD1a alpha"} } },
{FUC_GAL_GM1, {SP, "Acidic glycosphingolipids [SP06]", {"Fuc(Gal)-GM1"} } },
{GM3, {SP, "Acidic glycosphingolipids [SP06]", {"GM3"} } },
{SULFOGALCER, {SP, "Acidic glycosphingolipids [SP06]", {"SulfoGalCer"} } },
{GD1A, {SP, "Acidic glycosphingolipids [SP06]", {"GD1a"} } },
{GM1B, {SP, "Acidic glycosphingolipids [SP06]", {"GM1b"} } },
{GT1B, {SP, "Acidic glycosphingolipids [SP06]", {"GT1b"} } },
{GQ1B, {SP, "Acidic glycosphingolipids [SP06]", {"GQ1b"} } },
{GT1A, {SP, "Acidic glycosphingolipids [SP06]", {"GT1a"} } },
{GQ1C, {SP, "Acidic glycosphingolipids [SP06]", {"GQ1c"} } },
{GP1C, {SP, "Acidic glycosphingolipids [SP06]", {"GP1c"} } },
{GD1C, {SP, "Acidic glycosphingolipids [SP06]", {"GD1c"} } },
{GD1B, {SP, "Acidic glycosphingolipids [SP06]", {"GD1b"} } },
{GT1C, {SP, "Acidic glycosphingolipids [SP06]", {"GT1c"} } },
{GT1A_ALPHA, {SP, "Acidic glycosphingolipids [SP06]", {"GT1a alpha"} } },
{GQ1B_ALPHA, {SP, "Acidic glycosphingolipids [SP06]", {"GQ1b alpha"} } },
{GP1C_ALPHA, {SP, "Acidic glycosphingolipids [SP06]", {"GP1c alpha"} } },
{GB3CER, {SP, "Globoside", {"Gb3Cer"} } },
{GB4CER, {SP, "Globoside", {"Gb4Cer"} } },
{FORSSMAN, {SP, "Globoside", {"Forssman"} } },
{GALGB4CER, {SP, "Globoside", {"GalGb4Cer"} } },
{MSGG, {SP, "Globoside", {"MSGG"} } },
{DSGG, {SP, "Globoside", {"DSGG"} } },
{NEUAC_6_MSGG, {SP, "Globoside", {"NeuAc(alpha2-6)-MSGG"} } },
{NEUAC_8_MSGG, {SP, "Globoside", {"NeuAc(alpha2-8)-MSGG"} } },
{NOR1, {SP, "Globoside", {"NOR1"} } },
{NORINT, {SP, "Globoside", {"NORint"} } },
{NOR2, {SP, "Globoside", {"NOR2"} } },
{GLOBO_H, {SP, "Globoside", {"Globo-H"} } },
{GLOBO_A, {SP, "Globoside", {"Globo-A"} } },
{AC_O9_GT3, {SP, "Globoside", {"Ac-O-9-GT3"} } },
{GT1B_ALPHA_NEUGC, {SP, "Globoside", {"GT1b alpha(NeuGc)"} } },
{GT1B_ALPHA, {SP, "Globoside", {"GT1b alpha"} } },
{AC_O9_GT1B, {SP, "Globoside", {"Ac-O-9-GT1b"} } },
{GM2_NEUGC, {SP, "Globoside", {"GM2(NeuGc)"} } },
{GALGALNAC_GM1B_NEUGC, {SP, "Globoside", {"GalGalNAc-GM1b(NeuGc)"} } },
{GALNAC_GM1B_NEUGC, {SP, "Globoside", {"GalNAc-GM1b(NeuGc)"} } },
{GM1B_NEUGC, {SP, "Globoside", {"GM1b(NeuGc)"} } },
{GALNAC_GM1B, {SP, "Globoside", {"GalNAc-GM1b"} } },
{GM1_ALPHA, {SP, "Globoside", {"GM1 alpha"} } },
{LEX_GM1, {SP, "Globoside", {"Lex-GM1"} } },
{NEUGC_LACNAC_GM1_NEUGC, {SP, "Globoside", {"NeuGc-LacNAc-GM1(NeuGc)"} } },
{GALNAC_GM1, {SP, "Globoside", {"GalNAc-GM1"} } },
{SO3_GM1_NEUGC, {SP, "Globoside", {"SO3-GM1(NeuGc)"} } },
{FUC_GM1_NEUGC, {SP, "Globoside", {"Fuc-GM1(NeuGc)"} } },
{GM1_NEUGC, {SP, "Globoside", {"GM1(NeuGc)"} } },
{FUC_GM1, {SP, "Globoside", {"Fuc-GM1"} } },
{GD1C_NEUGC_NEUGC, {SP, "Globoside", {"GD1c(NeuGc/NeuGc)"} } },
{GALGAL_GD1B, {SP, "Globoside", {"GalGal-GD1b"} } },
{GAL_FUC_GD1B, {SP, "Globoside", {"Gal(Fuc)-GD1b"} } },
{GAL_GD1B, {SP, "Globoside", {"Gal-GD1b"} } },
{FUC_GD1B, {SP, "Globoside", {"Fuc-GD1b"} } },
{GALNAC_GD1A_NEUGC_NEUAC, {SP, "Globoside", {"GalNAc-GD1a(NeuGc/NeuAc)"} } },
{GALNAC_GD1A, {SP, "Globoside", {"GalNAc-GD1a"} } },
{GD1A_NEUGC_NEUGC, {SP, "Globoside", {"GD1a(NeuGc/NeuGc)"} } },
{GD1A_NEUAC_NEUGC, {SP, "Globoside", {"GD1a(NeuAc/NeuGc)"} } },
{GD1A_NEUGC_NEUAC, {SP, "Globoside", {"GD1a(NeuGc/NeuAc)"} } },
{AC_O9_GD1A, {SP, "Globoside", {"Ac-O-9-GD1a"} } },
{SB1A, {SP, "Globoside", {"SB1a"} } },
{SM1B, {SP, "Globoside", {"SM1b"} } },
{SM1A, {SP, "Globoside", {"SM1a"} } },
{GALNACGAL_FUC_GA1, {SP, "Globoside", {"GalNAcGal(Fuc)-GA1"} } },
{GAL_FUC_GA1, {SP, "Globoside", {"Gal(Fuc)-GA1"} } },
{FUC_GA1, {SP, "Globoside", {"Fuc-GA1"} } },
{FUCIGB3CER, {SP, "Globoside", {"(Fuc)iGb3Cer"} } },
{SO3_GAL_IGB4CER, {SP, "Globoside", {"SO3-Gal-iGb4Cer"} } },
{SO3_IGB4CER, {SP, "Globoside", {"SO3-iGb4Cer"} } },
{FUC_GAL_GAL_IGB4CER, {SP, "Globoside", {"Fuc(Gal)Gal-iGb4Cer"} } },
{FUC_IGB3CER, {SP, "Globoside", {"Fuc-iGb3Cer"} } },
{NEUACGAL_IGB4CER, {SP, "Globoside", {"NeuAcGal-iGb4Cer"} } },
{GAL_IGB4CER, {SP, "Globoside", {"Gal-iGb4Cer"} } },
{I_FORSSMAN, {SP, "Globoside", {"i-Forssman"} } },
{IGB4CER, {SP, "Globoside", {"iGb4Cer"} } },
{IGB3CER, {SP, "Globoside", {"iGb3Cer"} } },
{FUC_BRANCHED_FORSSMAN, {SP, "Globoside", {"Fuc-Branched-Forssman"} } },
{BRANCHED_FORSSMAN, {SP, "Globoside", {"Branched-Forssman"} } },
{NEUGCNEUGC_GALGB4CER, {SP, "Globoside", {"NeuGcNeuGc-GalGb4Cer"} } },
{NEUGC_GALGB4CER, {SP, "Globoside", {"NeuGc-GalGb4Cer"} } },
{SO3_GALGB4CER, {SP, "Globoside", {"SO3-GalGb4Cer"} } },
{SO3_GB4CER, {SP, "Globoside", {"SO3-Gb4Cer"} } },
{GLCNACGB3CER, {SP, "Globoside", {"GlcNAcGb3Cer"} } },
{GLOBO_B, {SP, "Globoside", {"Globo-B"} } },
{PARA_FORSSMAN, {SP, "Globoside", {"Para-Forssman"} } },
{GALNACGALGB3CER, {SP, "Globoside", {"GalNAcGalGb3Cer"} } },
{FUCGALGB3CER, {SP, "Globoside", {"FucGalGb3Cer"} } },
{GALGALGALGB3CER, {SP, "Globoside", {"GalGalGalGb3Cer"} } },
{GALGALGB3CER, {SP, "Globoside", {"GalGalGb3Cer"} } },
{GALGB3CER, {SP, "Globoside", {"GalGb3Cer"} } },
{GLOBO_LEX_9, {SP, "Globoside", {"Globo-Lex-9"} } },
{GALGLCNAC_GALGB4CER, {SP, "Globoside", {"GalGlcNAc-GalGb4Cer"} } },
{GLCNAC_GALGB4CER, {SP, "Globoside", {"GlcNAc-GalGb4Cer"} } },

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
{DAT, {SL, "Acyltrehaloses [SL03]", {"DAT"} } },
{DMPE, {GP, "Dimethylphosphatidylethanolamine", {"DMPE"} } },
{PIMIP, {GP, "Phosphatidylinositol mannoside inositol phosphate", {"PIMIP"} } },
{LCDPDAG, {GP, "Lyso-CDP-diacylglycerol", {"LCDPDAG"} } },
{LDMPE, {GP, "Lysodimethylphosphatidylethanolamine", {"LDMPE"} } },
{LMMPE, {GP, "Lysomonomethylphosphatidylethanolamine", {"LMMPE"} } },
{LPIMIP, {GP, "Lysophosphatidylinositol- mannosideinositolphosphate", {"LPIMIP"} } },
{LPIN, {GP, "Lysophosphatidylinositol-glucosamine", {"LPIN"} } }
};



static map<LipidClass, string> ClassString;
static map<string, LipidClass> StringClass;
static map<string, LipidCategory> StringCategory;
    

#endif /* LIPID_ENUMS_H */
