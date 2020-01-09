#include <iostream>
#include "cppgoslin/cppgoslin.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

int main(){
    try {
        /* initiating the parser class */
        LipidParser lipid_parser;
        LipidAdduct* lipid;
            
        /* read in file with lipid names, one per row */
        vector<string> lipidnames;
        ifstream infile("../data/goslin/lipidnames.txt");
        string line;
        while (getline(infile, line)){
            line = strip(line, ' ');
            if (line.length() < 2) continue;
            lipidnames.push_back(line);
        }
        infile.close();
        
        /* setting up a map for counting the distribution of
         * the lipids within the lipid categories
         */
        map<string, int> lipid_counts;
        
        /* parsing all lipids */
        for (auto lipid_name : lipidnames){
            lipid = lipid_parser.parse(lipid_name);
            
            /* checking if lipid name was parsed */
            if (lipid){
                string category = lipid->get_lipid_string(CATEGORY);
                delete lipid;
                
                /* adding category into the map */
                if (lipid_counts.find(category) == lipid_counts.end()) lipid_counts.insert({category, 0});
                ++lipid_counts.at(category);
            }
        }
        
        /* reporting the distribution */
        cout << "Lipid distribution:" << endl;
        for (auto key_value_pair : lipid_counts){
            cout << key_value_pair.first << ": " << key_value_pair.second << " lipids" << endl;
        }
    }
    catch (LipidException &e){
        cout << "Exception:" << endl;
        cout << e.what() << endl;
    }
    
    
    return 0;
}