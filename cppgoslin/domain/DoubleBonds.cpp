#include "DoubleBonds.h"

DoubleBonds::DoubleBonds(int num){
    num_double_bonds = num;
}


DoubleBonds* DoubleBonds::copy(){
    DoubleBonds* db = new DoubleBonds(num_double_bonds);
    for (auto &kv : double_bond_positions){
        db->double_bond_positions.insert({kv.first, kv.second});
    }
    return db;
}


int DoubleBonds::get_num(){
    if (double_bond_positions.size() > 0 && (int)double_bond_positions.size() != num_double_bonds)
        throw ConstraintViolationException("Number of double bonds does not match to number of double bond positions");
        
    return num_double_bonds;
}

