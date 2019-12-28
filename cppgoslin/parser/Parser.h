#ifndef PARSER_H
#define PARSER_H

/*
#include "cppgoslin/parser/GoslinParserEventHandler.h"
#include "cppgoslin/parser/GoslinFragmentParserEventHandler.h"
#include "cppgoslin/parser/LipidMapsParserEventHandler.h"
*/
#include "BaseParserEventHandler.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

enum Content {NoContext, InLineComment, InLongComment, InQuote};
enum MatchWords {NoMatch, LineCommentStart, LineCommentEnd, LongCommentStart, LongCommentEnd, Quote};

using namespace std;

class Parser;
    
// DP stands for dynamic programming
class DPNode {
public:    
    uint rule_index_1;
    uint rule_index_2;
    DPNode *left = NULL;
    DPNode *right = NULL;
    
    DPNode(uint _rule1, uint _rule2, DPNode *_left, DPNode *_right);
    ~DPNode();
};
        

class TreeNode {
public:
    uint rule_index;
    TreeNode *left;
    TreeNode *right;
    char terminal;
    bool fire_event;
    static const char EOF_SIGN = '\0';
    
    TreeNode(uint _rule, bool _fire_event);
    ~TreeNode();
    string get_text();
};
        
        

// this class is dedicated to have an efficient sorted set class storing
// values within 0..n-1 and fast sequencial iterator
class Bitfield {
public:
    unsigned long *field;
    unsigned long *superfield;
    unsigned int spre;
    unsigned long sv;
    unsigned int si;
    unsigned long v;
    unsigned int fi;
    unsigned int pos;
    unsigned int field_len;
    unsigned int superfield_len;
    
    
    
    
    Bitfield(uint length);
    ~Bitfield();
    void set_bit(uint pos);
    bool is_set(uint pos);
    bool is_not_set(uint pos);
    void init();
    int get_bit_positions();
    void print_bitfield(unsigned long l);
};
        
        

class Parser{
public:
    static const int SHIFT = 32;
    static const unsigned long MASK = (1 << SHIFT) - 1;
    static const char RULE_ASSIGNMENT = ':';
    static const RULE_SEPARATOR = '|';
    static const RULE_TERMINAL = ';';
    static const string EOF_RULE_NAME = "EOF";
    static const char EOF_SIGN = (char)1;
    static const unsigned long EOF_RULE = 1;
    static const unsigned long START_RULE = 2;
    static const char DEFAULT_QUOTE = '\'';
    static const unsigned long next_free_rule_index;
    
    
    unsigned long next_free_rule_index;
    map<char, set<unsigned long>*> TtoNT;
    map<unsigned long, set<unsigned long>*> NTtoNT;
    map<unsigned long, string> NTtoRule;
    map<char, set<unsigned long>*> originalNTtoNT;
    char quote = _quote;
    BaseParserEventHandler *parser_event_handler;
    TreeNode *parse_tree;
    bool word_in_grammar;
    string grammar_name;
    bool used_eof;
    
    
    
    Parser(BaseParserEventHandler *_parserEventHandler, string grammar_filename, _quote = DEFAULT_QUOTE);
    ~Parser();
    void get_next_free_rule_index();
    vector<string>* extract_text_based_rules(string grammar_filename, char _quote = DEFAULT_QUOTE);
    static unsigned long compute_rule_key(rule_index_1, rule_index_2);
    static vector<string>* split_string(string text, char separator, char _quote = DEFAULT_QUOTE);
    bool is_terminal(string product_token, char _quote);
    static string de_escape(string text, char _quote);
    unsigned long add_terminal(string text);
    vector<unsigned long>* collect_one_backwards(unsigned long rule_index);
    vector<unsigned long>* collect_backwards(unsigned long child_rule_index, unsigned parent_rule_index);
    void raise_events(TreeNode *node);
    void fill_tree(TreeNode *node, DPNode *dp_node);
    bool parse(string text_to_parse);
    static string strip(string s, char c);
    
/*
class GoslinParser(Parser):
    def __init__(self):
        self.event_handler = GoslinParserEventHandler()
        dir_name = path.dirname(pygoslin.__file__)
        super().__init__(self.event_handler, dir_name + "/data/goslin/Goslin.g4", Parser.DEFAULT_QUOTE)
        
        
        
class GoslinFragmentParser(Parser):
    def __init__(self):
        self.event_handler = GoslinFragmentParserEventHandler()
        dir_name = path.dirname(pygoslin.__file__)
        super().__init__(self.event_handler, dir_name + "/data/goslin/GoslinFragments.g4", Parser.DEFAULT_QUOTE)
        
        
class LipidMapsParser(Parser):
    def __init__(self):
        self.event_handler = LipidMapsParserEventHandler()
        dir_name = path.dirname(pygoslin.__file__)
        super().__init__(self.event_handler, dir_name + "/data/goslin/LipidMaps.g4", Parser.DEFAULT_QUOTE)
        
        
class LipidParser:
    def __init__(self):
        self.parser = None
        self.lipid = None
        self.event_handler = None
        
        self.parser_list = [GoslinParser(), GoslinFragmentParser(), LipidMapsParser()]
        
    def parse(self, lipid_name):
        self.parser = None
        self.lipid = None
        self.event_handler = None
        
        for parser in self.parser_list:
            parser.parse(lipid_name)
            if parser.word_in_grammar:
                self.parser = parser
                self.event_handler = parser.event_handler
                self.lipid = self.event_handler.lipid
                break
         
*/

#endif /* PARSER_H */
