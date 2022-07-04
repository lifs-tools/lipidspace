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


Lipidome::Lipidome(string lipidome_name, string lipidome_file, string sheet_name, bool is_file_name) : file_name(lipidome_file) {
    QFileInfo qFileInfo(file_name.c_str());
    string cleaned_file = qFileInfo.baseName().toStdString();
    if (is_file_name){
        cleaned_name = cleaned_file;
    }
    else {
        cleaned_name = lipidome_name;
    }
    features.insert({FILE_FEATURE_NAME, Feature(FILE_FEATURE_NAME, cleaned_file + (sheet_name.length() > 0 ?  "/" + sheet_name : ""))});
}




string Lipidome::to_json(){
    stringstream s;

    s << "{\"LipidomeName\": \"" << replace_all(cleaned_name, "\"", "") << "\", ";

    s << "\"LipidNames\": [";
    for (uint l = 0; l < species.size(); ++l) {
        if (l) s << ", ";
        s << "\"" << replace_all(species[l], "\"", "") << "\"";
    }
    s << "], ";

    s << "\"Intensities\": [";
    for (uint l = 0; l < original_intensities.size(); ++l) {
        if (l) s << ", ";
        s << original_intensities[l];
    }
    s << "], ";

    s << "\"X\": [";
    for (int r = 0; r < m.rows; ++r) {
        if (r) s << ", ";
        s << m(r, 0);
    }
    s << "], ";

    s << "\"Y\": [";
    for (int r = 0; r < m.rows; ++r) {
        if (r) s << ", ";
        s << m(r, 1);
    }
    s << "]";

    s << "}";
    return s.str();
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

        double pos_max = 0, d = 0, sep = 0;
        ks_separation_value(set1, set2, d, pos_max, sep);
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




bool sort_double_string_desc (pair<double, string> i, pair<double, string> j) { return (i.first > j.first); }
bool sort_double_double_asc (pair<double, double> i, pair<double, double> j) { return (i.first < j.first); }







double compute_accuracy(vector<Array> &v){
    vector<pair<double, double>> medians;
    for (uint i = 0; i < v.size(); ++i){
        auto &a = v[i];
        sort(a.begin(), a.end());
        medians.push_back({a.median(-1, -1, true), i});
    }
    sort(medians.begin(), medians.end(), sort_double_double_asc);
    double TP = 0, TN = 0, FP = 0, FN = 0;

    Array borders;
    for (uint i = 0; i < medians.size() - 1; ++i){
        double d = 0;
        double pos_max = 0;
        double separation_score = 0;
        int index_first = medians[i].second;
        int index_second = medians[i + 1].second;
        ks_separation_value(v[index_first], v[index_second], d, pos_max, separation_score);
        borders.push_back(pos_max);
    }
    borders.push_back(1e100);
    vector<int> pointers(v.size(), -1);

    for (uint i = 0; i < borders.size(); ++i){
        double border = borders[i];

        uint current_index = medians[i].second;
        for (uint j = 0; j < v.size(); ++j){
            int index = v[j].greatest_less(border, pointers[j]);

            if (current_index == j){
                TP += index - pointers[j];
                FN += v[j].size() - (index - pointers[j]);
            }
            else {
                FP += index - pointers[j];
                TN += v[j].size() - (index - pointers[j]);
            }
            pointers[j] = index;
        }
    }

    return (TP + TN) / (TP + TN + FP + FN);
}





void ks_separation_value(vector<double> &a, vector<double> &b, double &d, double &pos_max, double &separation_score, pair<vector<double>, vector<double>> *ROC){
    d = 0;
    pos_max = 0;
    separation_score = 0;
    int num1 = a.size();
    int num2 = b.size();
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    double inv_m = 1. / num1, inv_n = 1. / num2;
    double ptr1 = 0, ptr2 = 0;
    double cdf1 = 0, cdf2 = 0;
    double overlap1 = 0, overlap2 = 0;
    double min1 = 1, max1 = num1 - 1, min2 = 1, max2 = num2 - 1;
    if (num1 >= 15 && ROC == 0){
        min1 = floor(num1 * 0.05);
        max1 = ceil(num1 * 0.95);
    }
    if (num2 >= 15 && ROC == 0){
        min2 = floor(num2 * 0.05);
        max2 = ceil(num2 * 0.95);
    }
    while ((ptr1 < num1) && (ptr2 < num2)){
        if (ROC){
            ROC->first.push_back((num1 - ptr1) * inv_m);
            ROC->second.push_back((num2 - ptr2) * inv_n);
        }
        if (a[ptr1] <= b[ptr2]){
            cdf1 += inv_m;
            if (d < fabs(cdf1 - cdf2)){
                d = fabs(cdf1 - cdf2);
                pos_max = a[ptr1];
            }
            ptr1 += 1;
            if (min1 <= ptr1 && ptr1 <= max1 && min2 <= ptr2 && ptr2 <= max2) overlap1 += 1;
        }
        else {
            cdf2 += inv_n;
            if (d < fabs(cdf1 - cdf2)){
                d = fabs(cdf1 - cdf2);
                pos_max = b[ptr2];
            }
            ptr2 += 1;
            if (min1 <= ptr1 && ptr1 <= max1 && min2 <= ptr2 && ptr2 <= max2) overlap2 += 1;
        }
    }
    if (num1 > 0 && num2 > 0){
        separation_score = sqrt((1. - overlap1 / (double)num1) * (1. - overlap2 / (double)num2));
    }
}






/*
Sources:
https://www.cplusplus.com/forum/general/255896/
https://scicomp.stackexchange.com/questions/40536/hypergeometric-function-2f-1z-with-z-1-in-gsl
*/
double hyperg_2F1(double a, double b, double c, double d){
    double e = 1.;
    if (d > 1. || d < 0){
        a = c - a;
        e = 1. / pow(1. - d, b);
        d = 1. - 1. / (1. - d);
    }

    if (b < 0){
        e *= pow(1 - d, c - a - b);
        a = c - a;
        b = c - b;
    }

    double TOLERANCE = 1.0e-10;
    double term = a * b * d / c;
    double p = 1.0 + term;
    double n = 1.;

    while (abs( term ) > TOLERANCE){
        a += 1.;
        b += 1.;
        c += 1.;
        n += 1.;
        term *= a * b * d / c / n;
        if (isinf(term) or isnan(term)) break;
        p += term;
    }
    return p * e;
}



/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-distribution
*/
double t_distribution_cdf(double t_stat, double free_deg){
    if (t_stat > 0) t_stat = -t_stat;
    return (0.5 + t_stat * exp(lgamma((free_deg + 1.) / 2.) - lgamma(free_deg / 2.)) * hyperg_2F1(0.5, (free_deg + 1.) / 2., 1.5, - sq(t_stat) / free_deg) / sqrt(M_PI * free_deg)) * 2.;
}


/*
Source: https://en.wikipedia.org/wiki/Welch%27s_t-test
*/
double p_value_welch(Array &a, Array &b){
    double m1 = a.mean();
    double s1 = a.sample_stdev();
    double n = a.size();
    double m2 = b.mean();
    double s2 = b.sample_stdev();
    double m = b.size();

    double w0 = (m1 - m2);
    double t = w0 / sqrt(sq(s1 / sqrt(n)) + sq(s2 / sqrt(m)));
    double v = sq(sq(s1) / n + sq(s2) / m) / (sq(sq(s1)) / (sq(n) * (n - 1)) + sq(sq(s2)) / (sq(m) * (m - 1)));

    double pval = t_distribution_cdf(t, v);
    return max(0., min(1., pval));
}



/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-test
*/
double p_value_student(Array &sample1, Array &sample2){
    double m1 = sample1.mean();
    double s1 = sample1.sample_stdev();
    double n = sample1.size();
    double m2 = sample2.mean();
    double s2 = sample2.sample_stdev();
    double m = sample2.size();

    double s = sqrt(((n - 1) * sq(s1) + (m - 1) * sq(s2)) / (n + m - 2));
    double t = sqrt(n * m / (n + m)) * ((m1 - m2) / s);
    return t_distribution_cdf(t, n + m - 2);
}



/*
Sources:
https://en.wikipedia.org/wiki/F-distribution
https://docs.scipy.org/doc/scipy/reference/generated/scipy.special.betainc.html
*/
double f_distribution_cdf(double f_stat, double df1, double df2){

    double x = df1 * f_stat / (df1 * f_stat + df2);
    double a = df1 / 2.;
    double b = df2 / 2.;
    double pval = 1. - pow(x, a) / a * hyperg_2F1(a, 1 - b, a + 1, x) / beta(a, b);
    return max(0., min(1., pval));

}




/*
Algorithm adapted from: arXiv:2102.08037
Thomas Viehmann
Numerically more stable computation of the p-values for the two-sample Kolmogorov-Smirnov test
*/
double p_value_kolmogorov_smirnov(Array &sample1, Array &sample2){
    double d = 0, cdf1 = 0, cdf2 = 0;
    int ptr1 = 0, ptr2 = 0, m = sample1.size(), n = sample2.size();
    sort (sample1.begin(), sample1.end());
    sort (sample2.begin(), sample2.end());
    double inv_m = 1. / (double)m;
    double inv_n = 1. / (double)n;

    while (ptr1 < m && ptr2 < n){
        if (sample1[ptr1] < sample2[ptr2]){
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
    double pval = row[m - start_j];


    delete []lastrow;
    delete []row;
    return max(0., min(1., pval));
}




/*
Source: https://en.wikipedia.org/wiki/One-way_analysis_of_variance
*/
double p_value_anova(vector<Array> &arrays){
    vector<double> mean_y;
    vector<double> std_y;
    double total_y = 0;
    for (auto a : arrays){
        mean_y.push_back(a.mean());
        std_y.push_back(a.sample_stdev());
        total_y += mean_y.back();
    }
    total_y /= (double)mean_y.size();

    double MSB = 0;
    double MSW = 0;
    double N = 0;
    for (uint i = 0; i < mean_y.size(); ++i) {
        MSB += arrays[i].size() * sq(mean_y[i] - total_y);
        MSW += (arrays[i].size() - 1.) * sq(std_y[i]);
        N += arrays[i].size();
    }
    double v1 = mean_y.size() - 1.;
    double v2 = N - arrays.size();
    MSB /= v1;
    MSW /= v2;


    double F = MSB / MSW;
    return f_distribution_cdf(F, v1, v2);
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


void Progress::setError(QString interrupt_message){
    stop_progress = true;
    error(interrupt_message);
}


void Progress::interrupt(){
    stop_progress = true;
}



void Progress::reset(){
    stop_progress = false;
    current_progress = 0;
}



void Progress::prepare(int max_val){
    max_progress = max_val;
    current_progress = 0;
    set_max(max_progress);
}



bool sort_order_one (pair<double, int> i, pair<double, int> j) { return (i.first < j.first); }


double compute_aic(Matrix &data, Array &coefficiants, Array &values){
    Array S;
    S.mult(data, coefficiants);
    double s = 0;
    for (int i = 0; i < (int)S.size(); ++i) s += sq(S[i] - values[i]);
    return s;

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


ListItem::ListItem(string show_name, ListItemType t, QListWidget* parent, string _system_name) : QListWidgetItem(show_name.c_str(), parent) {
    system_name = _system_name.length() > 0 ? _system_name : show_name;
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
