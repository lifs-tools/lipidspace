#ifndef LIPID_SPECIES_H
#define LIPID_SPECIES_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidSpeciesInfo.h"


using namespace std;


class LipidSpeciesInfo;



class LipidSpecies {
public:
    string head_group;
    LipidCategory lipid_category;
    LipidClass lipid_class;
    LipidSpeciesInfo info;
    bool use_head_group;
    
    virtual ~LipidSpecies();
    LipidSpecies(string _head_group);
    LipidSpecies(string _head_group, LipidCategory _lipid_category, LipidClass lipid_class, LipidSpeciesInfo *lipid_species_info);
    virtual string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
    
    static LipidCategory get_category(string _head_group);
    static LipidClass get_class(string _head_group);
    static string get_class_string(LipidClass lipid_class);
    static string get_category_string(LipidCategory lipid_category);
};
            
#endif /* LIPID_SPECIES_H */
