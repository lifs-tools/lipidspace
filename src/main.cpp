#include "LipidSpace/auxiliary.h"
#include <iostream>
#include <fstream>
#include <math.h>


vector<string>* split_string(string text, char separator, char _quote, bool with_empty = false){
    bool in_quote = false;
    vector<string> *tokens = new vector<string>();
    stringstream sb;
    char last_char = '\0';
    bool last_escaped_backslash = false;
    
    for (uint32_t i = 0; i < text.length(); ++i){
        char c = text[i];
        bool escaped_backslash = false;
        if (!in_quote){
            if (c == separator){
                string sb_string;
                sb_string = sb.str();
                if (sb_string.length() > 0 || with_empty) tokens->push_back(sb_string);
                sb.str("");
            }
            else{
                if (c == _quote) in_quote = !in_quote;
                sb << c;
            }
        }
        else {
            if (c == '\\' && last_char == '\\' && !last_escaped_backslash){
                escaped_backslash = true;
            }
            else if (c == _quote && !(last_char == '\\' && !last_escaped_backslash)){
                in_quote = !in_quote;
            }
            sb << c;
        }
        
        last_escaped_backslash = escaped_backslash;
        last_char = c;
    }
    
    string sb_string;
    sb_string = sb.str();
    
    if (sb_string.length() > 0 || (last_char == ',' && with_empty)){
        tokens->push_back(sb_string);
    }
    if (in_quote){
        delete tokens;
        cout << "Error: corrupted token in grammar" << endl;
        return 0;
    }
    return tokens;
}




int main(){
    
    LaWeCSE cse;
    
    vector<string> lipid_names;
    vector<InputGraph*> graphs;
    
    string file_name = "lipid_classes.csv";
    
    ifstream infile(file_name);
    string line;
    int aa = 0;
    while (getline(infile, line)){
        vector<string>* lipid_data = split_string(line, '\t', '"');
        if (lipid_data == 0) continue;
        lipid_names.push_back(lipid_data->at(0));
        graphs.push_back(cse.makeGraph(lipid_data->at(1)));
        delete lipid_data;
    }
    infile.close();
    int n = graphs.size();
    
    int*** matrix = new int**[n];
    for (int i = 0; i < n; ++i) matrix[i] = new int*[n];
    
    for (int i = 0; i < n; ++i){
        for (int j = 0; j < n; ++j){
            matrix[i][j] = new int[2];
        }
    }
    
    #pragma omp parallel for
    for (int ii = 0; ii < n * n; ++ii){
        
        int i = ii / n;
        int j = ii % n;
        if (i <= j){
            
            #pragma omp critical
            {
                cout << lipid_names.at(i) << " / " << lipid_names.at(j) << " | " << i << " " << j << " / " << n << endl;
            }
            cse.computeSimilarity(graphs.at(i), graphs.at(j), matrix[i][j]);
            matrix[j][i][0] = matrix[i][j][0];
            matrix[j][i][1] = matrix[i][j][1];
            
            //matrix[j][i] = matrix[i][j] = max(0., 1. / cse.computeSimilarity(graphs.at(i), graphs.at(j)) - 1.);
        }
    }
    
    
    ofstream off("classes-matrix.csv");
    
    for (int i = 0; i < n; ++i){
        off << lipid_names.at(i) << "\t" << lipid_names.at(i) << "\t" << graphs.at(i)->size << "\t" << graphs.at(i)->size << endl;
        for (int j = i + 1; j < n; ++j){
            off << lipid_names.at(i) << "\t" << lipid_names.at(j) << "\t" << matrix[i][j][0] << "\t" << matrix[i][j][1] << endl;
        }
    }
    
    
    for (auto graph : graphs){
        delete graph;
    }
}
