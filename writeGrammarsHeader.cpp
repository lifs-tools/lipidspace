#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}



void addingGrammar(ofstream& offile, string grammarName, string grammarFilename){

    
    ifstream infile(grammarFilename.c_str());
    offile << "static const string " + grammarName + " = \"";
    if (!infile.good()){
        cout << "Error: file'" + grammarFilename + "' not found." << endl;
        exit(-1);
    }
    
    string line;
    while (getline(infile, line)){
        replaceAll(line, "\\", "\\\\");
        replaceAll(line, "\"", "\\\"");
        line += " \\n\\";
        offile << line << endl;
    }
    offile << "\";" << endl;
}



int main(int argc, char** argv){
    if (argc < 2){
        cout << "Error, specify grammar output filename." << endl;
        exit(-1);
    }

    string ofFileName = argv[1];
    ofstream offile(ofFileName.c_str());
    
    
    offile << "#ifndef KNOWN_GRAMMARS_H" << endl;
    offile << "#define KNOWN_GRAMMARS_H" << endl << endl;

    offile << "#include <string>" << endl;
    offile << "#include \"cppgoslin/parser/Parser.h\"" << endl;

    offile << "using namespace std;" << endl;

    
    addingGrammar(offile, "goslin_grammar", "data/goslin/Goslin.g4");
    offile << endl << endl << endl;
    
    addingGrammar(offile, "goslin_fragment_grammar", "data/goslin/GoslinFragments.g4");
    offile << endl << endl << endl;
    
    addingGrammar(offile, "lipid_maps_grammar", "data/goslin/LipidMaps.g4");
    offile << endl << endl << endl;
    
    addingGrammar(offile, "swiss_lipids_grammar", "data/goslin/SwissLipids.g4");
    offile << endl << endl << endl;
    
    offile << "#endif /* KNOWN_GRAMMARS_H */" << endl;
    
    return 0;
}