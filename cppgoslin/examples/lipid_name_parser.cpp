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
    
    if (lipid != NULL){
        cout << lipid->get_lipid_string() << endl;
        delete lipid;
    }
    
    return 0;
}