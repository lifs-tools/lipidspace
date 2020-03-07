#include <fstream>
#include <iostream>
#include <map>
#include <stdlib.h>
#include "cppgoslin/domain/StringFunctions.h"

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






void writeLipidEnum(string ofFileName){
    ofstream offile(ofFileName.c_str());
    
    ifstream infile("data/goslin/lipid-list.csv");
    if (!infile.good()){
        cout << "Error: file 'data/goslin/lipid-list.csv' not found." << endl;
        exit(-1);
    }
    
    string line;
    int i = 0;
    map<string, int> enum_names;
    
    map<string, vector<string>*> data;
    while (getline(infile, line)){
        if (i++ == 0) continue;
        vector<string>* tokens = split_string(line, ',', '"');
        for (int i = 0; i < tokens->size(); ++i){
            string s = tokens->at(i);
            if (s.length() >= 2 && s[0] == '"' && s[s.length() - 1] == '"'){
                tokens->at(i) = s.substr(1, s.length() - 2);
            }
        }
        
        
        string enum_name = tokens->at(0);
        
        for (int i = 0; i < enum_name.length(); ++i){
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
            enum_name += 'A' + cnt - 2;
            enum_names.at(enum_name) = cnt;
        }
        
        data.insert({enum_name, tokens});
    }
    
    
    offile << "#ifndef LIPID_ENUMS_H" << endl;
    offile << "#define LIPID_ENUMS_H" << endl;
    offile << "" << endl;
    offile << "#include <vector>" << endl;
    offile << "#include <map>" << endl;
    offile << "" << endl;
    offile << "using namespace std;" << endl;
    offile << "" << endl;
    offile << "enum LipidCategory {NO_CATEGORY," << endl;
    offile << "    UNDEFINED_CATEGORY, " << endl;
    offile << "    GL, // SLM:000117142 Glycerolipids" << endl;
    offile << "    GP, // SLM:000001193 Glycerophospholipids" << endl;
    offile << "    SP, // SLM:000000525 Sphingolipids" << endl;
    offile << "    ST, // SLM:000500463 Steroids and derivatives" << endl;
    offile << "    FA, // SLM:000390054 Fatty acyls and derivatives" << endl;
    offile << "    SL // Saccharo lipids" << endl;
    offile << "};" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "" << endl;
    offile << "static const map<LipidCategory, string> CategoryString = {" << endl;
    offile << "    {NO_CATEGORY, \"NO_CATEGORY\"}," << endl;
    offile << "    {UNDEFINED_CATEGORY, \"UNDEFINED_CATEGORY\"}," << endl;
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
    offile << "{UNDEFINED_CLASS, {UNDEFINED_CATEGORY, \"UNDEFINED\", {\"Undefined lipid class\"} } }," << endl;
    for (auto& kv : data){
        offile << "{" << kv.first << ", {" << kv.second->at(1) << ", \"" << kv.second->at(2) << "\", {\"" << kv.second->at(0) << "\"";
        for (int i = 3; i < kv.second->size(); ++i){
            string synonym = kv.second->at(i);
            if (synonym.length() < 1) continue;
            offile << ", \"" << synonym << "\"";
        }
        offile << "} } }," << endl; 
    }

    
    offile << "};" << endl; 
    offile << "" << endl; 
    offile << "static map<LipidClass, string> ClassString;" << endl; 
    offile << "static map<string, LipidClass> StringClass;" << endl; 
    offile << "static map<string, LipidCategory> StringCategory;" << endl; 
    offile << "" << endl; 
    offile << "#endif /* LIPID_ENUMS_H */" << endl; 
}








int main(int argc, char** argv){
    if (argc < 3){
        cout << "Error, specify grammar output filename." << endl;
        exit(-1);
    }

    writeGrammarHeader(argv[1]);
    writeLipidEnum(argv[2]);
    return 0;
}
