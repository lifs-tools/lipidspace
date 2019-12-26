#ifndef LIPID_EXCEPTIONS_H
#define LIPID_EXCEPTIONS_H

#include <string>

using namespace std;

class Exception {
public:
    string message;
    Exception(string _message){
        message = _message;
    }
    
    string what() const throw(){
        return message;
    }
};


class IllegalArgumentException : Exception {
public:
    IllegalArgumentException(string message) : Exception(message){
        
    }
};


class ConstraintViolationException : Exception {
public:
    ConstraintViolationException(string message) : Exception(message){
        
    }
};


class RuntimeException : Exception {
public:
    RuntimeException(string message) : Exception(message){
        
    }
};


#endif /* LIPID_EXCEPTIONS_H */