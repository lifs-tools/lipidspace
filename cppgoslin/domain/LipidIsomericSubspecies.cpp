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
            IsomericFattyAcid *fas = (IsomericFattyAcid*)_fa->at(i);
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
            
    info.level = STRUCTURAL_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
}

LipidIsomericSubspecies::~LipidIsomericSubspecies(){
    
}


string LipidIsomericSubspecies::build_lipid_isomeric_substructure_name(){
    stringstream s;
    
    s << (!use_head_group ? LipidSpecies::get_class_string(lipid_class) : head_group);
    
    for (unsigned int i = 0; i < fa_list.size(); ++i){
        if (i > 0) s << "/"; 
        
        IsomericFattyAcid* fatty_acid = (IsomericFattyAcid*)fa_list.at(i);
        int num_carbon = 0;
        int num_hydroxyl = 0;
        int num_double_bonds = fatty_acid->num_double_bonds;
        
        
        stringstream db;
        db << "(";
        if (fatty_acid->double_bond_positions.size()){
            int j = 0;
            for (auto it : fatty_acid->double_bond_positions){
                if (j > 0) db << ",";
                db << it.first << it.second;
                ++j;
            }
        }
        db << ")";
            
        num_carbon += fatty_acid->num_carbon;
        num_hydroxyl += fatty_acid->num_hydroxyl;
        
        s << num_carbon << ":" << num_double_bonds;
        
        if (fatty_acid->double_bond_positions.size()){
            s << db.str();
        }
        
        if (num_hydroxyl > 0){
            s << ";" << num_hydroxyl;
        }
        
        s << FattyAcid::suffix(fatty_acid->lipid_FA_bond_type);
    }
    return s.str();
}


string LipidIsomericSubspecies::get_lipid_string(LipidLevel level){
    switch(level){
        case UNDEFINED_LEVEL:
        case ISOMERIC_SUBSPECIES:
            return build_lipid_isomeric_substructure_name();
            
        case STRUCTURAL_SUBSPECIES:
        case MOLECULAR_SUBSPECIES:
        case CATEGORY:
        case CLASS:
        case SPECIES:
            return LipidStructuralSubspecies::get_lipid_string(level);
    
        default:
            throw IllegalArgumentException("LipidIsomericSubspecies does not know how to create a lipid string for level " + to_string(level));
    }
}
