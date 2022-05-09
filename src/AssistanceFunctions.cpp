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



FileTableHandler::FileTableHandler(string file_name, string sheet_name){
    // csv import
    if (sheet_name.length() == 0){
        
        ifstream infile(file_name);
        if (!infile.good()){
            throw LipidSpaceException("Error: file '" + file_name + "' could not be found.", FileUnreadable);
        }
        int line_num = 0;
        string line;
        while (getline(infile, line)){
            if (line.length() == 0) continue;
            
            vector<string>* tokens = split_string(line, ',', '"', true);
            if (line_num++ == 0){
                for (int i = 0; i < (int)tokens->size(); ++i) headers.push_back(goslin::strip(tokens->at(i), '"'));
            }
            else {
                if (tokens->size() != headers.size()){
                    delete tokens;
                    throw LipidSpaceException("Error: file '" + file_name + "' has a different number of cells in line " + std::to_string(line_num) + " than in the header line.", ColumnNumMismatch);
                }
                rows.push_back(vector<string>());
                vector<string> &row = rows.back();
                for (int i = 0; i < (int)tokens->size(); ++i) row.push_back(goslin::strip(tokens->at(i), '"'));
            }
            
            delete tokens;
        }
    }
    else {
        XLDocument doc(file_name.c_str());

        auto wks = doc.workbook().worksheet(sheet_name.c_str());
        
        int line_num = 0;
        for (auto& wks_row : wks.rows()) {
            if (line_num++ == 0){
                for (auto cell : wks_row.cells()){
                    string value = "";
                    switch(cell.value().type()){
                        case OpenXLSX::XLValueType::Boolean:
                            value = (cell.value().get<bool>()) ? "true" : "false";
                            break;
                        case OpenXLSX::XLValueType::Integer:
                        case OpenXLSX::XLValueType::Float:
                            value = std::to_string(cell.value().get<double>());
                            break;
                        case OpenXLSX::XLValueType::String:
                            value = cell.value().get<string>();
                            break;
                        default: break;
                    }
                    headers.push_back(value);
                }
            }
            else {
                rows.push_back(vector<string>());
                vector<string> &row = rows.back();
                int empty_cells = 0;
                for (auto cell : wks_row.cells()){
                    string value = "";
                    switch(cell.value().type()){
                        case OpenXLSX::XLValueType::Boolean:
                            value = (cell.value().get<bool>()) ? "true" : "false";
                            break;
                        case OpenXLSX::XLValueType::Integer:
                            value = std::to_string(cell.value().get<int>());
                            break;
                        case OpenXLSX::XLValueType::Float:
                            value = std::to_string(cell.value().get<double>());
                            break;
                        case OpenXLSX::XLValueType::String:
                            value = cell.value().get<string>();
                            break;
                        default: break;
                    }
                    row.push_back(value);
                    empty_cells += (value == "");
                }
                if (row.size() == 0 || empty_cells == wks_row.cells().size()){
                    rows.pop_back();
                    break;
                }
                
                if (row.size() > headers.size()){
                    throw LipidSpaceException("Error: file '" + file_name + "' has a different number of cells (" + std::to_string(row.size()) + ") in line " + std::to_string(line_num) + " than in the header line (" + std::to_string(headers.size()) + ").", ColumnNumMismatch);
                }
                else if (row.size() < headers.size()){
                    row.resize(headers.size(), "");
                }
            }
        }
    }
}




DendrogramNode::DendrogramNode(int index, map<string, FeatureSet> *feature_values, Lipidome *lipidome){
    indexes.insert(index);
    order = -1;
    left_child = 0;
    right_child = 0;
    distance = 0;
    x_left = 0;
    x_right = 0;
    y = 0;
    
    
    // initialize empty feature count table
    for (auto kv : *feature_values){
        if (kv.second.feature_type == NominalFeature){
            feature_count_nominal.insert({kv.first, map<string, int>()});
            for (auto kv_nom : kv.second.nominal_values){
                feature_count_nominal[kv.first].insert({kv_nom.first, 0});
            }
        }
        else {
            feature_numerical.insert({kv.first, vector<double>()});
        }
    }
    
    for (auto kv : lipidome->features){
        if (kv.second.feature_type == NominalFeature){
            feature_count_nominal[kv.first][kv.second.nominal_value] = 1;
        }
        else {
            feature_numerical[kv.first].push_back(kv.second.numerical_value);
        }
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
        order = cnt;
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
    for (auto kv : left_child->feature_count_nominal){
        feature_count_nominal.insert({kv.first, map<string, int>()});
        for (auto kv2 : kv.second){
            feature_count_nominal[kv.first].insert({kv2.first, kv2.second + right_child->feature_count_nominal[kv.first][kv2.first]});
        }
    }
    
    for (auto kv : left_child->feature_numerical){
        // find intersection points for numerical features
        vector<double> &set1 = kv.second;
        vector<double> &set2 = right_child->feature_numerical[kv.first];
        
        double pos_max = 0, d = 0;
        ks_separation_value(set1, set2, d, pos_max);
        /*
        int num1 = set1.size();
        int num2 = set2.size();
        sort(set1.begin(), set1.end());
        sort(set2.begin(), set2.end());
        double inv_m = 1. / num1, inv_n = 1. / num2;
        double ptr1 = 0, ptr2 = 0;
        double cdf1 = 0, cdf2 =0;
        while ((ptr1 < num1) && (ptr2 < num2)){
            if (set1[ptr1] <= set2[ptr2]){
                cdf1 += inv_m;
                if (d < fabs(cdf1 - cdf2)){
                    d = fabs(cdf1 - cdf2);
                    pos_max = set1[ptr1];
                }
                ptr1 += 1;
            }
            else {
                cdf2 += inv_n;
                if (d < fabs(cdf1 - cdf2)){
                    d = fabs(cdf1 - cdf2);
                    pos_max = set2[ptr2];
                }
                ptr2 += 1;
            }
        }*/
        feature_numerical_thresholds.insert({kv.first, pos_max});
        
        feature_numerical.insert({kv.first, vector<double>()});
        for(double val : kv.second){
            feature_numerical[kv.first].push_back(val);
        }
        for(double val : right_child->feature_numerical[kv.first]){
            feature_numerical[kv.first].push_back(val);
        }
    }
    return new double[3]{(x_left + x_right) / 2, y, (double)cnt};
}



void ks_separation_value(vector<double> &a, vector<double> &b, double &d, double &pos_max){
    d = 0;
    pos_max = 0;
    int num1 = a.size();
    int num2 = b.size();
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    double inv_m = 1. / num1, inv_n = 1. / num2;
    double ptr1 = 0, ptr2 = 0;
    double cdf1 = 0, cdf2 = 0;
    while ((ptr1 < num1) && (ptr2 < num2)){
        if (a[ptr1] <= b[ptr2]){
            cdf1 += inv_m;
            if (d < fabs(cdf1 - cdf2)){
                d = fabs(cdf1 - cdf2);
                pos_max = a[ptr1];
            }
            ptr1 += 1;
        }
        else {
            cdf2 += inv_n;
            if (d < fabs(cdf1 - cdf2)){
                d = fabs(cdf1 - cdf2);
                pos_max = b[ptr2];
            }
            ptr2 += 1;
        }
    }
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

void Progress::setError(QString interrupt_message){
    stop_progress = true;
    error(interrupt_message);
}

void Progress::interrupt(){
    stop_progress = true;
}


void Progress::reset(){
    stop_progress = false;
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

double compute_aic(Matrix &data, Array &coefficiants, Array &values){
    Array S;
    S.mult(data, coefficiants);
    double s = 0;
    //double t = 0;
    //double mue = S.mean();
    for (int i = 0; i < (int)S.size(); ++i) s += sq(S[i] - values[i]);
    return s;
    //for (int i = 0; i < (int)S.size(); ++i) t += sq(S[i] - mue);
    //return s / t;
    
    int k = data.cols;
    int n = data.rows;
    return n * (log(2 * M_PI) + 1 + log(s / n)) + (k * 2);
}


bool gene_aic(Gene g1, Gene g2){
    return g1.score < g2.score;
}

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


ListItem::ListItem(QString name, ListItemType t, QListWidget* parent) : QListWidgetItem(name, parent) {
    type = t;
    length = 0;
}




void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    painter->setPen(QPen(Qt::NoPen));
    int rr = index.row();
    double l = ((ListItem*)((QListWidget*)parent())->item(rr))->length;
    l = max(0., min(1., l));
    
    if (l > 1e-15) {
        painter->setBrush(QBrush(QColor(128, 128, 128, 20)));
        QRect r = option.rect;
        r.setWidth(r.width() * l - 2);
        painter->drawRect(r);
    }
    QItemDelegate::paint(painter, option, index);
}


TreeItem::TreeItem(int pos, QString name, string f, QTreeWidgetItem* parent) : QTreeWidgetItem(parent){
    setText(pos, name);
    feature = f;
}



TreeItem::TreeItem(int pos, QString name, QTreeWidget* parent) : QTreeWidgetItem(parent){
    setText(pos, name);
    feature = "";
}


Gene::Gene(int features){
    score = -1;
    gene_code.resize(features, false);
}


Gene::Gene(Gene *gene){
    score = gene->score;
    for (bool feature : gene->gene_code) gene_code.push_back(feature);
}

Gene::Gene(Gene *g1, Gene *g2, double mutation_rate){
    score = -1;
    for (int i = 0; i < (int)g1->gene_code.size(); ++i){
        bool feature = randnum() < 0.5 ? g1->gene_code[i] : g2->gene_code[i];
        if (randnum() < mutation_rate) feature = !feature;
        gene_code.push_back(feature);
    }
}

void Gene::get_indexes(Indexes &indexes){
    indexes.clear();
    for (int i = 0; i < (int)gene_code.size(); ++i){
        if (gene_code[i]) indexes.push_back(i);
    }
}
