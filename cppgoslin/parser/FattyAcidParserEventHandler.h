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

#ifndef FATTY_ACID_PARSER_EVENT_HANDLER_H
#define FATTY_ACID_PARSER_EVENT_HANDLER_H

#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/Adduct.h"
#include "cppgoslin/domain/Cycle.h"
#include "cppgoslin/domain/LipidAdduct.h"
#include "cppgoslin/domain/LipidStructuralSubspecies.h"
#include "cppgoslin/domain/LipidIsomericSubspecies.h"
#include "cppgoslin/domain/FattyAcid.h"
#include "cppgoslin/domain/Headgroup.h"
#include "cppgoslin/domain/FunctionalGroup.h"
#include "cppgoslin/domain/GenericDatastructures.h"
#include "cppgoslin/parser/BaseParserEventHandler.h"
#include <string>
#include <math.h>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;
using namespace goslin;


/*
last_numbers = {'un': 1, 'hen': 1, 'do': 2, 'di': 2, 'tri': 3, 'buta': 4, 'but': 4, 'tetra': 4, 'penta': 5, 'pent': 5, 'hexa': 6, 'hex': 6, 'hepta': 7, 'hept': 7, 'octa': 8, 'oct': 8, 'nona': 9, 'non': 9}

second_numbers = {'deca': 10, 'dec': 10, 'eicosa': 20, 'eicos': 20 , 'cosa': 20, 'cos': 20, 'triaconta': 30, 'triacont': 30, 'tetraconta': 40, 'tetracont': 40, 'pentaconta': 50, 'pentacont': 50, 'hexaconta': 60, 'hexacont': 60, 'heptaconta': 70, 'heptacont': 70, 'octaconta': 80, 'octacont': 80, 'nonaconta': 90, 'nonacont': 90}

special_numbers = {'meth': 1, 'etha': 2, 'eth': 2,  'propa': 3, 'isoprop': 3, 'prop': 3, 'propi': 3, 'propio': 3, 'buta': 4, 'but': 4, 'butr': 4, 'valer': 5, 'eicosa': 20, 'eicos': 20, 'icosa': 20, 'icos': 20, 'prosta': 20, 'prost': 20, 'prostan': 20}

func_groups = {'keto': 'oxo', 'ethyl': 'Et', 'hydroxy': "OH", 'phospho': 'Ph', 'oxo': 'oxo', 'bromo': 'Br', 'methyl': 'Me', 'hydroperoxy': 'OOH', 'homo': '', 'Epoxy': 'Ep', 'fluro': 'F', 'fluoro': 'F', 'chloro': 'Cl', 'methylene': 'My', 'sulfooxy': 'Su', 'amino': 'NH2', 'sulfanyl': 'SH', 'methoxy': 'OMe', 'iodo': 'I', 'cyano': 'CN', 'nitro': 'NO2', 'OH': 'OH', 'thio': 'SH', 'mercapto': 'SH', 'carboxy': "COOH", 'acetoxy': 'Ac', 'cysteinyl': 'Cys', 'phenyl': 'Phe', 's-glutathionyl': "SGlu", 's-cysteinyl': "SCys", "butylperoxy": "BOO", "dimethylarsinoyl": "MMAs", "methylsulfanyl": "SMe", "imino": "NH", 's-cysteinylglycinyl': "SCG"}

ate = {'formate': 1, 'acetate': 2, 'butyrate': 4, 'propionate': 3, 'valerate': 5, 'isobutyrate': 4}
*/

class FattyAcidParserEventHandler : public BaseParserEventHandler<LipidAdduct*> {
public:
    
    
    FattyAcidParserEventHandler();
    ~FattyAcidParserEventHandler();
    void reset_lipid(TreeNode *node);
    void build_lipid(TreeNode *node);
    void set_fatty_acid(TreeNode *node);
    void set_fatty_acyl_type(TreeNode *node);
    void set_double_bond_information(TreeNode *node);
    void add_double_bond_information(TreeNode *node);
    void set_double_bond_position(TreeNode *node);
    void set_cistrans(TreeNode *node);
    void check_db(TreeNode *node);
    void reset_length(TreeNode *node);
    void set_functional_length(TreeNode *node);
    void set_fatty_length(TreeNode *node);
    void special_number(TreeNode *node);
    void last_number(TreeNode *node);
    void second_number(TreeNode *node);
    void set_functional_group(TreeNode *node);
    void add_functional_group(TreeNode *node);
    void set_functional_pos(TreeNode *node);
    void set_functional_position(TreeNode *node);
    void set_functional_type(TreeNode *node);
    void rearrange_cycle(TreeNode *node);
    void add_epoxy(TreeNode *node);
    void set_cycle(TreeNode *node);
    void set_methylene(TreeNode *node);
    void set_dioic(TreeNode *node);
    void set_dial(TreeNode *node);
    void set_prosta(TreeNode *node);
    void add_cyclo(TreeNode *node);
    void reduction(TreeNode *node);
    void homo(TreeNode *node);
    void set_recursion(TreeNode *node);
    void add_recursion(TreeNode *node);
    void set_recursion_pos(TreeNode *node);
    void set_yl_ending(TreeNode *node);
    void set_acetic_acid(TreeNode *node);
    void add_hydroxyl(TreeNode *node);
    void setup_hydroxyl(TreeNode *node);
    void add_hydroxyls(TreeNode *node);
    void add_wax_ester(TreeNode *node);
    void set_ate(TreeNode *node);
    void set_iso(TreeNode *node);
    void set_coa(TreeNode *node);
    void set_methyl(TreeNode *node);
    void set_car(TreeNode *node);
    void add_car(TreeNode *node);
    void add_ethanolamine(TreeNode *node);
    void add_amine(TreeNode *node);
    void add_amine_name(TreeNode *node);
    void add_summary(TreeNode *node);
    void add_func_stereo(TreeNode *node);
};
        
        
#endif /* FATTY_ACID_PARSER_EVENT_HANDLER_H */
