#ifndef FRAGMENT_H
#define FRAGMENT_H

#include <string>

using namespace std;

class Fragment {
public:
    string name;
    
    Fragment(string _name);
    string get_lipid_string();
        
};

#endif /* FRAGMENT_H */
