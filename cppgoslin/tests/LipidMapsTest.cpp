#include "cppgoslin/cppgoslin.h"
#include <set>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <fstream>
#include <chrono>
#include <vector>
#include <map>


using namespace std;

int main(int argc, char** argv){
    
    try {
        LipidAdduct* lipid;
        LipidMapsParser lipid_maps_parser;
        
        
        // test several more lipid names
        vector<string> lipid_names_income;
        vector<string> lipid_names_outcome;
        ifstream infile("cppgoslin/tests/lipid-maps-test.csv");
        string line;
        while (getline(infile, line)){
            vector<string>* tokens = split_string(line, ',', '"');
            line = strip(line, ' ');
            if (tokens->size() > 1){
                lipid_names_income.push_back(strip(tokens->at(0), '"'));
                lipid_names_outcome.push_back(strip(tokens->at(1), '"'));
            }
            delete tokens;
        }
        infile.close();
        
        auto start = chrono::steady_clock::now();
        
        map<string, int> lipid_classes;
        
        for (uint i = 0; i < lipid_names_income.size(); ++i){
            string lipid_name = lipid_names_income.at(i);
            string correct_lipid_name = lipid_names_outcome.at(i);
            lipid = lipid_maps_parser.parse(lipid_name);
            if (lipid == NULL){
                throw LipidException("Error: '" + lipid_name + "'");
            }
            else {
                string lipid_class = lipid->get_lipid_string(CLASS);
                if (lipid_class == "Undefined lipid class"){
                    throw LipidException("Error (undefined): '" + lipid_name + "'");
                }
                if (lipid_classes.find(lipid_class) == lipid_classes.end()){
                    lipid_classes.insert({lipid_class, 0});
                }
                lipid_classes.at(lipid_class) = lipid_classes.at(lipid_class) + 1;
                delete lipid;
            }
        }
        auto end = chrono::steady_clock::now();
        auto sec = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "time elapsed: " << double(sec) / 1000. << " sec" << endl;
        cout << "lipids / sec: " << double(lipid_names_income.size()) * 1000. / (double(sec)) << endl;
        
        cout << endl;
        
        for (auto kv : lipid_classes){
            cout << kv.first << ": " << kv.second << endl;
        }
        
        
        cout << "All tests passed without any problem" << endl;

    }
    catch (LipidException &e){
        cout << "Exception:" << endl;
        cout << e.what() << endl;
    }
    
    return EXIT_SUCCESS;
}
