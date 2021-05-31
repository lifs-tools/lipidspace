#ifndef HEADGROUP_H
#define HEADGROUP_H

#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/LipidCategory.h"
#include "cppgoslin/domain/LipidSpeciesInfo.h"
#include "cppgoslin/domain/LipidClass.h"
#include "cppgoslin/domain/FunctionalGroup.h"
#include "cppgoslin/domain/LipidFaBondType.h"
#include "cppgoslin/domain/Element.h"
#include <string>
#include <vector>

using namespace std;

class HeadGroup {
public:
    string headgroup;
    LipidCategory lipid_category;
    LipidClass lipid_class;
    bool use_headgroup;
    vector<string> decorators;
    bool sp_exception;
    
    Headgroup(string headgroup, vector<string>* decorators = 0, bool use_headgroup = false);
    string get_lipid_string(LipidLevel level = NO_LEVEL);
    ElementTable* get_elements();
};

#endif /* HEADGROUP_H */
