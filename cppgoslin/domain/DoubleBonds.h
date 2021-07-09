#ifndef DOUBLEBONDS_H
#define DOUBLEBONDS_H

#include <string>
#include <map>
#include "cppgoslin/domain/LipidExceptions.h"

using namespace std;

class DoubleBonds {
public:
    int num_double_bonds;
    map<int, string> double_bond_positions;
    
    DoubleBonds(int num = 0);
    DoubleBonds(DoubleBonds *double_bonds);
    int get_num();
};

#endif /* DOUBLEBONDS_H */
