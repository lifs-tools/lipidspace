#include "cppgoslin/parser/BaseParserEventHandler.h"

    
BaseParserEventHandler::BaseParserEventHandler(){
    parser = NULL;
}

// checking if all registered events are reasonable and orrur as rules in the grammar
void BaseParserEventHandler::sanity_check(){
    
    for (auto event_name : registered_events){
        if (!endswith(event_name.first, "_pre_event") && !endswith(event_name.first, "_post_event")){
            throw RuntimeException("Parser event handler error: event '" + event_name.first + "' does not contain the suffix '_pre_event' or '_post_event'");
        }
        
        string rule_name = event_name.first;
        Parser::replace_all(rule_name, "_pre_event", "");
        Parser::replace_all(rule_name, "_post_event", "");
        if (rule_names.find(rule_name) == rule_names.end()){
            throw RuntimeException("Parser event handler error: rule '" + rule_name + "' in event '" + event_name.first + "' is not present in the grammar" + (parser != NULL ? " '" + parser->grammar_name + "'" : ""));
        }
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

