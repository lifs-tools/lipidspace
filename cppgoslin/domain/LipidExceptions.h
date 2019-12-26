#ifndef LIPID_EXCEPTIONS_H
#define LIPID_EXCEPTIONS_H

#include <string>

using namespace std;

class LipidException {
public:
    string message;
    LipidException(string _message){
        message = _message;
    }
    
    string what() const throw(){
        return message;
    }
};


class IllegalArgumentException : public LipidException {
public:
    IllegalArgumentException(string message) : LipidException(message){
        
    }
};


class ConstraintViolationException : public LipidException {
public:
    ConstraintViolationException(string message) : LipidException(message){
        
    }
};


class RuntimeException : public LipidException {
public:
    RuntimeException(string message) : LipidException(message){
        
    }
};


#endif /* LIPID_EXCEPTIONS_H */
