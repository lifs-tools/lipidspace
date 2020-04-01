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
    class iter;
    
public:
    unsigned long *field;
    unsigned long *superfield;
    uint32_t field_len;
    uint32_t superfield_len;
    uint32_t num_size;
    uint32_t length;
    
    iter begin();
    iter end();
    
    uint32_t size() const;
    
    
    Bitfield(uint32_t length);
    ~Bitfield();
    void insert(uint32_t pos);
    bool find(uint32_t pos);
    void init();
    int next(int pos = -1);
    void print_bitfield(unsigned long l);
    
private:
    class iter : public std::iterator<std::output_iterator_tag, int>{
        public:
            explicit iter(Bitfield& _bitfield, uint32_t index = 0);
            int operator*();
            iter & operator++();
            iter & operator++(int);
            bool operator!=(const iter &) const;
            uint32_t num_index;
            int last_position;
            Bitfield &bitfield;
            bool get_next;
    };
};


        

template <class T>
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
    static const string EOF_RULE_NAME;
    
    
    unsigned long next_free_rule_index;
    map<char, set<unsigned long>> TtoNT;
    map<char, unsigned long> originalTtoNT;
    map<unsigned long, set<unsigned long>> NTtoNT;
    map<unsigned long, string> NTtoRule;
    map<unsigned long, vector<unsigned long>*> substitution;
    //vector<set<unsigned long>> left_pair;
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
    unsigned long get_next_free_rule_index();
    vector<string>* extract_text_based_rules(string grammar_filename, char _quote = DEFAULT_QUOTE);
    vector<unsigned long>* top_nodes(unsigned long rule_index);
    static unsigned long compute_rule_key(unsigned long rule_index_1, unsigned long rule_index_2);
    bool is_terminal(string product_token, char _quote);
    static string de_escape(string text, char _quote);
    unsigned long add_terminal(string text);
    vector<unsigned long>* collect_one_backwards(unsigned long rule_index);
    vector<unsigned long>* collect_backwards(unsigned long child_rule_index, unsigned parent_rule_index);
    void raise_events(TreeNode *node);
    void fill_tree(TreeNode *node, DPNode *dp_node);
    T parse(string text_to_parse);
    void parse_regular(string text_to_parse);
};




#include "cppgoslin/parser/Parser_impl.h"
#endif /* PARSER_H */


