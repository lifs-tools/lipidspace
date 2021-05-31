#ifndef GENERICDATASTRUCTURES_H
#define GENERICDATASTRUCTURES_H

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <tuple>

enum Type {TYPE_INT, TYPE_LONG, TYPE_FLOAT, TYPE_DOUBLE, TYPE_BOOL, TYPE_STRING, TYPE_LIST, TYPE_DICTIONARY};

using namespace std;

class GenericDictionary;

class GenericList {
public:
    ~GenericList();
    vector<pair<int, void*>> list;
    
    void setInt(int i);
    int getInt(int i);
    
    void setLong(long l);
    long getLong(int i);
    
    void setFloat(float f);
    float getFloat(int i);
    
    void setDouble(double d);
    double getDouble(int i);
    
    void setString(string s);
    string getString(int i);
    
    void setList(GenericList* v);
    GenericList* getList(int i);
    
    void setDictionary(GenericDictionary* d);
    GenericDictionary* getDictionary(int i);
    
    void del(pair<int, void*> &x);
};




class GenericDictionary {
public:
    ~GenericDictionary();
    map<string, pair<int, void*>> dictionary;
    
    void setInt(string key, int i);
    int getInt(string key);
    
    void setLong(string key, long l);
    long getLong(string key);
    
    void setFloat(string key, float f);
    float getFloat(string key);
    
    void setDouble(string key, double d);
    double getDouble(string key);
    
    void setString(string key, string s);
    string getString(string key);
    
    void setList(string key, GenericList* v);
    GenericList* getList(string key);
    
    void setDictionary(string key, GenericDictionary* dict);
    GenericDictionary* getDictionary(string key);
    
    void remove(string key);
    void del(pair<int, void*> &x);
};

#endif /* GENERICDATASTRUCTURES_H */
