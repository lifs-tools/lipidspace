#include "lipidspace/AssistanceFunctions.h"


SingleListWidget::SingleListWidget(QWidget *parent) : QListWidget(parent) {
    field_name = "";
    num = 1;
}

void SingleListWidget::addFieldName(string _field_name){
    field_name = _field_name;
}

void SingleListWidget::setNum(int _num){
    num = _num;
}

void SingleListWidget::dropEvent(QDropEvent *event){
    if (count() + ((QListWidget*)event->source())->selectedItems().size() > num){
        oneItemViolation(field_name, num);
    }
    else {
        QListWidget::dropEvent(event);
    }
}


DendrogramNode::DendrogramNode(int index, map<string, set<string>> *feature_values, Table *lipidome){
    indexes.insert(index);
    left_child = 0;
    right_child = 0;
    distance = 0;
    x_left = 0;
    x_right = 0;
    y = 0;
    
    
    // initialize empty feature count table
    for (auto kv : *feature_values){
        feature_count.insert({kv.first, map<string, int>()});
        for (auto feature_value : kv.second){
            feature_count[kv.first].insert({feature_value, 0});
        }
    }
    
    for (auto kv : lipidome->features){
        feature_count[kv.first][kv.second] = 1;
    }
}

DendrogramNode::~DendrogramNode(){
    if (left_child) delete left_child;
    if (right_child) delete right_child;
}
    
DendrogramNode::DendrogramNode(DendrogramNode* n1, DendrogramNode* n2, double d){
    left_child = n1;
    right_child = n2;
    for (auto i : n1->indexes) indexes.insert(i);
    for (auto i : n2->indexes) indexes.insert(i);
    distance = d;
}


double* DendrogramNode::execute(int cnt, Array* points, vector<int>* sorted_ticks){
    if (left_child == 0){
        sorted_ticks->push_back(*indexes.begin());
        return new double[3]{(double)cnt, 0, (double)cnt + 1};
    }

    double* left_result = left_child->execute(cnt, points, sorted_ticks);
    x_left = left_result[0];
    double yl = left_result[1];
    cnt = left_result[2];
    delete []left_result;
    
    double* right_result = right_child->execute(cnt, points, sorted_ticks);
    x_right = right_result[0];
    double yr = right_result[1];
    cnt = right_result[2];
    delete []right_result;
    
    y = distance;
    
    points->push_back(x_left);
    points->push_back(yl);
    points->push_back(x_left);
    points->push_back(y);
    
    points->push_back(x_right);
    points->push_back(yr);
    points->push_back(x_right);
    points->push_back(y);
    
    points->push_back(x_left);
    points->push_back(y);
    points->push_back(x_right);
    points->push_back(y);
    
    // count features
    for (auto kv : left_child->feature_count){
        feature_count.insert({kv.first, map<string, int>()});
        for (auto kv2 : kv.second){
            feature_count[kv.first].insert({kv2.first, kv2.second + right_child->feature_count[kv.first][kv2.first]});
        }
    }
    
    
    return new double[3]{(x_left + x_right) / 2, y, (double)cnt};
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
