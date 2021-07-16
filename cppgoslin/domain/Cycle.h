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
    vector<Element>* bridge_chain;
    
    Cycle(int _cycle, int _start = -1, int _end = -1, DoubleBonds* _double_bonds = 0, map<string, vector<FunctionalGroup*> >* _functional_groups = 0, vector<Element>* _bridgeChain = 0);
    Cycle* copy();
    int get_double_bonds();
    void rearrange_functional_groups(FunctionalGroup *parent, int shift);
    void shift_positions(int shift);
    void compute_elements();
    string to_string(LipidLevel level);
};

#endif /* CYCLE_H */
