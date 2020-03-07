#ifndef STRINGFUNCTIONS_H
#define STRINGFUNCTIONS_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "cppgoslin/domain/LipidExceptions.h"


static const char DEFAULT_QUOTE = '\'';

using namespace std;

string replace_all(std::string str, const std::string& from, const std::string& to);
string strip(string s, char c);
vector<string>* split_string(string text, char separator, char _quote = DEFAULT_QUOTE);

#endif /* STRINGFUNCTIONS_H */
