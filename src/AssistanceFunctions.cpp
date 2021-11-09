#include "lipidspace/AssistanceFunctions.h"

Node::Node(int index){
    indexes.insert(index);
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


double* Node::execute(int cnt, Array* points, vector<int>* sorted_ticks){
    if (left_child == 0){
        sorted_ticks->push_back(*indexes.begin());
        return new double[3]{(double)cnt, 0, (double)cnt + 1};
    }

    double* left_result = left_child->execute(cnt, points, sorted_ticks);
    double xl = left_result[0];
    double yl = left_result[1];
    cnt = left_result[2];
    delete []left_result;
    
    double* right_result = right_child->execute(cnt, points, sorted_ticks);
    double xr = right_result[0];
    double yr = right_result[1];
    cnt = right_result[2];
    delete []right_result;
    
    double yn = distance;
    
    points->push_back(xl);
    points->push_back(yl);
    points->push_back(xl);
    points->push_back(yn);
    
    points->push_back(xr);
    points->push_back(yr);
    points->push_back(xr);
    points->push_back(yn);
    
    points->push_back(xl);
    points->push_back(yn);
    points->push_back(xr);
    points->push_back(yn);
    
    return new double[3]{(xl + xr) / 2, yn, (double)cnt};
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
