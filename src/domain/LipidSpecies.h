#ifndef LIPID_SPECIES_H
#define LIPID_SPECIES_H

#include <string>
#include <vector>
#include <map>
#include "LipidExceptions.h"
#include "LipidSpeciesInfo.h"


using namespace std;


class LipidSpeciesInfo;



class LipidSpecies {
public:
    string head_group;
    LipidCategory lipid_category;
    LipidClass lipid_class;
    LipidSpeciesInfo *lipid_species_info;
    bool use_head_group;
    
    
    
    
    
    LipidSpecies(string _head_group, LipidCategory _lipid_category = UNDEFINED_CATEGORY, LipidClass *lipid_class = NULL, LipidSpeciesInfo *lipid_species_info = NULL);
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
};
            
#endif /* LIPID_SPECIES_H */