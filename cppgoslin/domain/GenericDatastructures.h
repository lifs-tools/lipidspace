#ifndef GENERICDATASTRUCTURES_H
#define GENERICDATASTRUCTURES_H

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <tuple>
#include "cppgoslin/domain/StringFunctions.h"

enum Type {TYPE_INT, TYPE_LONG, TYPE_FLOAT, TYPE_DOUBLE, TYPE_BOOL, TYPE_STRING, TYPE_LIST, TYPE_DICTIONARY};

using namespace std;

class GenericDictionary;

class GenericList {
public:
    ~GenericList();
    vector<pair<int, void*>> list;
    
    void set_int(int i);
    int get_int(int i);
    
    void set_long(long l);
    long get_long(int i);
    
    void set_float(float f);
    float get_float(int i);
    
    void set_double(double d);
    double get_double(int i);
    
    void set_string(string s);
    string get_string(int i);
    
    void set_list(GenericList* v);
    GenericList* get_list(int i);
    
    void set_dictionary(GenericDictionary* d);
    GenericDictionary* get_dictionary(int i);
    
    void del(pair<int, void*> &x);
    void remove_all();
};




class GenericDictionary {
public:
    ~GenericDictionary();
    map<string, pair<int, void*>> dictionary;
    
    void set_int(string key, int i);
    int get_int(string key);
    
    void set_long(string key, long l);
    long get_long(string key);
    
    void set_float(string key, float f);
    float get_float(string key);
    
    void set_double(string key, double d);
    double get_double(string key);
    
    void set_string(string key, string s);
    string get_string(string key);
    
    void set_list(string key, GenericList* v);
    GenericList* get_list(string key);
    
    void set_dictionary(string key, GenericDictionary* dict);
    GenericDictionary* get_dictionary(string key);
    
    void remove(string key);
    void remove_all();
    void del(pair<int, void*> &x);
    bool contains_key(string key);
};

#endif /* GENERICDATASTRUCTURES_H */
