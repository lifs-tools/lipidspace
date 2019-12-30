#ifndef PARSER_H
#define PARSER_H

/*
#include "cppgoslin/parser/GoslinParserEventHandler.h"
#include "cppgoslin/parser/GoslinFragmentParserEventHandler.h"
#include "cppgoslin/parser/LipidMapsParserEventHandler.h"
*/
#include "cppgoslin/parser/BaseParserEventHandler.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

enum Content {NoContext, InLineComment, InLongComment, InQuote};
enum MatchWords {NoMatch, LineCommentStart, LineCommentEnd, LongCommentStart, LongCommentEnd, Quote};

using namespace std;

class BaseParserEventHandler;

    
// DP stands for dynamic programming
class DPNode {
public:    
    unsigned long rule_index_1;
    unsigned long rule_index_2;
    DPNode *left = NULL;
    DPNode *right = NULL;
    
    DPNode(unsigned long _rule1, unsigned long _rule2, DPNode *_left, DPNode *_right);
};
        

class TreeNode {
public:
    unsigned long rule_index;
    TreeNode *left;
    TreeNode *right;
    char terminal;
    bool fire_event;
    static const char EOF_SIGN = '\0';
    
    TreeNode(unsigned long _rule, bool _fire_event);
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
    bool do_init;
    
    
    
    
    Bitfield(uint length);
    ~Bitfield();
    void set_bit(uint pos);
    bool is_set(uint pos);
    bool is_not_set(uint pos);
    void init();
    int get_bit_positions();
    void print_bitfield(unsigned long l);
};
        
    
static const string EOF_RULE_NAME = "EOF";

class Parser {
public:
    static const int SHIFT;
    static const unsigned long MASK;
    static const char RULE_ASSIGNMENT;
    static const char RULE_SEPARATOR;
    static const char RULE_TERMINAL;
    static const char EOF_SIGN;
    static const unsigned long EOF_RULE;
    static const unsigned long START_RULE;
    static const char DEFAULT_QUOTE;
    
    
    unsigned long next_free_rule_index;
    map<char, set<unsigned long>*> TtoNT;
    map<unsigned long, set<unsigned long>*> NTtoNT;
    map<unsigned long, string> NTtoRule;
    map<unsigned long, set<unsigned long>*> originalNTtoNT;
    char quote;
    BaseParserEventHandler *parser_event_handler;
    TreeNode *parse_tree;
    bool word_in_grammar;
    string grammar_name;
    bool used_eof;
    
    
    
    Parser(BaseParserEventHandler *_parserEventHandler, string grammar_filename, char _quote = DEFAULT_QUOTE);
    ~Parser();
    unsigned long get_next_free_rule_index();
    vector<string>* extract_text_based_rules(string grammar_filename, char _quote = DEFAULT_QUOTE);
    static unsigned long compute_rule_key(unsigned long rule_index_1, unsigned long rule_index_2);
    static vector<string>* split_string(string text, char separator, char _quote = DEFAULT_QUOTE);
    bool is_terminal(string product_token, char _quote);
    static string de_escape(string text, char _quote);
    unsigned long add_terminal(string text);
    vector<unsigned long>* collect_one_backwards(unsigned long rule_index);
    vector<unsigned long>* collect_backwards(unsigned long child_rule_index, unsigned parent_rule_index, int s = 0);
    void raise_events(TreeNode *node);
    void fill_tree(TreeNode *node, DPNode *dp_node);
    bool parse(string text_to_parse);
    void parse_regular(string text_to_parse);
    static string strip(string s, char c);
    static string replace_all(std::string str, const std::string& from, const std::string& to);
};
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
