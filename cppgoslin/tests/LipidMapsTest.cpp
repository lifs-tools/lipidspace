/*
MIT License

Copyright (c) the authors (listed in global LICENSE file)

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
#include <vector>
#include <map>


using namespace std;
using namespace goslin;

int main(int argc, char** argv){
    
    LipidAdduct* lipid;
    LipidMapsParser lipid_maps_parser;
    
    
    // test several more lipid names
    vector<string> lipid_names_income;
    vector<string> lipid_names_outcome;
    ifstream infile("data/goslin/testfiles/lipid-maps-test.csv");
    assert(infile.good());
    string line;
    while (getline(infile, line)){
        vector<string>* tokens = split_string(line, ',', '"', true);
        line = strip(line, ' ');
        if (tokens->size() > 1){
            lipid_names_income.push_back(strip(tokens->at(0), '"'));
            lipid_names_outcome.push_back(strip(tokens->at(1), '"'));
        }
        delete tokens;
    }
    infile.close();
        
    for (uint32_t i = 0; i < lipid_names_income.size(); ++i){
        string lipid_name = lipid_names_income.at(i);
        string correct_lipid_name = lipid_names_outcome.at(i);
        try {
            lipid = lipid_maps_parser.parse(lipid_name);
            assert(lipid != NULL);
            if (correct_lipid_name != "Unsupported lipid" && correct_lipid_name.length() > 0){
                //cout << lipid_name << "  |  " << lipid->get_lipid_string() << "  |  " << correct_lipid_name << " (reference)" << endl;
                assert(correct_lipid_name == lipid->get_lipid_string());
            }
            delete lipid;
        }
        catch (LipidException &e){
            if (correct_lipid_name.length() > 0){
                cout << "Exception: " << lipid_names_income.at(i) << endl;
                cout << e.what() << endl;
                assert(false);
            }
        }
    }
    
    
    cout << "All tests passed without any problem" << endl;

    
    
    return EXIT_SUCCESS;
}
