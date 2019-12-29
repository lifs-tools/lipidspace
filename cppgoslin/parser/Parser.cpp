#include "cppgoslin/parser/Parser.h"



const int Parser::SHIFT = 32;
const unsigned long Parser::MASK = (1ull << SHIFT) - 1ull;
const char Parser::RULE_ASSIGNMENT = ':';
const char Parser::RULE_SEPARATOR = '|';
const char Parser::RULE_TERMINAL = ';';
const char Parser::EOF_SIGN = (char)1;
const unsigned long Parser::EOF_RULE = 1ull;
const unsigned long Parser::START_RULE = 2ull;
const char Parser::DEFAULT_QUOTE = '\'';


DPNode::DPNode(unsigned long _rule1, unsigned long _rule2, DPNode *_left, DPNode *_right){
    rule_index_1 = _rule1;
    rule_index_2 = _rule2;
    left = _left;
    right = _right;
}

      
    
TreeNode::TreeNode(unsigned long _rule, bool _fire_event){
    rule_index = _rule;
    left = NULL;
    right = NULL;
    terminal = 0;
    fire_event = _fire_event;
}


TreeNode::~TreeNode(){
    if (left != NULL) delete left;
    if (right != NULL) delete right;
}


string TreeNode::get_text(){
    if (!terminal){
        string left_str = left->get_text();
        string right_str = right != NULL ? right->get_text() : "";
        return (left_str != string(1, EOF_SIGN) ? left_str : "") + (right_str != string(1, EOF_SIGN) ? right_str : "");
    }
    return string(1, (char)terminal);
}
        

Bitfield::Bitfield(uint length){
    field_len = 1 + ((length + 1) >> 6);
    superfield_len = 1 + ((field_len + 1) >> 6);
    field = new unsigned long[field_len];
    superfield = new unsigned long[superfield_len];
}

Bitfield::~Bitfield(){
    delete []field;
    delete []superfield;
}


void Bitfield::set_bit(uint pos){
    field[pos >> 6] |= (unsigned long)(1ull << (pos & 63));
    superfield[pos >> 12] |= (unsigned long)(1ull << ((pos >> 6) & 63));
}


bool Bitfield::is_set(uint pos){
    return ((field[pos >> 6] >> (pos & 63)) & 1ull) == 1ull;
}


bool Bitfield::is_not_set(uint pos){
    return ((field[pos >> 6] >> (pos & 63)) & 1ull) == 0ull;
}

void Bitfield::print_bitfield(unsigned long l){
    for (int i = 63; i >= 0; --i){
        cout << ((l >> i) & 1);
    } cout << endl;
}

void Bitfield::init(){
    spre = 0;
    si = 0;
    sv = superfield[si];
    fi = 0;
    
    unsigned long sv1 = (unsigned long)(sv & -sv);
    pos = spre + __builtin_ctzll(sv1);
    v = field[pos];
}

int Bitfield::get_bit_positions(){
    do {
        while (sv != 0) {
            // algorithm for getting least significant bit position
            while (v != 0) {
                // algorithm for getting least significant bit position
                unsigned long v1 = (unsigned long)(v & -v);
                v &= v - 1;
                return (pos << 6) + __builtin_ctzll(v1);
            }
            sv &= sv - 1;
            
            if (sv != 0){
                unsigned long sv1 = (unsigned long)(sv & -sv);
                pos = spre + __builtin_ctzll(sv1);
                v = field[pos];
            }
        }
        
        spre += 64;
        if (si < superfield_len){
            sv = superfield[++si];
            unsigned long sv1 = (unsigned long)(sv & -sv);
            pos = spre + __builtin_ctzll(sv1);
            v = field[pos];
        }
    }
    while (si < superfield_len);
    return -1;
}




unsigned long Parser::get_next_free_rule_index(){
    if (next_free_rule_index <= MASK){
        return next_free_rule_index++;
    }
    throw RuntimeException("Error: grammar is too big.");
}


string Parser::strip(string s, char c){
    if (s.length() > 0) {
        uint st = 0;
        while (st < s.length() - 1 && s[st] == c) ++st;
        s = s.substr(st, s.length() - st);
    }
    
    if (s.length() > 0) {
        uint en = 0;
        while (en < s.length() - 1 && s[s.length() - 1 - en] == c) ++en;
        s = s.substr(0, s.length() - en);
    }
    return s;
}


Parser::~Parser(){
    if (parse_tree) delete parse_tree;
    
    for (auto kvp : TtoNT) delete kvp.second;
    for (auto kvp : NTtoNT) delete kvp.second;
    for (auto kvp : originalNTtoNT) delete kvp.second;
}


Parser::Parser(BaseParserEventHandler *_parserEventHandler, string grammar_filename, char _quote){
    next_free_rule_index = START_RULE;
    quote = _quote;
    parser_event_handler = _parserEventHandler;
    parse_tree = NULL;
    word_in_grammar = false;
    grammar_name = "";
    used_eof = false;
    map<string, unsigned long> ruleToNT;
    
    ifstream f(grammar_filename.c_str());
    if (f.good()){
        f.close();
        // interpret the rules and create the structure for parsing
        vector<string> *rules = extract_text_based_rules(grammar_filename, quote);
        vector<string> *tokens = split_string(rules->at(0), ' ', quote);
        grammar_name = tokens->at(1);
        delete tokens;
        
        
        rules->erase(rules->begin());
        ruleToNT.insert(pair<string, unsigned long>(EOF_RULE_NAME, EOF_RULE));
        TtoNT.insert(pair<char, set<unsigned long>*>(EOF_SIGN, new set<unsigned long>()));
        TtoNT.at(EOF_SIGN)->insert(EOF_RULE);
        
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
            for (uint i = 0; i < products->size(); ++i){
                products->at(i) = strip(products->at(i), ' ');
            }
            
            if (ruleToNT.find(rule) == ruleToNT.end()){
                ruleToNT.insert(pair<string, unsigned long>(rule, get_next_free_rule_index()));
            }
            unsigned long new_rule_index = ruleToNT.at(rule);
            
            if (NTtoRule.find(new_rule_index) == NTtoRule.end()){
                NTtoRule.insert(pair<unsigned long, string>(new_rule_index, rule));
            }
            
            
            for (auto product : *products){
                vector<string> non_terminals;
                vector<unsigned long> non_terminal_rules;
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
                            if (ruleToNT.find(non_terminal) == ruleToNT.end()){
                                ruleToNT.insert(pair<string, unsigned long>(non_terminal, get_next_free_rule_index()));
                            }
                            non_terminal_rules.push_back(ruleToNT.at(non_terminal));
                        }
                    }
                }
                else{
                    char c = NTFirst[1];
                    if (TtoNT.find(c) == TtoNT.end()) TtoNT.insert(pair<char, set<unsigned long>*>(c, new set<unsigned long>()));
                    TtoNT.at(c)->insert(new_rule_index);
                }
                
                // more than two rules, insert intermediate rule indexes
                while (non_terminal_rules.size() > 2){
                    unsigned long rule_index_2 = non_terminal_rules.back();
                    non_terminal_rules.pop_back();
                    unsigned long rule_index_1 = non_terminal_rules.back();
                    non_terminal_rules.pop_back();
                    
                    unsigned long key = compute_rule_key(rule_index_1, rule_index_2);
                    unsigned long next_index = get_next_free_rule_index();
                    if (NTtoNT.find(key) == NTtoNT.end()) NTtoNT.insert(pair<unsigned long, set<unsigned long>*>(key, new set<unsigned long>()));
                    NTtoNT.at(key)->insert(next_index);
                    non_terminal_rules.push_back(next_index);
                }
                    
                // two product rules
                if (non_terminal_rules.size() == 2){
                    unsigned long rule_index_2 = non_terminal_rules.at(1);
                    unsigned long rule_index_1 = non_terminal_rules.at(0);
                    unsigned long key = compute_rule_key(rule_index_1, rule_index_2);
                    if (NTtoNT.find(key) == NTtoNT.end()) NTtoNT.insert(pair<unsigned long, set<unsigned long>*>(key, new set<unsigned long>()));
                    NTtoNT.at(key)->insert(new_rule_index);
                }
                
                // only one product rule
                else if (non_terminal_rules.size() == 1){
                    unsigned long rule_index_1 = non_terminal_rules.at(0);
                    if (rule_index_1 == new_rule_index){
                        delete products;
                        delete rules;
                        throw RuntimeException("Error: corrupted token in grammar: rule '" + rule + "' is not allowed to refer soleley to itself.");
                    }
                    
                    if (NTtoNT.find(rule_index_1) == NTtoNT.end()) NTtoNT.insert(pair<unsigned long, set<unsigned long>*>(rule_index_1, new set<unsigned long>()));
                    NTtoNT.at(rule_index_1)->insert(new_rule_index);
                }
            }
            
            delete products;
        }
        delete rules;
        
        // adding all rule names into the event handler
        for (auto rule_name : ruleToNT){
            parser_event_handler->rule_names.insert(rule_name.first);
        }
            
        parser_event_handler->parser = this;
        parser_event_handler->sanity_check();
        
    }
    else{
        throw RuntimeException("Error: file '" + grammar_filename + "' does not exist or can not be opened.");
    }
    
    
    
    
    set<unsigned long> keys;
    for (auto key : TtoNT) keys.insert(key.first);
                                                                   
    for (auto c : keys){
        set<unsigned long> rules;
        for (auto rule : *TtoNT.at(c)) rules.insert(rule);
                                                                   
        TtoNT[c]->clear();
        for (auto rule : rules){
            vector<unsigned long> *backward_rules = collect_one_backwards(rule);
            for (auto p : *backward_rules){
                
                unsigned long key = compute_rule_key(p, rule);
                TtoNT.at(c)->insert(key);
            }
            delete backward_rules;
        }
    }
    
    for (auto kvp : NTtoNT){
        set<unsigned long> *copy_set = new set<unsigned long>();
        for (auto r : *kvp.second) copy_set->insert(r);
        originalNTtoNT.insert({kvp.first, copy_set});
        
    }
    /*
    for (auto kvp : originalNTtoNT){
        if ((unsigned long)kvp.first < 1000){
            cout << (unsigned long)kvp.first << " {";
            for (auto v : *kvp.second){
                cout << (unsigned long)v << ", ";
            }
            cout << "}" << endl;
        }
    }*/
    
    set<unsigned long> keysNT;
    for (auto k : NTtoNT) keysNT.insert(k.first);
    for (auto r : keysNT){
        set<unsigned long> rules;
        for (auto rr : *NTtoNT.at(r)) rules.insert(rr);
                                                                   
        for (auto rule : rules){
            vector<unsigned long> *backward_rules = collect_one_backwards(rule);
            for (auto p : *backward_rules) NTtoNT.at(r)->insert(p);
            delete backward_rules;
        }
    }
    
    
    
}


vector<string>* Parser::extract_text_based_rules(string grammar_filename, char _quote){
    
    
    
    std::ifstream t;
    t.open(grammar_filename);      // open input file
    t.seekg(0, std::ios::end);    // go to the end
    int grammar_length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    char* buffer = new char[grammar_length];    // allocate memory for a buffer of appropriate dimension
    t.read(buffer, grammar_length);       // read the whole file into the buffer
    t.close();
    string grammar = string(buffer);
    delete buffer;
    
    
    
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
        else if (grammar[i] == _quote && ! (i >= 1 && grammar[i - 1] == '\\' && i - 1 != last_escaped_backslash)) match = Quote;
        
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
        throw RuntimeException("Error: corrupted grammar '" + grammar_filename + "', ends either in comment or quote");
    }
    
    grammar = sb.str();
    grammar = replace_all(grammar, "\r\n", "");
    grammar = replace_all(grammar, "\n", "");
    grammar = replace_all(grammar, "\r", "");
    grammar = strip(grammar, ' ');
    
    
    if (grammar[grammar.length() - 1] != RULE_TERMINAL){
        throw RuntimeException("Error: corrupted grammar'" + grammar_filename + "', last rule has no termininating sign, was: '" + string(1, grammar[grammar.length() - 1]) + "'");
    }
    
    vector<string> *rules = split_string(grammar, RULE_TERMINAL, _quote);
    
    if (rules->size() < 1){
        throw RuntimeException("Error: corrupted grammar '" + grammar_filename + "', grammar is empty");
    }
    
    vector<string> *grammar_name_rule = split_string(rules->at(0), ' ', _quote);
    
    if (grammar_name_rule->at(0) != "grammar"){
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



string Parser::replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}



unsigned long Parser::compute_rule_key(unsigned long rule_index_1, unsigned long rule_index_2){
    return (rule_index_1 << SHIFT) | rule_index_2;
}






vector<string>* Parser::split_string(string text, char separator, char _quote){
    bool in_quote = false;
    vector<string> *tokens = new vector<string>();
    stringstream sb;
    char last_char = '\0';
    bool last_escaped_backslash = false;
    
    for (uint i = 0; i < text.length(); ++i){
        char c = text[i];
        bool escaped_backslash = false;
        if (!in_quote){
            if (c == separator){
                string sb_string;
                sb_string = sb.str();
                if (sb_string.length() > 0) tokens->push_back(sb_string);
                sb.str("");
            }
            else{
                if (c == _quote) in_quote = !in_quote;
                sb << c;
            }
        }
        else {
            if (c == '\\' && last_char == '\\' && !last_escaped_backslash){
                escaped_backslash = true;
            }
            else if (c == _quote && !(last_char == '\\' && !last_escaped_backslash)){
                in_quote = !in_quote;
            }
            sb << c;
        }
        
        last_escaped_backslash = escaped_backslash;
        last_char = c;
    }
    
    string sb_string;
    sb_string = sb.str();
    
    if (sb_string.length() > 0){
        tokens->push_back(sb_string);
    }
    if (in_quote){
        delete tokens;
        throw RuntimeException("Error: corrupted token in grammar");
    }
    return tokens;
}


// checking if string is terminal
bool Parser::is_terminal(string product_token, char _quote){
    return product_token[0] == _quote && product_token[product_token.length() - 1] == _quote && product_token.length() > 2;
}




string Parser::de_escape(string text, char _quote){
    // remove the escape chars
    stringstream sb;
    bool last_escape_char = false;
    for (uint i = 0; i < text.length(); ++i){
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
unsigned long Parser::add_terminal(string text){
    vector<unsigned long> terminal_rules;
    for (uint i = 1; i < text.length() - 1; ++i){
        char c = text[i];
        if (TtoNT.find(c) == TtoNT.end()) TtoNT.insert(pair<char, set<unsigned long>*>(c, new set<unsigned long>()));
        unsigned long next_index = get_next_free_rule_index();
        TtoNT.at(c)->insert(next_index);
        terminal_rules.push_back(next_index);
    }
    
    while (terminal_rules.size() > 1){
        unsigned long rule_index_2 = terminal_rules.back();
        terminal_rules.pop_back();
        unsigned long rule_index_1 = terminal_rules.back();
        terminal_rules.pop_back();
        
        unsigned long next_index = get_next_free_rule_index();
        
        unsigned long key = compute_rule_key(rule_index_1, rule_index_2);
        if (NTtoNT.find(key) == NTtoNT.end()) NTtoNT.insert(pair<unsigned long, set<unsigned long>*>(key, new set<unsigned long>()));
        NTtoNT.at(key)->insert(next_index);
        terminal_rules.push_back(next_index);
    }
    return terminal_rules.at(0);
}



// expanding singleton rules, e.g. S -> A, A -> B, B -> C
vector<unsigned long>* Parser::collect_one_backwards(unsigned long rule_index){
    vector<unsigned long> *collection = new vector<unsigned long>();
    collection->push_back(rule_index);
    uint i = 0;
    while (i < collection->size()){
        unsigned long current_index = collection->at(i);
        if (NTtoNT.find(current_index) != NTtoNT.end()){
            for (auto previous_index : *NTtoNT.at(current_index)) collection->push_back(previous_index);
        }
        i += 1;
    }
    
    return collection;
}



vector<unsigned long>* Parser::collect_backwards(unsigned long child_rule_index, unsigned parent_rule_index, int s){
    if (originalNTtoNT.find(child_rule_index) == originalNTtoNT.end()) return NULL;
    
    //cout << child_rule_index << " " << parent_rule_index << " " << s << endl;
    /*
    if (child_rule_index == 3ull) {
        for (auto a : *originalNTtoNT.at(child_rule_index)) cout << a << endl;
        exit(0);
    }*/
    
    for (set<unsigned long>::iterator previous_index = originalNTtoNT.at(child_rule_index)->begin(); previous_index != originalNTtoNT.at(child_rule_index)->end(); ++previous_index){
        if (*previous_index == parent_rule_index) return new vector<unsigned long>;
        
        else if (originalNTtoNT.find(*previous_index) != originalNTtoNT.end()){
            vector<unsigned long>* collection = collect_backwards(*previous_index, parent_rule_index, s + 1);
            if (collection != NULL){
                collection->push_back(*previous_index);
                return collection;
            }
        }
    }
    return NULL;
}
    
    
    
    

void Parser::raise_events(TreeNode *node){
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
void Parser::fill_tree(TreeNode *node, DPNode *dp_node){
    // checking and extending nodes for single rule chains
    unsigned long key = dp_node->left != NULL ? compute_rule_key(dp_node->rule_index_1, dp_node->rule_index_2) : dp_node->rule_index_2;
    
    //cout << "in" << endl;
    vector<unsigned long> *merged_rules = collect_backwards(key, node->rule_index);
    //cout << "in 2" << endl;
    if (merged_rules != NULL){
        for (auto rule_index : *merged_rules){
            node->left = new TreeNode(rule_index, NTtoRule.find(rule_index) != NTtoRule.end());
            node = node->left;
        }
        delete merged_rules;
    }
    //cout << "out" << endl;
    
    if (dp_node->left != NULL) { // None => leaf
        node->left = new TreeNode(dp_node->rule_index_1, NTtoRule.find(dp_node->rule_index_1) != NTtoRule.end());
        node->right = new TreeNode(dp_node->rule_index_2, NTtoRule.find(dp_node->rule_index_2) != NTtoRule.end());
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
bool Parser::parse(string text_to_parse){
    if (used_eof) text_to_parse += string(1, EOF_SIGN);
    
    parse_regular(text_to_parse);
    return word_in_grammar;
}
    
    
    
    
void Parser::parse_regular(string text_to_parse){
    word_in_grammar = false;
    if (parse_tree != NULL) delete parse_tree;
    parse_tree = NULL;
    int n = text_to_parse.length();
    // dp stands for dynamic programming, nothing else
    map<unsigned long, DPNode*> ***dp_table = new map<unsigned long, DPNode*>**[n];
    // Ks is a lookup, which fields in the dp_table are filled
    set<unsigned long> *Ks = new set<unsigned long>[n];
    
    vector<DPNode*> DPs;
    
    
    // init the tables
    for (int i = 0; i < n; ++i){
        dp_table[i] = new map<unsigned long, DPNode*>*[n - i];
        //Ks[i] = new Bitfield(n - 1);
        for (int j = 0; j < n - i; ++j) dp_table[i][j] = new map<unsigned long, DPNode*>();
        
    }
    
    bool requirement_fulfilled = true;
    for (int i = 0; i < n; ++i){
        char c = text_to_parse[i];
        if (TtoNT.find(c) == TtoNT.end()) {
            requirement_fulfilled = false;
            break;
        }
        
        for (auto rule_index : *TtoNT.at(c)){
            unsigned long new_key = rule_index >> SHIFT;
            unsigned old_key = rule_index & MASK;
            DPNode *dp_node = new DPNode(c, old_key, NULL, NULL);
            dp_table[i][0]->insert(pair<unsigned long, DPNode*>(new_key, dp_node));
            // Ks[i]->set_bit(0);
            Ks[i].insert(0);
        }
    }
    
    if (requirement_fulfilled){
        for (int i = 1; i < n; ++i){
            int im1 = i - 1;
            for (int j = 0; j < n - i; ++j){
                map<unsigned long, DPNode*>** D = dp_table[j];
                map<unsigned long, DPNode*>* Di = D[i];
                int jp1 = j + 1;
                
                // Ks[j]->init();
                //int k;
                //while ((k = Ks[j]->get_bit_positions()) != -1){
                for (auto k : Ks[j]){
                    if (k >= (unsigned int)i) break;
                    //if (Ks[jp1 + k]->is_not_set(im1 - k)) continue;
                    if (Ks[jp1 + k].find(im1 - k) == Ks[jp1 + k].end()) continue;
                
                    for (auto index_pair_1 : *D[k]){
                        for (auto index_pair_2 : *dp_table[jp1 + k][im1 - k]){
                            unsigned long key = compute_rule_key(index_pair_1.first, index_pair_2.first);
                            
                            if (NTtoNT.find(key) == NTtoNT.end()) continue;
                            
                            DPNode *content = new DPNode(index_pair_1.first, index_pair_2.first, index_pair_1.second, index_pair_2.second);
                            DPs.push_back(content);
                            //Ks[j]->set_bit(i);
                            Ks[j].insert(i);
                            for (auto rule_index : *NTtoNT.at(key)){
                                Di->insert(pair<unsigned long, DPNode*>(rule_index, content));
                            }
                        }
                    }
                }
            }
        }
        for (int i = n - 1; i > 0; --i){
            if (dp_table[0][i]->find(START_RULE) != dp_table[0][i]->end()){
                word_in_grammar = true;
                parse_tree = new TreeNode(START_RULE, NTtoRule.find(START_RULE) != NTtoRule.end());
                fill_tree(parse_tree, dp_table[0][i]->at(START_RULE));
                raise_events(parse_tree);
                break;
            }
        }
    }
    
    // delete tables
    for (auto dp_node : DPs) delete dp_node;
    for (int i = 0; i < n; ++i){
        for (int j = 0; j < n - i; ++j){
            delete dp_table[i][j];
        }
        delete[] dp_table[i];
    }
    delete[] dp_table;
    delete[] Ks;
    
}
