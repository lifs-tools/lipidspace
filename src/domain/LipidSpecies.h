#ifndef LIPID_SPECIES_H
#define LIPID_SPECIES_H

#include <string>
#include <vector>
#include <map>
#include "LipidExceptions.h"
#include "LipidSpeciesInfo.h"


using namespace std;

enum LipidCategory {UNDEFINED_CATEGORY, 
    GL, // SLM:000117142 Glycerolipids
    GP, // SLM:000001193 Glycerophospholipids
    SP, // SLM:000000525 Sphingolipids
    ST, // SLM:000500463 Steroids and derivatives
    FA, // SLM:000390054 Fatty acyls and derivatives
    SL // Saccharolipids
};

struct LipidClass {
    LipidCategory lipid_category;
    string class_name;
    vector<string> synonyms;
};

enum LipidLevel {UNDEFINED_LEVEL,
    CATEGORY, // Mediators, Glycerolipids, Glycerophospholipids, Sphingolipids, Steroids, Prenols
    CLASS, // Glyerophospholipids -> Glycerophosphoinositols (PI)
    SPECIES, // Phosphatidylinositol (16:0) or PI(16:0)
    MOLECULAR_SUBSPECIES, // Phosphatidylinositol (8:0-8:0) or PI(8:0-8:0)
    STRUCTURAL_SUBSPECIES, // Phosphatidylinositol (8:0/8:0) or PI(8:0/8:0)
    ISOMERIC_SUBSPECIES // e.g. Phosphatidylethanolamine (P-18:0/22:6(4Z,7Z,10Z,13Z,16Z,19Z))
};


typedef map<string, LipidClass> ClassMap;


class LipidSpeciesInfo;

class LipidSpecies {
public:
    string head_group;
    LipidCategory lipid_category;
    LipidClass lipid_class;
    LipidSpeciesInfo *lipid_species_info;
    bool use_head_group;
    
    /*
    static ClassMap lipid_classes = {
        { "Riti", { GL, "foo", {"a", "b"} } },
        { "Jack", { GP, "bar", {"c", "d"} } }
    };
    */
    
    
    LipidSpecies(string _head_group, LipidCategory _lipid_category = UNDEFINED_CATEGORY, LipidClass *lipid_class = NULL, LipidSpeciesInfo *lipid_species_info = NULL);
    string get_lipid_string(LipidLevel level = UNDEFINED_LEVEL);
};
            
#endif /* LIPID_SPECIES_H */