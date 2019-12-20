#ifndef FATTY_ACID_H
#define FATTY_ACID_H

#include <string>
#include "LipidFaBondType.h"

using namespace std;

enum LipidFaBondType { UNDEFINED_FA, ESTER, ETHER_PLASMANYL, ETHER_PLASMENYL};

class FattyAcid {
public:
    string name;
    int position;
    int num_carbon;
    int num_hydroxyl;
    LipidFaBondType lipid_FA_bond_type;
    bool lcb = lcb;

    FattyAcid(string _name, int _num_carbon, int _num_hydroxyl, LipidFaBondType _lipid_FA_bond_type, bool _lcb, int _position);
    static string suffix(LipidFaBondType _lipid_FA_bond_type);
};
#endif /* FATTY_ACID_H */