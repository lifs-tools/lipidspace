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
        

Bitfield::Bitfield(uint length){
    field_len = 1 + ((length + 1) >> 6);
    superfield_len = 1 + ((field_len + 1) >> 6);
    field = new unsigned long[field_len];
    superfield = new unsigned long[superfield_len];
    
    for (uint i = 0; i < field_len; ++i) field[i] = 0ull;
    for (uint i = 0; i < superfield_len; ++i) superfield[i] = 0ull;
    do_init = true;
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


int Bitfield::get_bit_positions(){
    if (do_init){
        spre = 0;
        si = 0;
        sv = superfield[si];
        fi = 0;
        
        unsigned long sv1 = (unsigned long)(sv & -sv);
        pos = spre + __builtin_ctzll(sv1);
        v = field[pos];
        do_init = false;
    }
    
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
                if (pos < field_len) v = field[pos];
            }
        }
        
        spre += 64;
        if (si < superfield_len){
            ++si;
            if (si < superfield_len){
                sv = superfield[si];
                unsigned long sv1 = (unsigned long)(sv & -sv);
                pos = spre + __builtin_ctzll(sv1);
                if (pos < field_len) v = field[pos];
            }
        }
    }
    while (si < superfield_len);
    
    do_init = true;
    return -1;
}


