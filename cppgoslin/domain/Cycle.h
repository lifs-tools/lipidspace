#ifndef CYCLE_H
#define CYCLE_H

#include "cppgoslin/domain/FunctionalGroup.h"
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/Element.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/DoubleBonds.h"
#include "cppgoslin/domain/StringFunctions.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include <string>

using namespace std;
using namespace goslin;

class Cycle : public FunctionalGroup {
public:
    int cycle;
    int start;
    int end;
    vector<Element>* bridgeChain;
    
    Cycle(int _cycle, int _start = -1, int _end = -1, DoubleBonds* _doubleBonds = 0, map<string, vector<FunctionalGroup*> >* _functionalGroups = 0, vector<Element>* _bridgeChain = 0);
    Cycle(Cycle* c);
    int getDoubleBonds();
    void rearrangeFunctionalGroups(FunctionalGroup *parent, int shift);
    void shiftPositions(int shift);
    void computeElements();
    string toString(LipidLevel level);
    static bool sortFunction(FunctionalGroup* f1, FunctionalGroup *f2);
};

#endif /* CYCLE_H */
