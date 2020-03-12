#include "cppgoslin/cppgoslin.h"
#include <set>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <fstream>


using namespace std;

int main(int argc, char** argv){
    
    try {
        LipidAdduct* lipid;
        GoslinParser goslin_parser;
        
        
        // test several more lipid names
        vector<string> lipid_names;
        ifstream infile("cppgoslin/tests/goslin-test.csv");
        string line;
        while (getline(infile, line)){
            line = strip(line, ' ');
            lipid_names.push_back(line);
        }
        infile.close();
        
        
        for (auto lipid_name : lipid_names){
            lipid = goslin_parser.parse(lipid_name);
            if (lipid == NULL){
                throw LipidException("Error: '" + lipid_name + "'");
            }
            else {
                delete lipid;
            }
        }
        
        
        cout << "All tests passed without any problem" << endl;

    }
    catch (LipidException &e){
        cout << "Exception:" << endl;
        cout << e.what() << endl;
    }
    
    return EXIT_SUCCESS;
}
