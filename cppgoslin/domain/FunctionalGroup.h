#ifndef FUNCTIONALGROUP_H
#define FUNCTIONALGROUP_H

#include "cppgoslin/domain/Element.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/DoubleBonds.h"
#include "cppgoslin/domain/StringFunctions.h"
#include <map>
#include <vector>
#include <string>


using namespace std;
using namespace goslin;

class FunctionalGroup {
public:
    string name;
    int position;
    int count;
    string stereochemistry;
    string ring_stereo;
    DoubleBonds* double_bonds;
    bool is_atomic;
    ElementTable* elements;
    map<string, vector<FunctionalGroup*>>* functional_groups;
    static bool position_sort_function(FunctionalGroup* f1, FunctionalGroup *f2);
    static bool lower_name_sort_function(string s1, string s2);
    
    FunctionalGroup(string _name, int _position = -1, int _count = 1, DoubleBonds* _double_bonds = 0, bool _is_atomic = false, string _stereochemistry = "", ElementTable* _elements = 0, map<string, vector<FunctionalGroup*>>* _functional_groups = 0);
    virtual ~FunctionalGroup();
    virtual FunctionalGroup* copy();
    virtual ElementTable* get_elements();
    void shift_positions(int shift);
    virtual ElementTable* get_functional_group_elements();
    virtual void compute_elements();
    virtual string to_string(LipidLevel level);
    virtual int get_double_bonds();
    void add(FunctionalGroup* fg);
};


class KnownFunctionalGroups {
public:
    KnownFunctionalGroups();
    ~KnownFunctionalGroups();
    map<string, FunctionalGroup*> known_functional_groups;
    static FunctionalGroup* get_functional_group(string fg_name);
    static KnownFunctionalGroups k;
};


class HeadgroupDecorator : public FunctionalGroup {
public:
    bool suffix;
    LipidLevel lowest_visible_level;
    
    HeadgroupDecorator(string _name, int _position = -1, int _count = 1, ElementTable* _elements = 0, bool _suffix = false, LipidLevel _level = NO_LEVEL);
    string to_string(LipidLevel level);
    HeadgroupDecorator* copy();
};

    

#endif /* FUNCTIONALGROUP_H */
        
        
