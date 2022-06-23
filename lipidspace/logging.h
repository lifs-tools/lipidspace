#ifndef LOGGING_H
#define LOGGING_H

#include <sstream>
#include <iostream>
#include <string>
#include <ctime>

using namespace std;

class Logging {
public:
    
    static void write_log(string log_string);
    static string get_log();
    static void set_stdout(bool out);
    
private:
    static stringstream log_stream;
    static bool to_stdout;
};


#endif // LOGGING_H
