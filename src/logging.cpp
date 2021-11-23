#include "lipidspace/logging.h"

void Logging::write_log(string log_string){
    time_t now = time(0);
    tm *ltm = localtime(&now);

    string month = std::to_string(1 + ltm->tm_mon);
    if (month.length() == 1) month = "0" + month;
    
    string day = std::to_string(ltm->tm_mday);
    if (day.length() == 1) day = "0" + day;
    
    string hour = std::to_string(ltm->tm_hour);
    if (hour.length() == 1) hour = "0" + hour;
    
    string minute = std::to_string(ltm->tm_min);
    if (minute.length() == 1) minute = "0" + minute;
    
    string second = std::to_string(ltm->tm_sec);
    if (second.length() == 1) second = "0" + second;
    
    log_stream << (1900 + ltm->tm_year) << "/" << month << "/" << day << " - " << hour << ":" << minute << ":" << second << " - " << log_string << endl;
}



string Logging::get_log(){
    return log_stream.str();
}



stringstream Logging::log_stream;
