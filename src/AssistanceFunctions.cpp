#include "lipidspace/AssistanceFunctions.h"

Node::Node(int index, string _name){
    indexes.insert(index);
    name = _name;
    left_child = 0;
    right_child = 0;
    distance = 0;
}

Node::~Node(){
    if (left_child) delete left_child;
    if (right_child) delete right_child;
}
    
Node::Node(Node* n1, Node* n2, double d){
    left_child = n1;
    right_child = n2;
    for (auto i : n1->indexes) indexes.insert(i);
    for (auto i : n2->indexes) indexes.insert(i);
    distance = d;
}

Progress::Progress(){
    current_progress = 0;
    max_progress = 0;
    stop_progress = false;
    connected = false;
    step_size = 0;
}


void Progress::increment(){
    current_progress += 1;
    set_current(current_progress);
}


void Progress::set(int cp){
    current_progress = cp;
    set_current(current_progress);
}

void Progress::interrupt(){
    stop_progress = true;
}


void Progress::prepare_steps(int steps){
    step_size = ceil((double)(max_progress - current_progress) / (double)steps);
}

void Progress::set_step(){
    current_progress += step_size;
    set_current(current_progress);
}
