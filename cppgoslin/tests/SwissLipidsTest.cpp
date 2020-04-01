/*
MIT License

Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de
                   Nils Hoffmann  -  nils.hoffmann {at} isas.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


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
        SwissLipidsParser swiss_lipids_parser;
        
        
        // test several more lipid names
        vector<string> lipid_names;
        ifstream infile("cppgoslin/tests/swiss-lipids-test.csv");
        string line;
        while (getline(infile, line)){
            line = strip(line, ' ');
            lipid_names.push_back(line);
        }
        infile.close();
        
        
        for (auto lipid_name : lipid_names){
            lipid = swiss_lipids_parser.parse(lipid_name);
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
