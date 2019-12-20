#include "LipidSpeciesInfo.h"

LipidSpeciesInfo::LipidSpeciesInfo (FattyAcid *fa){
    level = UNDEFINED_LEVEL;
    num_carbon = (fa) ? fa->num_carbon : 0;
    num_hydroxyl = (fa) ? fa->num_hydroxyl : 0;
    lipid_FA_bond_type = (fa) ? fa->lipid_FA_bond_type : UNDEFINED_FA;
    num_double_bonds = 0;
    
    try {
        num_double_bonds = ((MolecularFattyAcid*)fa)->num_double_bonds;
    }
    catch(const std::bad_cast& e) {
        
    }
}