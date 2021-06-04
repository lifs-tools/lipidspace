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
    string ringStereo;
    DoubleBonds* doubleBonds;
    bool is_atomic;
    ElementTable* elements;
    map<string, vector<FunctionalGroup*>>* functionalGroups;
    
    
    
    FunctionalGroup(string _name, int _position = -1, int _count = 1, DoubleBonds* _doubleBonds = 0, bool _is_atomic = false, string _stereochemistry = "", ElementTable* _elements = 0, map<string, vector<FunctionalGroup*>>* _functionalGroups = 0);
    FunctionalGroup(FunctionalGroup* fg);
    ~FunctionalGroup();
    ElementTable* getElements();
    void shiftPositions(int shift);
    ElementTable* getFunctionalGroupElements();
    void computeElements();
    string toString(LipidLevel level);
    int getDoubleBonds();
    void add(FunctionalGroup* fg);
    static FunctionalGroup* getFunctionalGroup(string fgName);
};


class HeadgroupDecorator : public FunctionalGroup {
public:
    bool suffix;
    LipidLevel lowestVisibleLevel;
    
    HeadgroupDecorator(string _name, int _position = -1, int _count = 1, ElementTable* _elements = 0, bool _suffix = false, LipidLevel _level = NO_LEVEL); 
    HeadgroupDecorator(HeadgroupDecorator* hgd);
    string toString(LipidLevel level);
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
        map<string, FunctionalGroup*> knownFunctionalGroups;
        KnownFunctionalGroups(KnownFunctionalGroups const&) = delete;
        void operator=(KnownFunctionalGroups const&) = delete;
};
    

#endif /* FUNCTIONALGROUP_H */
        
        
