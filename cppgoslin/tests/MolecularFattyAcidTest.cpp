#include <string>
#include "cppgoslin/domain/MolecularFattyAcid.h"
#include "cppgoslin/domain/LipidExceptions.h"
#include <cassert>
#include <iostream>

using namespace std;

int main(int argc, char** argv){

    
    MolecularFattyAcid instanceZero = MolecularFattyAcid("FA1", 2, 0, 0, UNDEFINED_FA, false, 0);
    assert(0 == instanceZero.num_double_bonds);
    
    
    MolecularFattyAcid instanceOne = MolecularFattyAcid("FA1", 2, 1, 0, UNDEFINED_FA, false, 0);
    assert(1 == instanceOne.num_double_bonds);
         
    try {
        MolecularFattyAcid instanceZeroFail = MolecularFattyAcid("FA1", 2, -1, 0, UNDEFINED_FA, false, 0);
        assert(false);
    }
    catch (LipidException &e){
        assert(true);
    }
       
    MolecularFattyAcid instance = MolecularFattyAcid("FAX", 2, 0, 0, UNDEFINED_FA, false, 0);
    assert("FAX" == instance.name);

    
    instance = MolecularFattyAcid("FAX", 2, 0, 0, UNDEFINED_FA, false, 1);
    assert(1 == instance.position);
    
    try {
        instanceZero = MolecularFattyAcid("FA1", 2, 0, 0, UNDEFINED_FA, false, -2);
        assert(false);
    }
    catch (LipidException &e){
        assert(true);
    }


    instance = MolecularFattyAcid("FAX", 2, 0, 0, UNDEFINED_FA, false, 1);
    assert(2 == instance.num_carbon);


    try {
        instance = MolecularFattyAcid("FAX", 1, 0, 0, UNDEFINED_FA, false, 1);
        assert(false);
    }
    catch (LipidException &e){
        assert(true);
    }
    

    instance = MolecularFattyAcid("FAX", 2, 0, 1, UNDEFINED_FA, false, 1);
    assert(1 == instance.num_hydroxyl);


    try {
        instance = MolecularFattyAcid("FAX", 2, 0, -1, UNDEFINED_FA, false, 1);
        assert(false);
    }
    catch (LipidException &e){
        assert(true);
    }
    
    cout << "All tests passed without any problem" << endl;
    return 0;
}
