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



#ifndef LIPID_ENUMS_H
#define LIPID_ENUMS_H


#include <vector>
#include <map>
#include <set>
#include "cppgoslin/domain/Element.h"


namespace goslin {
using namespace std;


enum LipidCategory {NO_CATEGORY,
    UNDEFINED,
    GL, // SLM:000117142 Glycerolipids
    GP, // SLM:000001193 Glycerophospholipids
    SP, // SLM:000000525 Sphingolipids
    ST, // SLM:000500463 Steroids and derivatives
    FA, // SLM:000390054 Fatty acyls and derivatives
    PK, // polyketides
    SL // Saccharo lipids
};


static const map<LipidCategory, string> CategoryString = {
    {NO_CATEGORY, "NO_CATEGORY"},
    {UNDEFINED, "UNDEFINED"},
    {GL, "GL"},
    {GP, "GP"},
    {SP, "SP"},
    {ST, "ST"},
    {FA, "FA"},
    {SL, "SL"}
};

    
    


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
    int max_num_fa;
    set<int> possible_num_fa;
    ElementTable elements;
    vector<string> synonyms;
};

enum LipidClass {NO_CLASS, UNDEFINED_CLASS, AA, AC2SGL, AC_O_9_GD1A, AC_O_9_GT1B, AC_O_9_GT3, ALPHA_LA, ANACARD, BMP, BRANCHED_FORSSMAN, CAR, CATECHOL, CDPDAG, CER, CERP, CL, COA, CPA, DAG, DAT, DGCC, DGDG, DHA, DHDG, DLCL, DMPE, DSGG, EPA, EPC, FAA, FAHFA, FMC_5, FMC_6, FORSSMAN, FUCGALGB3CER, FUC_BRANCHED_FORSSMAN, FUC_GA1, FUC_GAL_GAL_IGB4CER, FUC_GAL__GM1, FUC_GD1B, FUC_GM1, FUC_GM1_NEUGC_, FUC_IGB3CER, GA1, GA2, GALGALGALGB3CER, GALGALGB3CER, GALGALNAC_GM1B_NEUGC_, GALGAL_GD1B, GALGB3CER, GALGB4CER, GALGLCNAC_GALGB4CER, GALNACGALGB3CER, GALNACGAL_FUC__GA1, GALNAC_GD1A, GALNAC_GD1A_NEUAC_NEUGC_, GALNAC_GD1A_NEUGC_NEUAC_, GALNAC_GM1, GALNAC_GM1B, GALNAC_GM1B_NEUGC_, GAL_FUC__GA1, GAL_FUC__GD1B, GAL_GD1B, GAL_IGB4CER, GB3, GB3CER, GB4, GB4CER, GD1, GD1A, GD1A_ALPHA, GD1A_NEUAC_NEUGC_, GD1A_NEUGC_NEUAC_, GD1A_NEUGC_NEUGC_, GD1B, GD1C, GD1C_NEUGC_NEUGC_, GD2, GD3, GLCNACGB3CER, GLCNAC_GALGB4CER, GLC_DG, GLC_GP, GLOBO_A, GLOBO_B, GLOBO_H, GLOBO_LEX_9, GM1, GM1B, GM1B_NEUGC_, GM1_ALPHA, GM1_NEUGC_, GM2, GM2_NEUGC_, GM3, GM4, GP1, GP1C, GP1C_ALPHA, GP_NAE, GQ1, GQ1B, GQ1B_ALPHA, GQ1C, GT1, GT1A, GT1A_ALPHA, GT1B, GT1B_ALPHA, GT1B_ALPHA_NEUGC_, GT1C, GT2, GT3, HEX2CER, HEX3CER, HEXCER, IGB3CER, IGB4CER, IPC, I_FORSSMAN, L10_HDOHE, L11_12_DHET, L11_12__EET, L11_HDOHE, L11_HETE, L12_13__EPOME, L12_HEPE, L12_HETE, L12_HHTRE, L12_OXOETE, L13_HODE, L13_HOTRE, L14_15_DHET, L14_15__EET, L14_15__EPETE, L15D_PGJ2, L15_HEPE, L15_HETE, L16_HDOHE, L16_HETE, L18_HEPE, L1_O_BEHENOYL_CER, L1_O_CARBOCEROYL_CER, L1_O_CEROTOYL_CER, L1_O_EICOSANOYL_CER, L1_O_LIGNOCEROYL_CER, L1_O_MYRISTOYL_CER, L1_O_PALMITOYL_CER, L1_O_STEAROYL_CER, L1_O_TRICOSANOYL_CER, L5_12_DIHETE, L5_6_15_LXA4, L5_6_DIHETE, L5_6__EET, L5_HEPE, L5_HETE, L5_HPETE, L5_OXOETE, L6_AC_GLC_GP, L8_9_DHET, L8_9__EET, L8_HDOHE, L8_HETE, L9_10__EPOME, L9_HEPE, L9_HETE, L9_HODE, L9_HOTRE, LBPA, LCB, LCBP, LCDPDAG, LDMPE, LEX_GM1, LHEXCER, LINOLEIC_ACID, LMMPE, LPA, LPC, LPE, LPG, LPI, LPIM1, LPIM2, LPIM3, LPIM4, LPIM5, LPIM6, LPIMIP, LPIN, LPS, LSM, LTB4, LTC4, LTD4, L_3__SULFO_LACCER, L_FUC_IGB3CER, MAG, MARESIN_1, MGDG, MHDG, MIPC, MLCL, MMPE, MSGG, M_IP_2C, NAE, NAPE, NEUACGAL_IGB4CER, NEUAC_ALPHA2_6__MSGG, NEUAC_ALPHA2_8__MSGG, NEUGCNEUGC_GALGB4CER, NEUGC_GALGB4CER, NEUGC_LACNAC_GM1_NEUGC_, NOR1, NOR2, NORINT, PA, PALMITIC_ACID, PARA_FORSSMAN, PAT16, PAT18, PC, PE, PET, PE_GLCDG, PE_NME, PE_NME2, PG, PGB2, PGD2, PGE2, PGF2ALPHA, PGI2, PGP, PHENOL, PI, PIM1, PIM2, PIM3, PIM4, PIM5, PIM6, PIMIP, PIP, PIP2, PIP2_3__4__, PIP2_3__5__, PIP2_4__5__, PIP3, PIP3_3__4__5__, PIP_3__, PIP_4__, PIP_5__, PNC, PNE, PPA, PS, PS_NAC, PT, RESOLVIN_D1, RESOLVIN_D2, RESOLVIN_D3, RESOLVIN_D5, RESORCINOL, SB1A, SE, SE_27_1, SE_27_2, SE_28_2, SE_28_3, SE_29_2, SE_30_2, SHEX2CER, SHEXCER, SLBPA, SM, SM1A, SM1B, SO3_GALGB4CER, SO3_GAL_IGB4CER, SO3_GB4CER, SO3_GM1_NEUGC_, SO3_IGB4CER, SQDG, SQMG, STA, ST_27_1_1, ST_27_2_1, ST_28_2_1, ST_28_3_1, ST_29_2_1, ST_30_2_1, SULFOGALCER, TAG, TETRANOR_12_HETE, TXB1, TXB2, TXB3, UNDEFINEDA, WE};



typedef map<LipidClass, LipidClassMeta> ClassMap;
enum LipidFaBondType { NO_FA, UNDEFINED_FA, ESTER, ETHER_PLASMANYL, ETHER_PLASMENYL};


class LipidClasses {
    public:
        static LipidClasses& get_instance()
        {
            static LipidClasses instance;
            return instance;
        }
    private:
        LipidClasses();
        
    public:
        ClassMap lipid_classes;
        LipidClasses(LipidClasses const&) = delete;
        void operator=(LipidClasses const&) = delete;
};



static map<LipidClass, string> ClassString;
static map<string, LipidClass> StringClass;
static map<string, LipidCategory> StringCategory;
}

#endif /* LIPID_ENUMS_H */
