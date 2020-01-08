#include "cppgoslin/parser/Parser.h"



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
       
       
       
       

Bitfield::Bitfield(uint _length){
    length = _length;
    field_len = 1 + ((length + 1) >> 6);
    superfield_len = 1 + ((field_len + 1) >> 6);
    field = new unsigned long[field_len];
    superfield = new unsigned long[superfield_len];
    num_size = 0;
    
    
    for (uint i = 0; i < field_len; ++i) field[i] = 0ull;
    for (uint i = 0; i < superfield_len; ++i) superfield[i] = 0ull;
}



Bitfield::~Bitfield(){
    delete []field;
    delete []superfield;
}




void Bitfield::insert(uint pos){
    if (!find(pos)){
        field[pos >> 6] |= (unsigned long)(1ull << (pos & 63));
        superfield[pos >> 12] |= (unsigned long)(1ull << ((pos >> 6) & 63));
        ++num_size;
    }
}




bool Bitfield::find(uint pos){
    return ((field[pos >> 6] >> (pos & 63)) & 1ull) == 1ull;
}



/*
void Bitfield::print_bitfield(unsigned long l){
    for (int i = 63; i >= 0; --i){
        cout << ((l >> i) & 1);
    } cout << endl;
}
*/



int Bitfield::next(uint pos){
    if (pos >= length) return -1;
    
    uint field_pos = pos >> 6;
    uint superfield_pos = pos >> 12;
    
    unsigned long field_bits = field[field_pos] & (~((1ull << (pos & 63)) - 1ull));
    unsigned long superfield_bits = superfield[superfield_pos] & (~((1ull << ((pos >> 6) & 63)) - 1ull));
    
    do {
        while (superfield_bits){
            if (field_bits){
                return (field_pos << 6) + __builtin_ctzll(field_bits & -field_bits);
            }
            
            
            superfield_bits &= superfield_bits - 1ull;
            field_pos = (superfield_pos << 6) + __builtin_ctzll(superfield_bits & -superfield_bits);
            if (field_pos < field_len) field_bits = field[field_pos];
        }
        
        
        
        if (++superfield_pos < superfield_len){
            superfield_bits = superfield[superfield_pos];
            field_pos = (superfield_pos << 6) + __builtin_ctzll(superfield_bits & -superfield_bits);
            if (field_pos < field_len) field_bits = field[field_pos];
        }
    } while (superfield_pos < superfield_len);
    
    return -1;
}





Bitfield::iter::iter(Bitfield & _bitfield, uint index) : bitfield(_bitfield), num_index(index), last_position(0) {
    last_position = bitfield.next(last_position);
}




int Bitfield::iter::operator*() const {
    return last_position;
}




Bitfield::iter & Bitfield::iter::operator++() {
    num_index++;
    last_position = bitfield.next(last_position + 1);
    return *this;
}




Bitfield::iter & Bitfield::iter::operator++(int i) {
    return ++(*this);
}




bool Bitfield::iter::operator!=(const iter & rhs) const {
    return num_index != rhs.num_index;
}




volatile uint Bitfield::size() const {
    return num_size;
}




Bitfield::iter Bitfield::begin() {
    return Bitfield::iter(*this, 0);
}





Bitfield::iter Bitfield::end(){
    return Bitfield::iter(*this, size());
}



string strip(string s, char c){
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



vector<string>* split_string(string text, char separator, char _quote){
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