#ifndef BASE_PARSER_EVENT_HANDLER_H
#define BASE_PARSER_EVENT_HANDLER_H

#include <set>
#include <map>
#include <string>
#include <functional>
#include "cppgoslin/domain/LipidExceptions.h"
#include "cppgoslin/parser/Parser.h"

class Parser;
class TreeNode;

using namespace std;

class BaseParserEventHandler {
public:
    Parser* parser;
    map<string, function<void(TreeNode *)>> registered_events;
    set<string> rule_names;
    
    
    BaseParserEventHandler();
    void sanity_check();
    void handle_event(string event_name, TreeNode *node);
    static bool endswith(const string &main_str, const string &to_match);
};
            
#endif /*  BASE_PARSER_EVENT_HANDLER_H */
            
