#ifndef FUNCTIONALGROUP_H
#define FUNCTIONALGROUP_H

#include "cppgoslin/domain/Element.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/DoubleBonds.h"
#include <map>
#include <vector>
#include <string>

using namespace std;
using namespace goslin;


#define contains(X, Y) ((X).find(Y) != (X).end())
#define contains_p(X, Y) ((X)->find(Y) != (X)->end())
#define uncontains(X, Y) ((X).find(Y) == (X).end())
#define uncontains_p(X, Y) ((X)->find(Y) == (X)->end())

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
    
    FunctionalGroup(string _name, int _position = -1, int _count = 1, DoubleBonds* _double_bonds = 0, bool _is_atomic = false, string _stereochemistry = "", ElementTable* _elements = 0, map<string, vector<FunctionalGroup*>>* _functional_groups = 0);
    FunctionalGroup(FunctionalGroup* fg);
    ~FunctionalGroup();
    ElementTable* get_elements();
    void shift_positions(int shift);
    ElementTable* get_functional_group_elements();
    void compute_elements();
    string to_string(LipidLevel level);
    int get_double_bonds();
    void add(FunctionalGroup* fg);
    static FunctionalGroup* get_functional_group(string fg_name);
};


class HeadgroupDecorator : public FunctionalGroup {
public:
    bool suffix;
    LipidLevel lowest_visible_level;
    
    HeadgroupDecorator(string _name, int _position = -1, int _count = 1, ElementTable* _elements = 0, bool _suffix = false, LipidLevel _level = NO_LEVEL); 
    HeadgroupDecorator(HeadgroupDecorator* hgd);
    string to_string(LipidLevel level);
};


class KnownFunctionalGroups {
    public:
        static KnownFunctionalGroups& get_instance()
        {
            static KnownFunctionalGroups instance;
            return instance;
        }
    private:
        KnownFunctionalGroups();
        
    public:
        map<string, FunctionalGroup*> known_functional_groups;
        KnownFunctionalGroups(KnownFunctionalGroups const&) = delete;
        void operator=(KnownFunctionalGroups const&) = delete;
};
    

#endif /* FUNCTIONALGROUP_H */
        
        
