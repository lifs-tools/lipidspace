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

/*
Algorithm adapted from: arXiv:2102.08037
Thomas Viehmann
Numerically more stable computation of the p-values for the two-sample Kolmogorov-Smirnov test
*/
double KS_pvalue(vector<double> &sample1, vector<double> &sample2){
    double d = 0, cdf1 = 0, cdf2 = 0;
    int ptr1 = 0, ptr2 = 0, m = sample1.size(), n = sample2.size();
    sort (sample1.begin(), sample1.end());
    sort (sample2.begin(), sample2.end());
    double inv_m = 1. / (double)m;
    double inv_n = 1. / (double)n;
    
    while (ptr1 < m && ptr2 < n){
        if (sample1[ptr1] <= sample2[ptr2]){
            cdf1 += inv_m;
            ptr1 += 1;
        }
        else {
            cdf2 += inv_n;
            ptr2 += 1;
        }
        d = max(d, fabs(cdf1 - cdf2));
    }
    
    int size = 2 * m * d + 2;
    double *lastrow = new double[size];
    double *row = new double[size];
    for (int i = 0; i < size; ++i){
        lastrow[i] = 0;
        row[i] = 0;
    }
    int last_start_j = 0;
    int start_j = 0;
    for (int i = 0; i < n + 1; ++i){
        start_j = max((int)(m * ((double)i / n + d )) + 1 - size, 0);
        swap(lastrow, row);
        double val = 0;
        for (int jj = 0; jj < size; ++jj){
            int j = jj + start_j;
            double dist = (double)i / (double)n - (double)j / (double)m;
            if (dist > d || dist < -d) val = 1;
            else if (i == 0 || j == 0) val = 0;
            else if (jj + start_j - last_start_j >= size) val = (double)(i + val * j) / (double)(i + j);
            else  val = (lastrow[jj + start_j - last_start_j] * i + val * j) / (double)(i + j);
            row[jj] = val;
        }
        last_start_j = start_j;
    }
    double p_val = row[m - start_j];
    
    
    delete []lastrow;
    delete []row;
    return p_val;
}


bool sort_order_one (pair<double, int> i, pair<double, int> j) { return (i.first < j.first); }

void BH_fdr(vector<double> &data){
    if (data.size() < 2) return;
        
    // sort p_values and store order for back ordering
    vector<pair<double, int>> sorted;
    for (int i = 0; i < (int)data.size(); ++i){
        sorted.push_back({data[i], i});
    }
    sort (sorted.begin(), sorted.end(), sort_order_one);
    
    // perform Benjamini-Hochberg correction
    double n = data.size();
    for (int i = n - 2; i >= 0; --i){
        sorted[i].first = min(sorted[i].first * n / (double)(i + 1), sorted[i + 1].first);
    }
    
    // sort corrected values back into original data vector
    for (auto p : sorted) data[p.second] = p.first;
}


SpeciesItem::SpeciesItem(QString name, LipidAdduct *l, QListWidget* parent) : QListWidgetItem(name, parent) {
    species = l;
    setCheckState(Qt::Checked);
    is_checked = true;
}
