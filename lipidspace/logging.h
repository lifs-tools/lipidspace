#ifndef LOGGING_H
#define LOGGING_H

#include <sstream>
#include <string>
#include <ctime>

using namespace std;

class Logging {
public:
    Logging();
    
    static void write_log(string log_string);
    static string get_log();
    
private:
    static stringstream log_stream;
};


#endif // LOGGING_H
