#ifndef HEADGROUP_H
#define HEADGROUP_H

#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/LipidSpeciesInfo.h"
#include "cppgoslin/domain/FunctionalGroup.h"
#include "cppgoslin/domain/Element.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Headgroup {
public:
    string headgroup;
    LipidCategory lipid_category;
    LipidClass lipid_class;
    bool use_headgroup;
    vector<HeadgroupDecorator*>* decorators;
    bool sp_exception;
    
    Headgroup(string _headgroup, vector<HeadgroupDecorator*>* _decorators = 0, bool _use_headgroup = false);
    ~Headgroup();
    string get_lipid_string(LipidLevel level = NO_LEVEL);
    ElementTable* get_elements();
    LipidCategory get_category(string _headgroup);
    LipidClass get_class(string _head_group);
    string get_class_string(LipidClass _lipid_lass);
    string get_class_name();
    string get_category_string(LipidCategory _lipid_category);
    
};

#endif /* HEADGROUP_H */
