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
        cout << "Error: file '" + grammarFilename + "' not found." << endl;
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



void writeGrammarHeader(string ofFileName){
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
}







int main(int argc, char** argv){
    if (argc < 2){
        cout << "Error, specify grammar output filename." << endl;
        exit(-1);
    }

    writeGrammarHeader(argv[1]);
    return 0;
}
