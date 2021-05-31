#include "GenericDatastructures.h"


GenericList::~GenericList(){
    for (auto& x : list){
        del(x);
    }
}

/*
void GenericList::remove(string key){
    del(dictionary.at(key));
    dictionary.erase(key);
}*/


void GenericList::del(pair<int, void*> &x){
    switch (x.first){
        case TYPE_BOOL:
            delete (bool*)x.second;
            break;
            
        case TYPE_INT:
            delete (int*)x.second;
            break;
            
        case TYPE_LONG:
            delete (long*)x.second;
            break;
            
        case TYPE_FLOAT:
            delete (float*)x.second;
            break;
            
        case TYPE_DOUBLE:
            delete (double*)x.second;
            break;
            
        case TYPE_STRING:
            delete (string*)x.second;
            break;
            
        case TYPE_LIST:
            delete (GenericList*)x.second;
            break;
            
        case TYPE_DICTIONARY:
            delete (GenericList*)x.second;
            break;
            
        default:
            break;
    }
}



void GenericList::setInt(int i){
    void* vi = new int(i);
    list.push_back({TYPE_INT, vi});
}

int GenericList::getInt(int key){
    key += list.size() * (key < 0);
    return *((int*)(list.at(key).second));
}

void GenericList::setLong(long l){
    void* vl = new long(l);
    list.push_back({TYPE_LONG, vl});
}

long GenericList::getLong(int key){
    key += list.size() * (key < 0);
    return *((long*)(list.at(key).second));
}

void GenericList::setFloat(float f){
    void* vf = new float(f);
    list.push_back({TYPE_FLOAT, vf});
}

float GenericList::getFloat(int key){
    key += list.size() * (key < 0);
    return *((float*)(list.at(key).second));
}

void GenericList::setDouble(double d){
    void* vd = new double(d);
    list.push_back({TYPE_DOUBLE, vd});
}

double GenericList::getDouble(int key){
    key += list.size() * (key < 0);
    return *((double*)(list.at(key).second));
}

void GenericList::setString(string s){
    void* vs = new string(s);
    list.push_back({TYPE_STRING, vs});
}

string GenericList::getString(int key){
    key += list.size() * (key < 0);
    return *((string*)(list.at(key).second));
}

void GenericList::setList(GenericList* v){
    list.push_back({TYPE_LIST, v});
}

GenericList* GenericList::getList(int key){
    key += list.size() * (key < 0);
    return (GenericList*)(list.at(key).second);
}


void GenericList::setDictionary(GenericDictionary* dict){
    list.push_back({TYPE_DICTIONARY, (void*)dict});
}


GenericDictionary* GenericList::getDictionary(int key){
    key += list.size() * (key < 0);
    return (GenericDictionary*)(list.at(key).second);
}






GenericDictionary::~GenericDictionary(){
    for (auto& x : dictionary){
        del(x.second);
    }
}

void GenericDictionary::remove(string key){
    del(dictionary.at(key));
    dictionary.erase(key);
}

void GenericDictionary::del(pair<int, void*> &x){
    switch (x.first){
        case TYPE_BOOL:
            delete (bool*)x.second;
            break;
            
        case TYPE_INT:
            delete (int*)x.second;
            break;
            
        case TYPE_LONG:
            delete (long*)x.second;
            break;
            
        case TYPE_FLOAT:
            delete (float*)x.second;
            break;
            
        case TYPE_DOUBLE:
            delete (double*)x.second;
            break;
            
        case TYPE_STRING:
            delete (string*)x.second;
            break;
            
        case TYPE_LIST:
            delete (GenericList*)x.second;
            break;
            
        case TYPE_DICTIONARY:
            delete (GenericDictionary*)x.second;
            break;
            
        default:
            break;
    }
}

void GenericDictionary::setInt(string key, int i){
    void* vi = new int(i);
    dictionary.insert({key, {TYPE_INT, vi}});
}

int GenericDictionary::getInt(string key){
    return *((int*)(dictionary.at(key).second));
}

void GenericDictionary::setLong(string key, long l){
    void* vl = new long(l);
    dictionary.insert({key, {TYPE_LONG, vl}});
}

long GenericDictionary::getLong(string key){
    return *((long*)(dictionary.at(key).second));
}

void GenericDictionary::setFloat(string key, float f){
    void* vf = new float(f);
    dictionary.insert({key, {TYPE_FLOAT, vf}});
}

float GenericDictionary::getFloat(string key){
    return *((float*)(dictionary.at(key).second));
}

void GenericDictionary::setDouble(string key, double d){
    void* vd = new double(d);
    dictionary.insert({key, {TYPE_DOUBLE, vd}});
}

double GenericDictionary::getDouble(string key){
    return *((double*)(dictionary.at(key).second));
}

void GenericDictionary::setString(string key, string s){
    void* vs = new string(s);
    dictionary.insert({key, {TYPE_STRING, vs}});
}

string GenericDictionary::getString(string key){
    return *((string*)(dictionary.at(key).second));
}

void GenericDictionary::setList(string key, GenericList* v){
    dictionary.insert({key, {TYPE_LIST, v}});
}

GenericList* GenericDictionary::getList(string key){
    return (GenericList*)(dictionary.at(key).second);
}


void GenericDictionary::setDictionary(string key, GenericDictionary* dict){
    dictionary.insert({key, {TYPE_DICTIONARY, (void*)dict}});
}


GenericDictionary* GenericDictionary::getDictionary(string key){
    return (GenericDictionary*)(dictionary.at(key).second);
}

