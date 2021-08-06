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
    string test_file = "data/goslin/testfiles/fatty-acids-test.csv";
    FattyAcidParser lipid_parser;
    ShorthandParser shorthand_parser;
    

    
    if (0){
        LipidAdduct *l = lipid_parser.parse("1-methyl-cyclopentanol");
        cout << l->get_lipid_string() << endl;
        cout << l->get_sum_formula() << endl;
        return 0;
    }
    
        
    // test several more lipid names
    vector<string> lipid_data;
    vector<string> lipid_names;
    ifstream infile(test_file);
    assert(infile.good());
    string line;
    while (getline(infile, line)){
        line = strip(line, ' ');
        lipid_data.push_back(line);
    }
    infile.close();
    
    int not_implemented = 0;
    int failed = 0;
    int failed_sum = 0;
    
    
    
    ////////////////////////////////////////////////////////////////////////////
    // Test for correctness
    ////////////////////////////////////////////////////////////////////////////
    
    int i = -1;
    ofstream off("fails.csv");
    ofstream correct("correct.csv");
    for (auto lipid_name : lipid_data){
        ++i;
        
        vector<string> *data = split_string(lipid_name, ',', '"', true);
        string name = strip(data->at(3), '\"');
        if (name.length() == 0){
            delete data;
            continue;
        }
        
        if (name.find("yn") != string::npos || name.find("furan") != string::npos || endswith(name, "ane") || endswith(name, "one") || name.find("phosphate") != string::npos || name.find("pyran") != string::npos || endswith(name, "olide") || endswith(name, "-one")){
            not_implemented += 1;
            delete data;
            continue;
        }
        
        
        lipid_names.push_back(name);
        LipidAdduct *lipid = 0;
        try {
            //cout << name << endl;
            lipid = lipid_parser.parse(name);
        }
        catch (LipidException &e) {
            failed += 1;
            off << name << endl;
            delete data;
            continue;
        }
            
        string lipid_formula = lipid->get_sum_formula();
        ElementTable *e = SumFormulaParser::get_instance().parse(data->at(2));
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
        }
        delete lipid2;
        
        lipid2 = shorthand_parser.parse(lipid->get_lipid_string(MOLECULAR_SUBSPECIES));
        lipid_formula = lipid2->get_sum_formula();
        
        if (formula != lipid_formula){
            cout << "molecular subspecies, " << i << ", " << lipid_name << ": " << formula << " != " << lipid_formula << endl;
            failed_sum += 1;
        }
        delete lipid2;
        
        lipid2 = shorthand_parser.parse(lipid->get_lipid_string(SPECIES));
        lipid_formula = lipid2->get_sum_formula();
        
        if (formula != lipid_formula){
            cout << "species, " << i << ", " << lipid_name << ": " << formula << " != " << lipid_formula << endl;
            failed_sum += 1;
        }
        
        correct << lipid_name << ",\"" << lipid->get_lipid_string() << "\"" << endl;
            
        delete lipid2;
        delete lipid;
        delete data;
    }
    
    
    cout << "In the test, " << not_implemented << " of " << lipid_data.size() << " lipids can not be described by nomenclature" << endl;
    cout << "In the test, " << failed << " of " << (lipid_data.size() - not_implemented) << " lipids failed" << endl;
    cout << "In the test, " << failed_sum << " of " << (lipid_data.size() - not_implemented) << " lipid sum formulas failed" << endl;
    cout << endl;
    
    
    return 0;
}

