#include "MolecularFattyAcid.h"


MolecularFattyAcid::MolecularFattyAcid(string name, int num_carbon, int _num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position) : FattyAcid(name, num_carbon, num_hydroxyl, lipid_FA_bond_type, lcb, position){
    
    if (_num_double_bonds < 0){
        throw ConstraintViolationException("MolecularFattyAcid must have at least 0 double bonds!");
    }
        
    num_double_bonds = _num_double_bonds;
}

MolecularFattyAcid::~MolecularFattyAcid(){
    
}

int MolecularFattyAcid::get_num_double_bonds(){
    return num_double_bonds;
}