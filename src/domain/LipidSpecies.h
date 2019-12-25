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
    LipidSpeciesInfo info;
    bool use_head_group;
    
    LipidSpecies(string _head_group, LipidCategory _lipid_category = UNDEFINED_CATEGORY, LipidClass lipid_class = UNKNOWN_CLASS, LipidSpeciesInfo *lipid_species_info = NULL);
    ~LipidSpecies();
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
    
    static LipidCategory get_category(string _head_group);
    static LipidClass get_class(string _head_group);
};
            
#endif /* LIPID_SPECIES_H */
