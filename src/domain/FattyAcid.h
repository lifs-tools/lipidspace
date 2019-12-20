#ifndef FATTY_ACID_H
#define FATTY_ACID_H

#include <string>
#include "LipidExceptions.h"
#include "LipidEnums.h"

using namespace std;

class FattyAcid {
public:
    string name;
    int position;
    int num_carbon;
    int num_hydroxyl;
    LipidFaBondType lipid_FA_bond_type;
    bool lcb;

    FattyAcid(string _name, int _num_carbon, int _num_hydroxyl, LipidFaBondType _lipid_FA_bond_type, bool _lcb, int _position);
    static string suffix(LipidFaBondType _lipid_FA_bond_type);
};
#endif /* FATTY_ACID_H */