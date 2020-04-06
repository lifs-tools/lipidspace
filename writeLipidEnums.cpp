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
#include <map>
#include <stdlib.h>
#include "cppgoslin/domain/StringFunctions.h"

using namespace std;



void writeLipidEnum(string ofFileName){
    ofstream offile(ofFileName.c_str());
    
    ifstream infile("data/goslin/lipid-list.csv");
    if (!infile.good()){
        cout << "Error: file 'data/goslin/lipid-list.csv' not found." << endl;
        exit(-1);
    }
    
    string line;
    unsigned int i = 0;
    map<string, int> enum_names = {{"GL", 1}, {"GP", 1}, {"SP", 1}, {"ST", 1}, {"FA", 1}, {"SL", 1}, {"UNDEFINED", 1}};
    
    map<string, vector<string>*> data;
    while (getline(infile, line)){
        if (i++ == 0) continue;
        vector<string>* tokens = split_string(line, ',', '"');
        for (unsigned int i = 0; i < tokens->size(); ++i){
            string s = tokens->at(i);
            if (s.length() >= 2 && s[0] == '"' && s[s.length() - 1] == '"'){
                tokens->at(i) = s.substr(1, s.length() - 2);
            }
        }
        
        
        string enum_name = tokens->at(0);
        
        for (unsigned int i = 0; i < enum_name.length(); ++i){
            char c = enum_name[i];
            if ('A' <= c && c <= 'Z'){
                
            }
            else if ('0' <= c && c <= '9'){
                
            }
            else if ('a' <= c && c <= 'z'){
                enum_name[i] = c - ('a' - 'A');
            }
            else {
                enum_name[i] = '_';
            }
        }
        
        
        if (enum_name[0] == '_'){
            enum_name = "L" + enum_name;
        }
        
        if (enum_name[0] < 'A' || 'Z' < enum_name[0]){
            enum_name = "L" + enum_name;
        }
        
        if (enum_names.find(enum_name) == enum_names.end()){
            enum_names.insert({enum_name, 1});
        }
        else {
            int cnt = enum_names.at(enum_name)++;
            enum_names.at(enum_name) = cnt;
            enum_name += ('A' + cnt - 1);
            enum_names.insert({enum_name, 1});
        }
        
        data.insert({enum_name, tokens});
    }
    
    offile << "/* DO NOT CHANGE THE FILE, IT IS AUTOMATICALLY GENERATED */" << endl << endl;
    
    
    offile << "/*" << endl;
    offile << "MIT License" << endl;
    offile << endl;
    offile << "Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de" << endl;
    offile << "                   Nils Hoffmann  -  nils.hoffmann {at} isas.de" << endl;
    offile << endl;
    offile << "Permission is hereby granted, free of charge, to any person obtaining a copy" << endl;
    offile << "of this software and associated documentation files (the \"Software\"), to deal" << endl;
    offile << "in the Software without restriction, including without limitation the rights" << endl;
    offile << "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell" << endl;
    offile << "copies of the Software, and to permit persons to whom the Software is" << endl;
    offile << "furnished to do so, subject to the following conditions:" << endl;
    offile << endl;
    offile << "The above copyright notice and this permission notice shall be included in all" << endl;
    offile << "copies or substantial portions of the Software." << endl;
    offile << endl;
    offile << "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR" << endl;
    offile << "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY," << endl;
    offile << "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE" << endl;
    offile << "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER" << endl;
    offile << "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM," << endl;
    offile << "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE" << endl;
    offile << "SOFTWARE." << endl;
    offile << "*/" << endl;
    offile << endl;
    offile << endl;
    offile << endl;
    
    offile << "#ifndef LIPID_ENUMS_H" << endl;
    offile << "#define LIPID_ENUMS_H" << endl;
    offile << "" << endl;
    offile << "#include <vector>" << endl;
    offile << "#include <map>" << endl;
    offile << "#include <set>" << endl;
    offile << "" << endl;
    offile << "namespace goslin {" << endl;
    offile << "using namespace std;" << endl;
    offile << "" << endl;
    offile << "enum LipidCategory {NO_CATEGORY," << endl;
    offile << "    UNDEFINED," << endl;
    offile << "    GL, // SLM:000117142 Glycerolipids" << endl;
    offile << "    GP, // SLM:000001193 Glycerophospholipids" << endl;
    offile << "    SP, // SLM:000000525 Sphingolipids" << endl;
    offile << "    ST, // SLM:000500463 Steroids and derivatives" << endl;
    offile << "    FA, // SLM:000390054 Fatty acyls and derivatives" << endl;
    offile << "    PK, // polyketides" << endl;
    offile << "    SL // Saccharo lipids" << endl;
    offile << "};" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "static const map<LipidCategory, string> CategoryString = {" << endl;
    offile << "    {NO_CATEGORY, \"NO_CATEGORY\"}," << endl;
    offile << "    {UNDEFINED, \"UNDEFINED\"}," << endl;
    offile << "    {GL, \"GL\"}," << endl;
    offile << "    {GP, \"GP\"}," << endl;
    offile << "    {SP, \"SP\"}," << endl;
    offile << "    {ST, \"ST\"}," << endl;
    offile << "    {FA, \"FA\"}," << endl;
    offile << "    {SL, \"SL\"}" << endl;
    offile << "};" << endl;
    offile << "" << endl;
    offile << "    " << endl;
    offile << "    " << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "enum LipidLevel {NO_LEVEL," << endl;
    offile << "    UNDEFINED_LEVEL," << endl;
    offile << "    CATEGORY, // Mediators, Glycerolipids, Glycerophospholipids, Sphingolipids, Steroids, Prenols" << endl;
    offile << "    CLASS, // Glyerophospholipids -> Glycerophosphoinositols (PI)" << endl;
    offile << "    SPECIES, // Phosphatidylinositol (16:0) or PI(16:0)" << endl;
    offile << "    MOLECULAR_SUBSPECIES, // Phosphatidylinositol (8:0-8:0) or PI(8:0-8:0)" << endl;
    offile << "    STRUCTURAL_SUBSPECIES, // Phosphatidylinositol (8:0/8:0) or PI(8:0/8:0)" << endl;
    offile << "    ISOMERIC_SUBSPECIES // e.g. Phosphatidylethanolamine (P-18:0/22:6(4Z,7Z,10Z,13Z,16Z,19Z))" << endl;
    offile << "};" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "struct LipidClassMeta {" << endl;
    offile << "    LipidCategory lipid_category;" << endl;
    offile << "    string class_name;" << endl;
    offile << "    int max_num_fa;" << endl;
    offile << "    set<int> possible_num_fa;" << endl;
    offile << "    vector<string> synonyms;" << endl;
    offile << "};" << endl;
    offile << "" << endl;
    offile << "enum LipidClass {NO_CLASS, UNDEFINED_CLASS";
    
    for (auto& kv : data){
        offile << ", " << kv.first;
    }
    offile << "};" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "typedef map<LipidClass, LipidClassMeta> ClassMap;" << endl;
    offile << "enum LipidFaBondType { NO_FA, UNDEFINED_FA, ESTER, ETHER_PLASMANYL, ETHER_PLASMENYL};" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "static const ClassMap lipid_classes = {" << endl;
    unsigned int cnt = 0;
    for (auto& kv : data){
        offile << "{" << kv.first << ", {" << kv.second->at(1) << ", \"" << kv.second->at(2) << "\", ";
        offile << kv.second->at(3) << ", {";
        
        
        vector<string>* tokens = split_string(kv.second->at(4), '|', '"');
        for (unsigned int i = 0; i < tokens->size(); ++i){
            string tok = strip(tokens->at(i), ' ');
            if (i > 0) offile << ", ";
            offile << tok;
        }
        delete tokens;
        
        offile << "}, {\"" << kv.second->at(0) << "\"";
        for (unsigned int i = 5; i < kv.second->size(); ++i){
            string synonym = kv.second->at(i);
            if (synonym.length() < 1) continue;
            offile << ", \"" << synonym << "\"";
        }
        offile << "} } }" << (++cnt < data.size() ? "," : "") << endl; 
    }

    
    offile << "};" << endl; 
    offile << "" << endl; 
    offile << "static map<LipidClass, string> ClassString;" << endl; 
    offile << "static map<string, LipidClass> StringClass;" << endl; 
    offile << "static map<string, LipidCategory> StringCategory;" << endl; 
    offile << "}" << endl;
    offile << "" << endl; 
    offile << "#endif /* LIPID_ENUMS_H */" << endl; 
}








int main(int argc, char** argv){
    if (argc < 2){
        cout << "Error, specify grammar output filename." << endl;
        exit(-1);
    }

    writeLipidEnum(argv[1]);
    return 0;
}
