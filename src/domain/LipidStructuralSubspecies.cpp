#include "LipidStructuralSubspecies.h"



LipidStructuralSubspecies::LipidStructuralSubspecies(string head_group, vector<FattyAcid*> *_fa) : LipidMolecularSubspecies (head_group) {
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = LipidFaBondType.ESTER;
    
    for (int i = 0; i < _fa->size; ++i){
        StructuralFattyAcid *fas = (StructuralFattyAcid)_fa;
        if (fa.find(fas->name) == fa.end()){
            throw ConstraintViolationException("FA names must be unique! FA with name " + fas->name + " was already added!");
        }
        else {
            fa.insert({fas->name, fas});
            fa_list.push_back(fas);
            num_carbon += fas->num_carbon;
            num_hydroxyl += fas->num_hydroxyl;
            num_double_bonds += fas->num_double_bonds;
            if (lipid_FA_bond_type == LipidFaBondType.ESTER && (fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMANYL || fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMENYL)){
                lipid_FA_bond_type = fas.lipid_FA_bond_type;
            }
            else if (lipid_FA_bond_type != LipidFaBondType.ESTER && (fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMANYL || fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMENYL)){
                stringstream ss;
                ss << "Only one FA can define an ether bond to the head group! Tried to add " << lipid_FA_bond_type << " over existing " << lipid_FA_bond_type;
                string error_message;
                ss >> error_message;
                
                throw ConstraintViolationException(error_message);
        }
    }
            
    info = new LipidSpeciesInfo();
    info->level = LipidLevel.STRUCTURAL_SUBSPECIES;
    info->num_carbon = num_carbon;
    info->num_hydroxyl = num_hydroxyl;
    info->num_double_bonds = num_double_bonds;
    info->lipid_FA_bond_type = lipid_FA_bond_type;
}

LipidStructuralSubspecies::~LipidStructuralSubspecies(){
    delete info;
}

LipidStructuralSubspecies::string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL){
    if (level == UNDEFINED_LEVEL || level == LipidLevel.STRUCTURAL_SUBSPECIES){
        return build_lipid_subspecies_name("/");
    }
    
    else if (level == LipidLevel.MOLECULAR_SUBSPECIES ||
             level == LipidLevel.MOLECULAR_SUBSPECIESLipidLevel.CATEGORY ||
             level == LipidLevel.MOLECULAR_SUBSPECIESLipidLevel.CLASS ||
             level == LipidLevel.MOLECULAR_SUBSPECIESLipidLevel.SPECIES){
        return get_lipid_string(level);
    }
        
    else{
        stringstream ss;
        ss << "LipidStructuralSubspecies does not know how to create a lipid string for level " << level;
        string error_message;
        ss >> error_message;
        raise RuntimeException(error_message);
    }
}
