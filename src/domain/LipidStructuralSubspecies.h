#ifndef LIPID_STRUCTURAL_SUBSPECIES_H
#define LIPID_STRUCTURAL_SUBSPECIES_H

#include <string>
#include "LipidExceptions.h"
#include "LipidSpeciesInfo.h"
#include "LipidEnums.h"
#include "LipidMolecularSubspecies.h"
#include <sstream>
#include <vector>
#include "FattyAcid.h"
#include <map>

using namespace std;

class LipidStructuralSubspecies : LipidMolecularSubspecies {
public:
    
    LipidStructuralSubspecies(string head_group, vector<FattyAcid*> _fa = NULL) : LipidMolecularSubspecies(head_group) {
        int num_carbon = 0;
        int num_hydroxyl = 0;
        int num_double_bonds = 0;
        LipidFaBondType lipid_FA_bond_type = ESTER;
        if (_fa != NULL){
            for (int i = 0; i < _fa->size(); ++i){
                FattyAcid *fas = _fa->at(i);
                if (fa.find(fas->name) != fa.end()){
                    throw ConstraintViolationException("FA names must be unique! FA with name " + fas->name + " was already added!");
                }
                else {
                    fa.insert(pair<string, FattyAcid*>(fas.name, fas);
                    self.fa_list.append(fas)
                    num_carbon += fas.num_carbon
                    num_hydroxyl += fas.num_hydroxyl
                    num_double_bonds += fas.num_double_bonds
                    if lipid_FA_bond_type == LipidFaBondType.ESTER and fas.lipid_FA_bond_type in (LipidFaBondType.ETHER_PLASMANYL, LipidFaBondType.ETHER_PLASMENYL):
                        lipid_FA_bond_type = fas.lipid_FA_bond_type
                        
                    elif lipid_FA_bond_type != LipidFaBondType.ESTER and fas.lipid_FA_bond_type in (LipidFaBondType.ETHER_PLASMANYL, LipidFaBondType.ETHER_PLASMENYL):
                        raise ConstraintViolationException("Only one FA can define an ether bond to the head group! Tried to add %s over existing %s" % (fas.lipid_FA_bond_type, lipid_FA_bond_type))
                }
            }
        }
        
        info.level = STRUCTURAL_SUBSPECIES;
        info.num_carbon = num_carbon;
        info.num_hydroxyl = num_hydroxyl;
        info.num_double_bonds = num_double_bonds;
        info.lipid_FA_bond_type = lipid_FA_bond_type;
    

    
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL){
        switch(level){
            case UNDEFINED_LEVEL:
            case STRUCTURAL_SUBSPECIES:
                return super().build_lipid_subspecies_name("/");
        
            case MOLECULAR_SUBSPECIES:
            case CATEGORY:
            case CLASS:
            case SPECIES:
                return super().get_lipid_string(level);
            
            default:
                stringstream s;
                s << "LipidStructuralSubspecies does not know how to create a lipid string for level " << level;
                string error_message;
                s >> error_message;
                throw RuntimeException(error_message);
        }
    }
};

#endif /* LIPID_STRUCTURAL_SUBSPECIES_H */
