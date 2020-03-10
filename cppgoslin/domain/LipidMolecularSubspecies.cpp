#include "LipidMolecularSubspecies.h"

LipidMolecularSubspecies::LipidMolecularSubspecies (string _head_group) : LipidSpecies(_head_group) {
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = ESTER;
    
    info.level = MOLECULAR_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
    
}


LipidMolecularSubspecies::LipidMolecularSubspecies (string _head_group, vector<FattyAcid*> *_fa) : LipidSpecies(_head_group) {
    
    int num_carbon = 0;
    int num_hydroxyl = 0;
    int num_double_bonds = 0;
    LipidFaBondType lipid_FA_bond_type = ESTER;
    if (_fa) {
        for (unsigned int i = 0; i < _fa->size(); ++i){
            MolecularFattyAcid *fas = new MolecularFattyAcid(_fa->at(i));
            delete _fa->at(i);
            
            if (fas->position != -1) {
                throw ConstraintViolationException("MolecularFattyAcid " + fas->name + " must have position set to -1! Was: " + to_string(fas->position));
            }
            
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
                    throw ConstraintViolationException("Only one FA can define an ether bond to the head group! Tried to add " + to_string(fas->lipid_FA_bond_type) + " over existing " + to_string(lipid_FA_bond_type));
                }
            }
        }
    }        
    info.level = MOLECULAR_SUBSPECIES;
    info.num_carbon = num_carbon;
    info.num_hydroxyl = num_hydroxyl;
    info.num_double_bonds = num_double_bonds;
    info.lipid_FA_bond_type = lipid_FA_bond_type;
    
}

LipidMolecularSubspecies::~LipidMolecularSubspecies(){
    
}

#include <iostream>
using namespace std;

string LipidMolecularSubspecies::build_lipid_subspecies_name(string fa_separator){
    stringstream s;
    s << (!use_head_group ? get_class_string(lipid_class) : head_group);
    bool special_case = (lipid_class == PC) | (lipid_class == LPC) | (lipid_class == PE) | (lipid_class == LPE);
    
    if (fa_list.size() > 0){
        s << " ";
        for (unsigned int i = 0; i < fa_list.size(); ++i){
            if (i > 0) s << fa_separator;
            FattyAcid *fatty_acid = fa_list.at(i);
            int num_double_bonds = 0;
            
            
            MolecularFattyAcid* mol_fatty_acid = dynamic_cast<MolecularFattyAcid*>(fatty_acid);
            if (mol_fatty_acid){
                num_double_bonds = mol_fatty_acid->num_double_bonds;
            }
            
            int num_carbon = fatty_acid->num_carbon;
            int num_hydroxyl = fatty_acid->num_hydroxyl;
            string suffix = FattyAcid::suffix(fatty_acid->lipid_FA_bond_type);
            if (special_case && suffix.length() > 0) s << "O-";
            s << num_carbon << ":" << num_double_bonds;
            IsomericFattyAcid* iso_fatty_acid = dynamic_cast<IsomericFattyAcid*>(fatty_acid);
            if (iso_fatty_acid != NULL && iso_fatty_acid->double_bond_positions.size()){
                stringstream db;
                db << "(";
                int j = 0;
                for (auto it : iso_fatty_acid->double_bond_positions){
                    if (j > 0) db << ",";
                    db << it.first << it.second;
                    ++j;
                }
                db << ")";
                s << db.str();
            }
            
            if (num_hydroxyl) s << ";" << num_hydroxyl;
            s << suffix;
        }
    }
    
    return s.str();
}


string LipidMolecularSubspecies::get_lipid_string(LipidLevel level) {
    switch (level){
        case NO_LEVEL:
        case MOLECULAR_SUBSPECIES:
            return build_lipid_subspecies_name("_");
    
        case CATEGORY:
        case CLASS:
        case SPECIES:
            return LipidSpecies::get_lipid_string(level);
    
        default:
            stringstream s;
            s << "LipidMolecularSubspecies does not know how to create a lipid string for level " << level;
            string error_message;
            s >> error_message;
            throw IllegalArgumentException("LipidMolecularSubspecies does not know how to create a lipid string for level " + to_string(level));
    }
}
