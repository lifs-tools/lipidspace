#include "LipidSpeciesInfo.h"

LipidSpeciesInfo::LipidSpeciesInfo (FattyAcid *fa) : MolecularFattyAcid("", (fa) ? fa->num_carbon : 2, 0, (fa) ? fa->num_hydroxyl : 0, (fa) ? fa->lipid_FA_bond_type : UNDEFINED_FA, (fa) ? fa->lcb : false, (fa) ? fa->position : -1) {
    level = UNDEFINED_LEVEL;
    
    try {
        num_double_bonds = ((MolecularFattyAcid*)fa)->num_double_bonds;
    }
    catch(const std::bad_cast& e) {
        
    }
}
