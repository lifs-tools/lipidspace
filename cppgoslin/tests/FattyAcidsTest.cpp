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


using namespace std;
using namespace goslin;

int main(int argc, char** argv){
    string test_file = "cppgoslin/tests/fatty-acids-test.csv";
    FattyAcidParser lipid_parser;
    ShorthandParser shorthand_parser;
    SumFormulaParser formula_parser;
        
    // test several more lipid names
    vector<string> lipid_data;
    ifstream infile(test_file);
    string line;
    while (getline(infile, line)){
        line = strip(line, ' ');
        lipid_data.push_back(line);
    }
    infile.close();
    
    int not_implemented = 0;
    int failed = 0;
    int failed_sum = 0;
    
    int i = 0;
    for (auto lipid_name : lipid_data){
            
        vector<string> *data = split_string(lipid_name, ',', '"');
        
        string name = data->at(3);
        if (i && i % 100 == 0) cout << i << endl;
        
        if (name.find("yn") != string::npos || name.find("furan") != string::npos || endswith(name, "ane") || endswith(name, "one") || name.find("phosphate") != string::npos || name.find("pyran") != string::npos || endswith(name, "olide") || endswith(name, "-one")){
            not_implemented += 1;
            delete data;
            continue;
        }
        
        
        LipidAdduct *lipid = 0;
        try {
            lipid = lipid_parser.parse(name);
        }
        catch (...) {
            failed += 1;
            delete data;
            continue;
        }
            
        string lipid_formula = lipid->get_sum_formula();
        ElementTable *e = formula_parser.parse(data->at(2));
        string formula = goslin::compute_sum_formula(e);
        delete e;
        
        if (formula != lipid_formula){
            cout << i << ", " << lipid_name << ": " << formula << " / " << lipid_formula << endl;
            failed_sum += 1;
            assert(false);
        }
            
        if (to_lower(name).find("cyano") != string::npos){
            delete lipid;
            delete data;
            continue;
        }
        
        LipidAdduct *lipid2 = shorthand_parser.parse(lipid->get_lipid_string());
        lipid_formula = lipid2->get_sum_formula();
        
        if (formula != lipid_formula){
            cout << "current, " << i << ", " << lipid_name << ": " << formula << " != " << lipid_formula << " / " << lipid->get_lipid_string() << endl; 
            failed_sum += 1;
            assert(false);
        }
        delete lipid2;
        
        lipid2 = shorthand_parser.parse(lipid->get_lipid_string(MOLECULAR_SUBSPECIES));
        lipid_formula = lipid2->get_sum_formula();
        
        if (formula != lipid_formula){
            cout << "molecular subspecies, " << i << ", " << lipid_name << ": " << formula << " != " << lipid_formula << endl;
            failed_sum += 1;
            assert(false);
        }
        delete lipid2;
        
        lipid2 = shorthand_parser.parse(lipid->get_lipid_string(SPECIES));
        lipid_formula = lipid2->get_sum_formula();
        
        if (formula != lipid_formula){
            cout << "species, " << i << ", " << lipid_name << ": " << formula << " != " << lipid_formula << endl;
            failed_sum += 1;
            assert(false);
        }
            
        delete lipid2;
        delete lipid;
        delete data;
        ++i;
    }
    cout << "In the test, " << not_implemented << " of " << lipid_data.size() << " lipids can not be described by nomenclature" << endl;
    cout << "In the test, " << failed << " of " << (lipid_data.size() - not_implemented) << " lipids failed" << endl;
    
    return 0;
}

