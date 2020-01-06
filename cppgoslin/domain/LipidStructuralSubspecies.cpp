#include "LipidStructuralSubspecies.h"

LipidStructuralSubspecies::LipidStructuralSubspecies(string head_group) : LipidMolecularSubspecies (head_group) {
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = ESTER;
    
            
    info.level = STRUCTURAL_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
}



LipidStructuralSubspecies::LipidStructuralSubspecies(string head_group, vector<FattyAcid*> *_fa) : LipidMolecularSubspecies (head_group) {
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = ESTER;
    
    
    if (_fa != NULL){
        for (unsigned int i = 0; i < _fa->size(); ++i){
            StructuralFattyAcid *fas = (StructuralFattyAcid*)(_fa->at(i));
            if (fa.find(fas->name) != fa.end()){
                throw ConstraintViolationException("FA names must be unique! FA with name " + fas->name + " was already added!");
            }
            else {
                fa.insert({fas->name, fas});
                fa_list.push_back(fas);
                num_carbon += fas->num_carbon;
                num_hydroxyl += fas->num_hydroxyl;
                num_double_bonds += fas->num_double_bonds;
                if (lipid_FA_bond_type == ESTER && (fas->lipid_FA_bond_type == ETHER_PLASMANYL || fas->lipid_FA_bond_type == ETHER_PLASMENYL)){
                    lipid_FA_bond_type = fas->lipid_FA_bond_type;
                }
                else if (lipid_FA_bond_type != ESTER && (fas->lipid_FA_bond_type == ETHER_PLASMANYL || fas->lipid_FA_bond_type == ETHER_PLASMENYL)){
                    stringstream ss;
                    ss << "Only one FA can define an ether bond to the head group! Tried to add " << lipid_FA_bond_type << " over existing " << lipid_FA_bond_type;
                    string error_message;
                    ss >> error_message;
                    
                    throw ConstraintViolationException(error_message);
                }
            }
        }
    }
            
    info.level = STRUCTURAL_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
}


LipidStructuralSubspecies::~LipidStructuralSubspecies(){
    
}


string LipidStructuralSubspecies::get_lipid_string(LipidLevel level) {
    switch(level){
        case UNDEFINED_LEVEL:
        case STRUCTURAL_SUBSPECIES:
            return build_lipid_subspecies_name("/");
    
        case MOLECULAR_SUBSPECIES:
        case CATEGORY:
        case CLASS:
        case SPECIES:
            return LipidStructuralSubspecies::get_lipid_string(level);
        
        default:
            throw RuntimeException("LipidStructuralSubspecies does not know how to create a lipid string for level " + to_string(level));
    }
}
