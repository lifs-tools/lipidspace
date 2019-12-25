#include "LipidSpecies.h"

    
LipidSpecies::LipidSpecies(string _head_group, LipidCategory _lipid_category, LipidClass _lipid_class, LipidSpeciesInfo *lipid_species_info){
    head_group = head_group;
    lipid_category = (_lipid_category != UNDEFINED_CATEGORY) ? _lipid_category : get_category(head_group);
    
    lipid_class = (_lipid_class != UNKNOWN_CLASS) ? _lipid_class : get_class(head_group);
    info = lipid_species_info;
    use_head_group = false;
}

LipidSpecies::~LipidSpecies(){
    delete info;
}

string LipidSpecies::get_lipid_string(LipidLevel level = UNDEFINED_LEVEL){
    switch (level) {
        
        case UNDEFINED_LEVEL:
            if (info != NULL){
                level = info->level;
            }
            else {
                stringstream s;
                s << "LipidSpecies does not know how to create a lipid string for level " << level;
                string error_message;
                s >> error_message;
                
                throw RuntimeException(error_message);
            }
            break;
        
        if (CATEGORY) {
            return self.lipid_category.name;
        }
        
        elif level == LipidLevel.CLASS:
            return self.lipid_class.value[2] if self.lipid_class != None else ""
        
        elif level == LipidLevel.SPECIES:
            lipid_string = [self.lipid_class.value[2] if not self.use_head_group else self.head_group]
            if self.info != None and self.info.num_carbon > 0:
                lipid_string += " %i" % self.info.num_carbon
                lipid_string += ":%i" % self.info.num_double_bonds
                if self.info.num_hydroxyl > 0: lipid_string += ";%i" % self.info.num_hydroxyl
                if self.info.lipid_FA_bond_type != None: lipid_string += self.info.lipid_FA_bond_type.suffix()
            return "".join(lipid_string)
        
        else:
            raise RuntimeException("LipidSpecies does not know how to create a lipid string for level %s" + level)
            */
    }
    return "";
}

LipidCategory LipidSpecies::get_category(string _head_group){
    return UNKNOWN_CATEGORY;
}

LipidClass LipidSpecies::get_class(string _head_group){
    return UNDEFINED_CLASS;
}
