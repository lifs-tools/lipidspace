#ifndef BASE_PARSER_EVENT_HANDLER_H
#define BASE_PARSER_EVENT_HANDLER_H

#include <set>
#include <map>
#include <string>
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/domain/Parser.h"

using namespace std;

class BaseParserEventHandler {
public:
    Parser* parser;
    map<string, void(*)()> registered_events;
    set<string> rule_names;
    
    
    BaseParserEventHandler();
    void sanity_check(self);
    void handle_event(self, event_name, node);
    static bool endswith(const string &main_str, const string to_match);
    static replace(string& str, const string& from, const string& to);
};
            
#endif /*  BASE_PARSER_EVENT_HANDLER_H */
            
