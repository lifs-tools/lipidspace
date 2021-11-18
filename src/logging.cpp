#include "lipidspace/logging.h"

void Logging::write_log(string log_string){
    time_t now = time(0);
    tm *ltm = localtime(&now);

    log_stream << (1900 + ltm->tm_year) << "/" << (1 + ltm->tm_mon) << "/" << ltm->tm_mday << " - " << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << ":   " << log_string << endl;
}



string Logging::get_log(){
    return log_stream.str();
}



stringstream Logging::log_stream;
