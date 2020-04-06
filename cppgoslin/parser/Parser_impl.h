/*
MIT License

Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de
                   Nils Hoffmann  -  nils.hoffmann {at} isas.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

template <class T>
const uint32_t Parser<T>::SHIFT = 32;
template <class T>
const uint64_t Parser<T>::MASK = (1ull << SHIFT) - 1ull;
template <class T>
const char Parser<T>::RULE_ASSIGNMENT = ':';
template <class T>
const char Parser<T>::RULE_SEPARATOR = '|';
template <class T>
const char Parser<T>::RULE_TERMINAL = ';';
template <class T>
const char Parser<T>::EOF_SIGN = (char)1;
template <class T>
const uint64_t Parser<T>::EOF_RULE = 1ull;
template <class T>
const uint64_t Parser<T>::START_RULE = 2ull;
template <class T>
const string Parser<T>::EOF_RULE_NAME = "EOF";


#include <iostream>
#define contains(X, Y) ((X).find(Y) != (X).end())
#define contains_p(X, Y) ((X)->find(Y) != (X)->end())
#define uncontains(X, Y) ((X).find(Y) == (X).end())
#define uncontains_p(X, Y) ((X)->find(Y) == (X)->end())


template <class T>
uint64_t Parser<T>::get_next_free_rule_index(){
    if (next_free_rule_index <= MASK){
        return next_free_rule_index++;
    }
    throw RuntimeException("Error: grammar is too big.");
}




template <class T>
Parser<T>::~Parser(){
    for (auto& kv : substitution){
        delete kv.second;
    }
    
    for (auto& b : right_pair){
        delete b;
    }
}


template <class T>
Parser<T>::Parser(BaseParserEventHandler<T> *_parserEventHandler, GrammarString grammar_string, char _quote){
    
    quote = _quote;
    parser_event_handler = _parserEventHandler;
    
    read_grammar(grammar_string);
}


template <class T>
Parser<T>::Parser(BaseParserEventHandler<T> *_parserEventHandler, string grammar_filename, char _quote){
    
    quote = _quote;
    parser_event_handler = _parserEventHandler;
    
    ifstream f(grammar_filename);
    if (!f.good()){
        throw RuntimeException("Error: file '" + grammar_filename + "' does not exist or can not be opened.");
    }
    
    string grammar( (std::istreambuf_iterator<char>(f)), (std::istreambuf_iterator<char>()));
    f.close();
    read_grammar(grammar);
}

    
template <class T>
void Parser<T>::read_grammar(string grammar){
    next_free_rule_index = START_RULE;
    word_in_grammar = false;
    grammar_name = "";
    used_eof = false;
    map<string, uint64_t> ruleToNT;
    
    
    // interpret the rules and create the structure for parsing
    vector<string> *rules = extract_text_based_rules(grammar, quote);
    vector<string> *tokens = split_string(rules->at(0), ' ', quote);
    grammar_name = tokens->at(1);
    delete tokens;
    
    
    rules->erase(rules->begin());
    ruleToNT.insert({EOF_RULE_NAME, EOF_RULE});
    TtoNT.insert({EOF_SIGN, set<uint64_t>()});
    TtoNT.at(EOF_SIGN).insert(EOF_RULE);
    
    for (auto rule_line : *rules){
        
        vector<string> tokens_level_1;
        vector<string> *line_tokens = split_string(rule_line, RULE_ASSIGNMENT, quote);
        for (auto t : *line_tokens) tokens_level_1.push_back(strip(t, ' '));
        delete line_tokens;
            
        if (tokens_level_1.size() != 2){
            delete rules;
            throw RuntimeException("Error: corrupted token in grammar rule: '" + rule_line + "'");
        }
        
        vector<string> *rule_tokens = split_string(tokens_level_1.at(0), ' ', quote);
        if (rule_tokens->size() > 1) {
            delete rule_tokens;
            delete rules;
            throw RuntimeException("Error: several rule names on left hand side in grammar rule: '" + rule_line + "'");
        }
        delete rule_tokens;

        string rule = tokens_level_1.at(0);
        
        if (rule == EOF_RULE_NAME){
            throw RuntimeException("Error: rule name is not allowed to be called EOF");
        }
        
        vector<string>* products = split_string(tokens_level_1.at(1), RULE_SEPARATOR, quote);
        for (uint64_t i = 0; i < products->size(); ++i){
            products->at(i) = strip(products->at(i), ' ');
        }
        
        if (uncontains(ruleToNT, rule)){
            ruleToNT.insert({rule, get_next_free_rule_index()});
        }
        uint64_t new_rule_index = ruleToNT.at(rule);
        
        if (uncontains(NTtoRule, new_rule_index)){
            NTtoRule.insert({new_rule_index, rule});
        }
        
        
        for (auto product : *products){
            vector<string> non_terminals;
            vector<uint64_t> non_terminal_rules;
            vector<string> *product_rules = split_string(product, ' ', quote);
            for (auto NT : *product_rules){
                string stripedNT = strip(NT, ' ');
                if (is_terminal(stripedNT, quote)) stripedNT = de_escape(stripedNT, quote);
                non_terminals.push_back(stripedNT);
                used_eof |= (stripedNT == EOF_RULE_NAME);
            }
            delete product_rules;
            
            string NTFirst = non_terminals.at(0);
            if (non_terminals.size() > 1 || !is_terminal(NTFirst, quote) || NTFirst.length() != 3){
                for (auto non_terminal : non_terminals){
                    
                    if (is_terminal(non_terminal, quote)){
                        non_terminal_rules.push_back(add_terminal(non_terminal));
                    }
                        
                    else{
                        if (uncontains(ruleToNT, non_terminal)){
                            ruleToNT.insert({non_terminal, get_next_free_rule_index()});
                        }
                        non_terminal_rules.push_back(ruleToNT.at(non_terminal));
                    }
                }
            }
            else{
                char c = NTFirst[1];
                uint64_t tRule = 0;
                if (uncontains(TtoNT, c)){
                    tRule = get_next_free_rule_index();
                    TtoNT.insert({c, set<uint64_t>()});
                    TtoNT.at(c).insert(tRule);
                    
                }
                else {
                    tRule = *TtoNT.at(c).begin();
                }
                
                if (uncontains(NTtoNT, tRule)) NTtoNT.insert({tRule, set<uint64_t>()});
                NTtoNT.at(tRule).insert(new_rule_index);
            }
            
            // more than two rules, insert intermediate rule indexes
            while (non_terminal_rules.size() > 2){
                uint64_t rule_index_2 = non_terminal_rules.back();
                non_terminal_rules.pop_back();
                uint64_t rule_index_1 = non_terminal_rules.back();
                non_terminal_rules.pop_back();
                
                uint64_t key = compute_rule_key(rule_index_1, rule_index_2);
                uint64_t next_index = get_next_free_rule_index();
                if (uncontains(NTtoNT, key)) NTtoNT.insert({key, set<uint64_t>()});
                NTtoNT.at(key).insert(next_index);
                non_terminal_rules.push_back(next_index);
            }
                
            // two product rules
            if (non_terminal_rules.size() == 2){
                uint64_t rule_index_2 = non_terminal_rules.at(1);
                uint64_t rule_index_1 = non_terminal_rules.at(0);
                uint64_t key = compute_rule_key(rule_index_1, rule_index_2);
                if (uncontains(NTtoNT, key)) NTtoNT.insert({key, set<uint64_t>()});
                NTtoNT.at(key).insert(new_rule_index);
            }
            
            // only one product rule
            else if (non_terminal_rules.size() == 1){
                uint64_t rule_index_1 = non_terminal_rules.at(0);
                if (rule_index_1 == new_rule_index){
                    delete products;
                    delete rules;
                    throw RuntimeException("Error: corrupted token in grammar: rule '" + rule + "' is not allowed to refer soleley to itself.");
                }
                
                if (uncontains(NTtoNT, rule_index_1)) NTtoNT.insert({rule_index_1, set<uint64_t>()});
                NTtoNT.at(rule_index_1).insert(new_rule_index);
            }
        }
        
        delete products;
    }
    delete rules;
    
    // adding all rule names into the event handler
    for (auto rule_name : ruleToNT) parser_event_handler->rule_names.insert(rule_name.first);
        
    parser_event_handler->parser = this;
    parser_event_handler->sanity_check();
    
    
    // keeping the original terminal dictionary
    for (auto& kv : TtoNT){
        for (auto& rule : kv.second){
            originalTtoNT.insert({kv.first, rule});
            break;
        }
    }
    
    
    // creating substitution dictionary for adding single rule chains into the parsing tree
    for (auto& kv : NTtoNT){
        for (auto& rule : kv.second){
            vector<uint64_t>* topnodes = collect_one_backwards(rule);
            for (auto& rule_top : *topnodes){
                vector<uint64_t>* chain = collect_backwards(rule, rule_top);
                if (chain){
                    chain->push_back(rule);
                    
                    uint64_t key = kv.first + (rule_top << 16);
                    substitution.insert({key, chain});
                }
            }
            delete topnodes;
        }
    }

    // expanding terminal dictionary for single rule chains
    set<uint64_t> keys;
    for (auto key : TtoNT) keys.insert(key.first);
    for (auto c : keys){
        set<uint64_t> rules;
        for (auto rule : TtoNT.at(c)) rules.insert(rule);
                             
        for (auto rule : rules){
            vector<uint64_t> *backward_rules = collect_one_backwards(rule);
            for (auto p : *backward_rules) TtoNT.at(c).insert(p);
            delete backward_rules;
        }
    }
    
    
    
    // expanding non-terminal dictionary for single rule chains
    set<uint64_t> keysNT;
    for (auto k : NTtoNT) keysNT.insert(k.first);
    for (auto r : keysNT){
        set<uint64_t> rules;
        for (auto rr : NTtoNT.at(r)) rules.insert(rr);
                                                                   
        for (auto rule : rules){
            vector<uint64_t> *backward_rules = collect_one_backwards(rule);
            for (auto p : *backward_rules) NTtoNT.at(r).insert(p);
            delete backward_rules;
        }
    }

    
    // creating lookup table for right index pairs to a given left index
    for (uint64_t i = 0; i < next_free_rule_index; ++i){
        right_pair.push_back(new Bitfield(next_free_rule_index));
    }
    
    for (auto& kvp : NTtoNT){
        if (kvp.first <= MASK) continue;
        right_pair.at(kvp.first >> SHIFT)->insert(kvp.first & MASK);
    }
    
    
}


template <class T>
vector<string>* Parser<T>::extract_text_based_rules(string grammar, char _quote){
    vector<string> *rules = NULL;
    int grammar_length = grammar.length();
    
    /*
    deleting comments to prepare for splitting the grammar in rules.
    Therefore, we have to consider three different contexts, namely
    within a quote, within a line comment, within a long comment.
    As long as we are in one context, key words for starting / ending
    the other contexts have to be ignored.
    */
    stringstream sb;
    Content current_context = NoContext;
    int current_position = 0;
    int last_escaped_backslash = -1;
    
    for (int i = 0; i < grammar_length - 1; ++i){
        MatchWords match = NoMatch;
        
        if (i > 0 && grammar[i] == '\\' && grammar[i - 1] == '\\' && last_escaped_backslash != i - 1){
            last_escaped_backslash = i;
            continue;
        }
        
        if (grammar[i] == '/' && grammar[i + 1] == '/') match = LineCommentStart;
        else if (grammar[i] == '\n') match = LineCommentEnd;
        else if (grammar[i] == '/' && grammar[i + 1] == '*') match = LongCommentStart;
        else if (grammar[i] == '*' && grammar[i + 1] == '/') match = LongCommentEnd;
        else if (grammar[i] == _quote &&  !(i >= 1 && grammar[i - 1] == '\\' && i - 1 != last_escaped_backslash)) match = Quote;
        
        if (match != NoMatch){
            switch (current_context){
                case NoContext:
                    switch (match){
                        case LongCommentStart:
                            sb << grammar.substr(current_position, i - current_position);
                            current_context = InLongComment;
                            break;
                            
                        case LineCommentStart:
                            sb << grammar.substr(current_position, i - current_position);
                            current_context = InLineComment;
                            break;
                            
                        case Quote:
                            current_context = InQuote;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case InQuote:
                    if (match == Quote) {
                        current_context = NoContext;
                    }
                    break;
                    
                    
                case InLineComment:
                    if (match == LineCommentEnd) {
                        current_context = NoContext;
                        current_position = i + 1;
                    }
                    break;
                    
                case InLongComment:
                    if (match == LongCommentEnd) {
                        current_context = NoContext;
                        current_position = i + 2;
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    if (current_context == NoContext){
        sb << grammar.substr(current_position, grammar_length - current_position);
    }
    else {
        throw RuntimeException("Error: corrupted grammar, ends either in comment or quote");
    }
    
    grammar = sb.str();
    grammar = replace_all(grammar, "\r\n", "");
    grammar = replace_all(grammar, "\n", "");
    grammar = replace_all(grammar, "\r", "");
    grammar = strip(grammar, ' ');
    
    
    if (grammar[grammar.length() - 1] != RULE_TERMINAL){
        throw RuntimeException("Error: corrupted grammar, last rule has no termininating sign, was: '" + string(1, grammar[grammar.length() - 1]) + "'");
    }
    
    rules = split_string(grammar, RULE_TERMINAL, _quote);
    
    if (rules->size() < 1){
        throw RuntimeException("Error: corrupted grammar, grammar is empty");
    }
    vector<string> *grammar_name_rule = split_string(rules->at(0), ' ', _quote);
    
    if (grammar_name_rule->size() > 0 && grammar_name_rule->at(0) != "grammar"){
        delete grammar_name_rule;
        throw RuntimeException("Error: first rule must start with the keyword 'grammar'");
    }
    
    
    else if (grammar_name_rule->size() != 2){
        delete grammar_name_rule;
        throw RuntimeException("Error: incorrect first rule");
    }
    
    delete grammar_name_rule;
    return rules;
}







template <class T>
uint64_t Parser<T>::compute_rule_key(uint64_t rule_index_1, uint64_t rule_index_2){
    return (rule_index_1 << SHIFT) | rule_index_2;
}







// checking if string is terminal
template <class T>
bool Parser<T>::is_terminal(string product_token, char _quote){
    return product_token[0] == _quote && product_token[product_token.length() - 1] == _quote && product_token.length() > 2;
}




template <class T>
string Parser<T>::de_escape(string text, char _quote){
    // remove the escape chars
    stringstream sb;
    bool last_escape_char = false;
    for (uint64_t i = 0; i < text.length(); ++i){
        char c = text[i];
        bool escape_char = false;
        
        if (c != '\\') sb << c;
            
        else{
            if (!last_escape_char) escape_char = true;
            else sb << c;
        }
        
        last_escape_char = escape_char;
    
    }
    string sb_string;
    sb_string = sb.str();
    return sb_string;
}


// splitting the whole terminal in a tree structure where characters of terminal are the leafs and the inner nodes are added non terminal rules
template <class T>
uint64_t Parser<T>::add_terminal(string text){
    vector<uint64_t> terminal_rules;
    for (uint64_t i = 1; i < text.length() - 1; ++i){
        char c = text[i];
        uint64_t tRule = 0;
        if (uncontains(TtoNT, c)){
            tRule = get_next_free_rule_index();
            TtoNT.insert({c, set<uint64_t>()});
            TtoNT.at(c).insert(tRule);
        }
        else {
            tRule = *TtoNT.at(c).begin();
        }
        terminal_rules.push_back(tRule);
    }
    
    while (terminal_rules.size() > 1){
        uint64_t rule_index_2 = terminal_rules.back();
        terminal_rules.pop_back();
        uint64_t rule_index_1 = terminal_rules.back();
        terminal_rules.pop_back();
        
        uint64_t next_index = get_next_free_rule_index();
        
        uint64_t key = compute_rule_key(rule_index_1, rule_index_2);
        if (uncontains(NTtoNT, key)) NTtoNT.insert({key, set<uint64_t>()});
        NTtoNT.at(key).insert(next_index);
        terminal_rules.push_back(next_index);
    }
    return terminal_rules.at(0);
}


template <class T>
vector<uint64_t>* Parser<T>::top_nodes(uint64_t rule_index){
    vector<uint64_t> *collection = new vector<uint64_t>();
    vector<uint64_t> *collection_top = new vector<uint64_t>();
    collection->push_back(rule_index);
    uint64_t i = 0;
    while (i < collection->size()){
        uint64_t current_index = collection->at(i);
        if (uncontains(NTtoNT, current_index)){
            for (auto previous_index : NTtoNT.at(current_index)) collection->push_back(previous_index);
        }
        else {
            collection_top->push_back(current_index);
        }
        i += 1;
    }
    delete collection;
    
    return collection_top;
}


// expanding singleton rules, e.g. S -> A, A -> B, B -> C
template <class T>
vector<uint64_t>* Parser<T>::collect_one_backwards(uint64_t rule_index){
    vector<uint64_t> *collection = new vector<uint64_t>();
    collection->push_back(rule_index);
    uint64_t i = 0;
    while (i < collection->size()){
        uint64_t current_index = collection->at(i);
        if (contains(NTtoNT, current_index)){
            for (auto previous_index : NTtoNT.at(current_index)) collection->push_back(previous_index);
        }
        i += 1;
    }
    
    return collection;
}



template <class T>
vector<uint64_t>* Parser<T>::collect_backwards(uint64_t child_rule_index, unsigned parent_rule_index){
    if (uncontains(NTtoNT, child_rule_index)) return NULL;
    
    for (auto previous_index : NTtoNT.at(child_rule_index)){
        if (previous_index == parent_rule_index) return new vector<uint64_t>;
        
        else if (contains(NTtoNT, previous_index)){
            vector<uint64_t>* collection = collect_backwards(previous_index, parent_rule_index);
            if (collection != NULL){
                collection->push_back(previous_index);
                return collection;
            }
        }
    }
    return NULL;
}
    
    
    
    

template <class T>
void Parser<T>::raise_events(TreeNode *node){
    if (node != NULL){
        string node_rule_name = node->fire_event ?  NTtoRule.at(node->rule_index) : "";
        if (node->fire_event) parser_event_handler->handle_event(node_rule_name + "_pre_event", node);
        
        if (node->left != NULL) { // node.terminal is != None when node is leaf
            raise_events(node->left);
            if (node->right != NULL) raise_events(node->right);
        }
            
        if (node->fire_event) parser_event_handler->handle_event(node_rule_name + "_post_event", node);
    }
}





// filling the syntax tree including events
template <class T>
void Parser<T>::fill_tree(TreeNode *node, DPNode *dp_node){
    // checking and extending nodes for single rule chains
    
    uint64_t bottom_rule = 0, top_rule = 0;
    if (dp_node->left != NULL){
        bottom_rule = compute_rule_key(dp_node->rule_index_1, dp_node->rule_index_2);
        top_rule = node->rule_index;
    }
    else {
        top_rule = dp_node->rule_index_2;
        bottom_rule = originalTtoNT.at(dp_node->rule_index_1);
    }
    
    uint64_t subst_key = bottom_rule + (top_rule << 16);
    
    if ((bottom_rule != top_rule) and (contains(substitution, subst_key))){
        for (auto& rule_index : *substitution.at(subst_key)){
            node->left = new TreeNode(rule_index, contains(NTtoRule, rule_index));
            node = node->left;
        }
    }


    
    if (dp_node->left != NULL) { // None => leaf
        node->left = new TreeNode(dp_node->rule_index_1, contains(NTtoRule, dp_node->rule_index_1));
        node->right = new TreeNode(dp_node->rule_index_2, contains(NTtoRule, dp_node->rule_index_2));
        fill_tree(node->left, dp_node->left);
        fill_tree(node->right, dp_node->right);
    }
    else {
        // I know, it is not 100% clean to store the character in an integer
        // especially when it is not the dedicated attribute for, but the heck with it!
        node->terminal = dp_node->rule_index_1;
    }
}



// re-implementation of Cocke-Younger-Kasami algorithm
template <class T>
T Parser<T>::parse(string text_to_parse){
    if (used_eof) text_to_parse += string(1, EOF_SIGN);
    parser_event_handler->content = NULL;
    parser_event_handler->parse_string = text_to_parse;
    
    parse_regular(text_to_parse);
    return parser_event_handler->content;
}
    
    
    
    
template <class T>
void Parser<T>::parse_regular(string text_to_parse){
    word_in_grammar = false;
    
    int n = text_to_parse.length();
    // dp stands for dynamic programming, nothing else
    map<uint64_t, DPNode*> ***DP = new map<uint64_t, DPNode*>**[n];
    vector<DPNode*> DPnodes;
    
    // Ks is a lookup, which fields in the DP are filled
    Bitfield **Ks = new Bitfield*[n];
    
    
    // init the tables
    for (int i = 0; i < n; ++i){
        DP[i] = new map<uint64_t, DPNode*>*[n - i];
        for (int j = 0; j < n - i; ++j){
            DP[i][j] = new map<uint64_t, DPNode*>();
        }
        Ks[i] = new Bitfield(n);
    }
    
    bool requirement_fulfilled = true;
    for (int i = 0; i < n; ++i){
        char c = text_to_parse[i];
        if (uncontains(TtoNT, c)) {
            requirement_fulfilled = false;
            break;
        }
            
        for (auto T_rule_index : TtoNT.at(c)){
            DPNode *dp_node = new DPNode(c, T_rule_index, NULL, NULL);
            DP[i][0]->insert({T_rule_index, dp_node});
            DPnodes.push_back(dp_node);
        }
        Ks[i]->insert(0);
    }

    
    if (requirement_fulfilled){
        for (int i = 1; i < n; ++i){
            int im1 = i - 1;
            
            for (int j = 0; j < n - i; ++j){
                map<uint64_t, DPNode*>* DPji = DP[j][i];
                int jp1 = j + 1;
                
                for (auto k : *Ks[j]){
                    int jpok = jp1 + k;
                    int im1mk = im1 - k;
                    if (Ks[jpok]->find(im1mk)){
                    
                        for (auto index_pair_1 : *DP[j][k]){
                            Bitfield* b = right_pair.at(index_pair_1.first);
                            for (auto index_pair_2 : *DP[jpok][im1mk]){
                                
                                if (b->find(index_pair_2.first)){
                                    uint64_t key = compute_rule_key(index_pair_1.first, index_pair_2.first);
                                    
                                    DPNode *content = new DPNode(index_pair_1.first, index_pair_2.first, index_pair_1.second, index_pair_2.second);
                                    DPnodes.push_back(content);
                                    for (auto rule_index : NTtoNT.at(key)){
                                        DPji->insert({rule_index, content});
                                    }
                                    
                                }
                            }
                        }
                    }
                    
                }
                if (DPji->size() > 0) Ks[j]->insert(i);
            }
        }
        
        for (int i = n - 1; i > 0; --i){
            if (contains_p(DP[0][i], START_RULE)){
                word_in_grammar = true;
                TreeNode parse_tree(START_RULE, contains(NTtoRule, START_RULE));
                fill_tree(&parse_tree, DP[0][i]->at(START_RULE));
                raise_events(&parse_tree);
                break;
            }
        }
    }
    
    // delete tables
    for (auto dp_node : DPnodes) delete dp_node;
    for (int i = 0; i < n; ++i){
        for (int j = 0; j < n - i; ++j){
            delete DP[i][j];
        }
        delete[] DP[i];
        delete Ks[i];
    }
    delete[] DP;
    delete[] Ks;
}


