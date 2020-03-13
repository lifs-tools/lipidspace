#include "StructuralFattyAcid.h"


StructuralFattyAcid::StructuralFattyAcid(string name, int num_carbon, int num_double_bonds, int num_hydroxyl, LipidFaBondType lipid_FA_bond_type, bool lcb, int position) : MolecularFattyAcid(name, num_carbon, num_double_bonds, num_hydroxyl, lipid_FA_bond_type, lcb, position){
    
}

StructuralFattyAcid::StructuralFattyAcid(FattyAcid* fa) : MolecularFattyAcid(fa){
    
}

StructuralFattyAcid::~StructuralFattyAcid(){
    
}


string StructuralFattyAcid::to_string(bool special_case, LipidLevel level){
    switch(level){
        case NO_LEVEL:            
        case STRUCTURAL_SUBSPECIES:
            return FattyAcid::to_string(special_case);
            
        case MOLECULAR_SUBSPECIES:
        case SPECIES:
            return MolecularFattyAcid::to_string(special_case, level);
            
        default:
            throw IllegalArgumentException("StructuralFattyAcid does not know how to create a fatty acid string for level " + to_string(level));
    }
}