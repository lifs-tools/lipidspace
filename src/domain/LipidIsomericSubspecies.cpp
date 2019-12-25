#include "LipidIsomericSubspecies.h"

LipidIsomericSubspecies::LipidIsomericSubspecies(string head_group, vector<FattyAcid*> *_fa) : LipidStructuralSubspecies(head_group) {
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = UNDEFINED_FA;
    if (_fa->size() > 0){
        lipid_FA_bond_type = ESTER;
    }
    
    if (_fa) {
        for (int i = 0; i < _fa->size(); ++i){
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
                
                if ((lipid_FA_bond_type == ESTER && (fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMANYL || fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMENYL)){
                    lipid_FA_bond_type = fas->lipid_FA_bond_type;
                }
                    
                eles if n(lipid_FA_bond_type != LipidFaBondType.ESTER && (fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMANYL || fas->lipid_FA_bond_type == LipidFaBondType.ETHER_PLASMENYL)){
                    stringstream s;
                    s << "Only one FA can define an ether bond to the head group! Tried to add " << fas->lipid_FA_bond_type << " over existing " << lipid_FA_bond_type;
                    string error_message;
                    s >> error_message;
                    
                    throw ConstraintViolationException(error_message);
                }
            }
        }
    }
            
    info = new LipidSpeciesInfo();
    info->level = LipidLevel.STRUCTURAL_SUBSPECIES;
    info->num_carbon = num_carbon;
    info->num_hydroxyl = num_hydroxyl;
    info->num_double_bonds = num_double_bonds;
    info->lipid_FA_bond_type = lipid_FA_bond_type;
}

string LipidIsomericSubspecies::build_lipid_isomeric_substructure_name(){
    stringstream s;
    
    s << (!use_headgroup ? LipidSpecies.get_headgroup(lipid_class) : head_group);
    
    for (int i = 0; i < fa_list.size(); ++i){
        if (i > 0) s << "/"; 
        
        fattyAcid* fatty_acid = fa_list.at(i);
        int num_carbon = 0;
        int num_hydroxyl = 0;
        int num_double_bonds = fatty_acid.num_double_bonds;
        
        
        stringstream db;
        db << "(";
        if (((IsomericFattyAcid)fatty_acid)->double_bond_positions.size()){
            for (map<string, int>::iterator it = fatty_acid->double_bond_positions.begin(), j = 0; it != fatty_acid->double_bond_positions.end(); ++it, ++j){
                if (j > 0) db << ",";
                db << it->first << it->second;s
        }
        db << ")";
        string db_pos;
        db >> db_pos;
        
            
        num_carbon += fattyAcid->num_carbon;
        num_hydroxyl += fattyAcid->num_hydroxyl;
        
        s << num_carbon << ":" << num_double_bonds;
        
        if (((IsomericFattyAcid)fatty_acid)->double_bond_positions.size()){
            s << db_pos;
        }
        
        if (num_hydroxyl > 0){
            s << ";" << num_hydroxyl;
        }
        
        s << FattyAcid.suffix(fatty_acid->lipid_FA_bond_type);
    }
    
    
    string fa_strings;
    s >> fa_strings;
    return fa_strings;
}


string LipidIsomericSubspecies::get_lipid_string(LipidLevel level = UNDEFINED_LEVEL){
    switch(level){
        case UNDEFINED_LEVEL;
        case ISOMERIC_SUBSPECIES:
            return build_lipid_isomeric_substructure_name();
            
        case STRUCTURAL_SUBSPECIES:
        case MOLECULAR_SUBSPECIES;
        case CATEGORY; 
        case CLASS:
        case SPECIES:
            return super().get_lipid_string(level);
    
        default:
            stringstream s;
            s << "LipidIsomericSubspecies does not know how to create a lipid string for level " << level;
            string error_message;
            s >> error_message;
            throw Exception(error_message);
    }
}
