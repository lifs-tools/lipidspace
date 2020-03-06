#include <iostream>
#include "cppgoslin/cppgoslin.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

using namespace std;

int main(){
    try {
        /* initiating the parser class */
        SwissLipidsParser lipid_parser;
        LipidAdduct* lipidAdduct;
        
        
            
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
	ofstream slout;
	string fileName = "swisslipids-results.tsv";
	slout.open(fileName, std::ofstream::out | std::ofstream::trunc);
	if(slout.is_open()) {
        cout << "Saving parsing results to '" << fileName << "'" << endl; 
	slout << "Original Name" << '\t' << "Structural Level" << '\t' << "Name at Level" << '\t' << "Category" << '\t' << "Species" << endl;
        /* parsing all lipids */
        for (auto lipid_name : lipidnames){
	    ++totalLipids;
	    slout << lipid_name << '\t';
            lipidAdduct = lipid_parser.parse(lipid_name);
            /* checking if lipid name was parsed */
            if (lipidAdduct){
		++parsedLipids;
		LipidSpeciesInfo info = lipidAdduct->lipid->info;
		string nativeLevelName = lipidAdduct->get_lipid_string(info.level);
                slout << info.level << '\t';
		slout << nativeLevelName << '\t';
                string category = lipidAdduct->get_lipid_string(CATEGORY);
                slout << category << '\t';
		string species = lipidAdduct->get_lipid_string(SPECIES);
                slout << species << endl;
                delete lipidAdduct;
                
                /* adding species into the map */
                if (category!="") {
                    if (lipid_counts.find(category) == lipid_counts.end()) lipid_counts.insert({category, 0});
                    ++lipid_counts.at(category);
                }
            } else {
                cout << "Could not parse '" << lipid_name << "'" << endl;
		slout << "N.A." << '\t' << "N.A." << '\t' << "N.A." << '\t' << "N.A." << endl;
            }
        }
	slout.close();
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
