#include "MolecularFattyAcid.h"


MolecularFattyAcid::MolecularFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position) : FattyAcid(name, num_carbon, num_double_bonds, num_hydroxyl, lipid_FA_bond_type, lcb, position){
    
    if (num_double_bonds < 0){
        throw ConstraintViolationException("MolecularFattyAcid must have at least 0 double bonds!");
    }

}

MolecularFattyAcid::~MolecularFattyAcid(){
    
}
