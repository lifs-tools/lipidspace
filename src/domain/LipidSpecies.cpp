#include "LipidSpecies.h"

    
LipidSpecies::LipidSpecies(string _head_group, LipidCategory _lipid_category, LipidClass *lipid_class, LipidSpeciesInfo *lipid_species_info){
    /*
    self.head_group = head_group.strip(" ")
    self.lipid_category = lipid_category if lipid_category != None else LipidClass.get_category(self.head_group)
    
    self.lipid_class = lipid_class if lipid_class != None else LipidClass.get_class(self.head_group)
    self.info = lipid_species_info
    self.use_head_group = False
        */  
}

string LipidSpecies::get_lipid_string(LipidLevel level){
    /*
    if level == None:
        if self.info != None:
            level = self.info.level
        else:
            raise RuntimeException("LipidSpecies does not know how to create a lipid string for level %s" % (level if level != None else " unknown"))
    
    if level == LipidLevel.CATEGORY:
        return self.lipid_category.name
    
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
    return "";
}
