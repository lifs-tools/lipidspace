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


#ifndef PARSER_H
#define PARSER_H


#include "cppgoslin/parser/BaseParserEventHandler.h"
#include "cppgoslin/domain/LipidAdduct.h"
#include "cppgoslin/domain/StringFunctions.h"
#include "cppgoslin/parser/KnownGrammars.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

enum Content {NoContext, InLineComment, InLongComment, InQuote};
enum MatchWords {NoMatch, LineCommentStart, LineCommentEnd, LongCommentStart, LongCommentEnd, Quote};

using namespace std;

template<class T>
class BaseParserEventHandler;


class GrammarString : public string {
public:
    GrammarString(string s) : string(s){}
        
};



    
// DP stands for dynamic programming
class DPNode {
public:    
    uint64_t rule_index_1;
    uint64_t rule_index_2;
    DPNode *left = NULL;
    DPNode *right = NULL;
    
    DPNode(uint64_t _rule1, uint64_t _rule2, DPNode *_left, DPNode *_right);
};
        

class TreeNode {
public:
    uint64_t rule_index;
    TreeNode *left;
    TreeNode *right;
    char terminal;
    bool fire_event;
    static const char EOF_SIGN = '\0';
    
    TreeNode(uint64_t _rule, bool _fire_event);
    ~TreeNode();
    string get_text();
};
   

        

// this class is dedicated to have an efficient sorted set class storing
// values within 0..n-1 and fast sequencial iterator
class Bitfield {
    class iter;
    
public:
    uint64_t *field;
    uint64_t *superfield;
    uint64_t field_len;
    uint64_t superfield_len;
    uint64_t num_size;
    uint64_t length;
    
    iter begin();
    iter end();
    
    uint64_t size() const;
    
    
    Bitfield(uint64_t length);
    ~Bitfield();
    void insert(uint64_t pos);
    bool find(uint64_t pos);
    void init();
    int next(int pos = -1);
    void print_bitfield(uint64_t l);
    
private:
    class iter : public std::iterator<std::output_iterator_tag, int>{
        public:
            explicit iter(Bitfield& _bitfield, uint64_t index = 0);
            int operator*();
            iter & operator++();
            iter & operator++(int);
            bool operator!=(const iter &) const;
            uint64_t num_index;
            int last_position;
            Bitfield &bitfield;
            bool get_next;
    };
};


        

template <class T>
class Parser {
public:
    static const uint32_t SHIFT;
    static const uint64_t MASK;
    static const char RULE_ASSIGNMENT;
    static const char RULE_SEPARATOR;
    static const char RULE_TERMINAL;
    static const char EOF_SIGN;
    static const uint64_t EOF_RULE;
    static const uint64_t START_RULE;
    static const string EOF_RULE_NAME;
    
    
    uint64_t next_free_rule_index;
    map<char, set<uint64_t>> TtoNT;
    map<char, uint64_t> originalTtoNT;
    map<uint64_t, set<uint64_t>> NTtoNT;
    map<uint64_t, string> NTtoRule;
    map<uint64_t, vector<uint64_t>*> substitution;
    //vector<set<uint64_t>> left_pair;
    vector<Bitfield*> right_pair;
    int avg_pair;
    char quote;
    BaseParserEventHandler<T> *parser_event_handler;
    bool word_in_grammar;
    string grammar_name;
    bool used_eof;
    
    
    
    Parser(BaseParserEventHandler<T> *_parserEventHandler, string grammar_filename, char _quote = DEFAULT_QUOTE);
    Parser(BaseParserEventHandler<T> *_parserEventHandler, GrammarString grammar_string, char _quote = DEFAULT_QUOTE);
    void read_grammar(string grammar);
    virtual ~Parser();
    uint64_t get_next_free_rule_index();
    vector<string>* extract_text_based_rules(string grammar_filename, char _quote = DEFAULT_QUOTE);
    vector<uint64_t>* top_nodes(uint64_t rule_index);
    static uint64_t compute_rule_key(uint64_t rule_index_1, uint64_t rule_index_2);
    bool is_terminal(string product_token, char _quote);
    static string de_escape(string text, char _quote);
    uint64_t add_terminal(string text);
    vector<uint64_t>* collect_one_backwards(uint64_t rule_index);
    vector<uint64_t>* collect_backwards(uint64_t child_rule_index, unsigned parent_rule_index);
    void raise_events(TreeNode *node);
    void fill_tree(TreeNode *node, DPNode *dp_node);
    T parse(string text_to_parse);
    void parse_regular(string text_to_parse);
};




#include "cppgoslin/parser/Parser_impl.h"
#endif /* PARSER_H */


