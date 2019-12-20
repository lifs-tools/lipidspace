#ifndef LIPID_ENUMS_H
#define LIPID_ENUMS_H

#include <string>

using namespace std;

class Fragment {
public:
    string name;
    
    Fragment(string _name);
    string get_lipid_string();
        
};

#endif /* LIPID_ENUMS_H */