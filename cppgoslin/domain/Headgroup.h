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
    LipidCategory lipidCategory;
    LipidClass lipidClass;
    bool useHeadgroup;
    vector<HeadgroupDecorator*>* decorators;
    bool spException;
    
    Headgroup(string _headgroup, vector<HeadgroupDecorator*>* _decorators = 0, bool _useHeadgroup = false);
    ~Headgroup();
    string getLipidString(LipidLevel level = NO_LEVEL);
    ElementTable* getElements();
    LipidCategory getCategory(string _headgroup);
    LipidLevel getLipidLevel();
    LipidClass getClass(string _head_group);
    string getClassString(LipidClass _lipidClass);
    string getClassName();
    string getCategoryString(LipidCategory _lipidCategory);
    
};

#endif /* HEADGROUP_H */
