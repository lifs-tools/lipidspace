#include <iostream>
#include "cppgoslin/cppgoslin.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

int main(){
    try {
        /* initiating the parser class */
        SwissLipidsParser lipid_parser;
        LipidAdduct* lipid;
            
        /* read in file with lipid names, one per row */
        vector<string> lipidnames;
        ifstream infile("../data/goslin/swisslipids-names-only.tsv");
        string line;
        while (getline(infile, line)){
            line = strip(line, ' ');
            if (line.length() < 2) continue;
            lipidnames.push_back(line);
        }
        infile.close();
        cout << "Reading input complete" << endl;  
        /* setting up a map for counting the distribution of
         * the lipids within the lipid categories
         */
        map<string, int> lipid_counts;

	cout << "Loaded " << lipidnames.size() << " lipid names." << endl;
        
        cout << "Parsing lipids" << endl; 
        int parsedLipids = 0;	
        int totalLipids = 0;	
        /* parsing all lipids */
        for (auto lipid_name : lipidnames){
            cout << "Parsing name: " << lipid_name << endl;
            lipid = lipid_parser.parse(lipid_name);
            /* checking if lipid name was parsed */
            if (lipid){
                string category = lipid->get_lipid_string(CATEGORY);
                string species = lipid->get_lipid_string(SPECIES);
                delete lipid;
                
                /* adding species into the map */
		if (species!="") {
			cout << "Category: " << category <<" | Species: " << species << endl;
                	if (lipid_counts.find(species) == lipid_counts.end()) lipid_counts.insert({species, 0});
               		++lipid_counts.at(species);
		} else {
			cout << "Could not retrieve category for '" << lipid_name << "'" << endl;
		}
            } else {
	    	cout << "Could not parse '" << lipid_name << "'" << endl;
	    }
        }
       
        cout << "Parsed " << parsedLipids << " of " << totalLipids << " lipid names" << endl;	
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
