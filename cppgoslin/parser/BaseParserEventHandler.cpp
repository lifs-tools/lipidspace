#include "cppgoslin/domain/BaseParserEventHandler.h"

    
BaseParserEventHandler::BaseParserEventHandler(){
    self.parser = NULL;
}

// checking if all registered events are reasonable and orrur as rules in the grammar
void BaseParserEventHandler::sanity_check(self){
    
    for event_name in self.registered_events:
        if (!endswith(event_name, "_pre_event") && !endswith(event_name, "_post_event")){
            throw RuntimeException("Parser event handler error: event '" + event_name + "' does not contain the suffix '_pre_event' or '_post_event'");
        }
        
        string rule_name = event_name;
        replace(rule_name, "_pre_event", "");
        replace(rule_name, "_post_event", "");
        if (rule_names.find(rule_name) == rule_names.end()){
            throw RuntimeException("Parser event handler error: rule '" + rule_name + "' in event '" + event_name + "' is not present in the grammar" + (parser != NULL ? " '" + parser->grammar_name + "'" : ""));
        }
}


void BaseParserEventHandler::handle_event(string event_name, TreeNode *node){
    if (registered_events.find(event_name) != registered_events.end()){
        registered_events.at(event_name)(node);
    }
}
 
 
bool BaseParserEventHandler::endswith(const string &main_str, const string &to_match){
    if(main_str.size() >= to_match.size() &&
            main_str.compare(main_str.size() - to_match.size(), to_match.size(), to_match) == 0)
            return true;
        else
            return false;
}


bool BaseParserEventHandler::replace(string& str, const string& from, const string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
