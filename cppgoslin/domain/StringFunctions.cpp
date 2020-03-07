#include "cppgoslin/domain/StringFunctions.h"

string strip(string s, char c){
    if (s.length() > 0) {
        uint st = 0;
        while (st < s.length() - 1 && s[st] == c) ++st;
        s = s.substr(st, s.length() - st);
    }
    
    if (s.length() > 0) {
        uint en = 0;
        while (en < s.length() - 1 && s[s.length() - 1 - en] == c) ++en;
        s = s.substr(0, s.length() - en);
    }
    return s;
}



vector<string>* split_string(string text, char separator, char _quote){
    bool in_quote = false;
    vector<string> *tokens = new vector<string>();
    stringstream sb;
    char last_char = '\0';
    bool last_escaped_backslash = false;
    
    for (uint i = 0; i < text.length(); ++i){
        char c = text[i];
        bool escaped_backslash = false;
        if (!in_quote){
            if (c == separator){
                string sb_string;
                sb_string = sb.str();
                if (sb_string.length() > 0) tokens->push_back(sb_string);
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
    
    if (sb_string.length() > 0){
        tokens->push_back(sb_string);
    }
    if (in_quote){
        delete tokens;
        throw RuntimeException("Error: corrupted token in grammar");
    }
    return tokens;
}


string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
