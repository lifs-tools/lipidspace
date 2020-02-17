#include "cppgoslin/cppgoslin.h"
#include <iostream>
#include <string>

using namespace std;

int main(){
    /* create instance of lipid parser containing several grammars */
    LipidParser lipid_parser;
    
    /* parsing lipid name into a lipid container data structure */
    string lipid_name = "PA(12:0_14:1)";
    LipidAdduct* lipid = lipid_parser.parse(lipid_name);
    
    /* checking if parsing was successful, otherwise lipid reference remains NULL */
    if (lipid != NULL){
        /* creating lipid name according to the recent lipid nomenclature */
        cout << lipid->get_lipid_string() << endl;
        cout << lipid->get_class_name() << endl;
        
        /* important to delete lipid to avoid memory leaks */
        delete lipid;
    }
    
    return 0;
}