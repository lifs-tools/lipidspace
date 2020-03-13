#include "LipidIsomericSubspecies.h"


LipidIsomericSubspecies::LipidIsomericSubspecies(string _head_group, vector<FattyAcid*> *_fa) : LipidStructuralSubspecies(_head_group) {
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = UNDEFINED_FA;
    if (_fa->size() > 0){
        lipid_FA_bond_type = ESTER;
    }
    
    if (_fa) {
        for (unsigned int i = 0; i < _fa->size(); ++i){
            IsomericFattyAcid *fas = new IsomericFattyAcid(_fa->at(i));
            delete _fa->at(i);
            
            if (fa.find(fas->name) != fa.end()){
                throw ConstraintViolationException("FA names must be unique! FA with name " + fas->name + " was already added!");
            }
            
            else {
                fa.insert(pair<string, FattyAcid*>(fas->name, fas));
                fa_list.push_back(fas);
                num_carbon += fas->num_carbon;
                num_hydroxyl += fas->num_hydroxyl;
                num_double_bonds += fas->num_double_bonds;
                
                if (lipid_FA_bond_type == ESTER && (fas->lipid_FA_bond_type == ETHER_PLASMANYL || fas->lipid_FA_bond_type == ETHER_PLASMENYL)){
                    lipid_FA_bond_type = fas->lipid_FA_bond_type;
                }
                    
                else if (lipid_FA_bond_type != ESTER && (fas->lipid_FA_bond_type == ETHER_PLASMANYL || fas->lipid_FA_bond_type == ETHER_PLASMENYL)){
                    throw ConstraintViolationException("Only one FA can define an ether bond to the head group! Tried to add " + to_string(fas->lipid_FA_bond_type) + " over existing " + to_string(lipid_FA_bond_type));
                }
            }
        }
    }
            
    info.level = ISOMERIC_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
}

LipidIsomericSubspecies::~LipidIsomericSubspecies(){
    
}




LipidLevel LipidIsomericSubspecies::get_lipid_level(){
    return ISOMERIC_SUBSPECIES;
}



string LipidIsomericSubspecies::get_lipid_string(LipidLevel level){
    switch(level){
        case NO_LEVEL:
        case ISOMERIC_SUBSPECIES:
            return LipidMolecularSubspecies::build_lipid_subspecies_name("/", level);
            
        case SPECIES:
        case STRUCTURAL_SUBSPECIES:
        case MOLECULAR_SUBSPECIES:
        case CATEGORY:
        case CLASS:
            return LipidStructuralSubspecies::get_lipid_string(level);
    
        default:
            throw IllegalArgumentException("LipidIsomericSubspecies does not know how to create a lipid string for level " + to_string(level));
    }
}
