#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/globaldata.h"

LipidSpaceException::LipidSpaceException(string _message, LipidSpaceExceptionType _type){
    Logging::write_log(message);
    message = _message;
    type = _type;
}






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






CBTableView::CBTableView(QWidget *parent) : QTableView(parent) {
    QAbstractButton *btn = findChild<QAbstractButton *>();
    transpose_label = 0;

    if (btn != NULL) {
        QVBoxLayout* lay = new QVBoxLayout(btn);
        lay->setContentsMargins(0, 0, 0, 0);
        transpose_label = new QLabel("Transpose");
        transpose_label->setAlignment(Qt::AlignCenter);
        transpose_label->setContentsMargins(2, 2, 2, 2);
        lay->addWidget(transpose_label);
        btn->disconnect(SIGNAL(clicked()));
        connect(btn, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
    }
}


void CBTableView::cornerButtonClicked(){
    cornerButtonClick();
}


void CBTableView::wheelEvent(QWheelEvent* event){
    if (GlobalData::ctrl_pressed){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool zoom_in = event->angleDelta().y() > 0;
#else
        bool zoom_in = event->delta() > 0;
#endif
        int zoom_value = max(1, (int)GlobalData::gui_num_var["table_zoom"] + (2 * zoom_in - 1));
        GlobalData::gui_num_var["table_zoom"] = zoom_value;
        QFont item_font("Helvetica", zoom_value);
        transpose_label->setFont(item_font);
        emit zooming();
    }
    else {
        QTableView::wheelEvent(event);
    }
}







Mapping::Mapping(){
    name = "";
    parent = "";
    action = NoAction;
    study_variable_type = NominalStudyVariable;
    mapping = "";
}

Mapping::Mapping(string _name, StudyVariableType v_type){
    name = _name;
    parent = "";
    study_variable_type = v_type;
    action = NoAction;
    mapping = "";
}




ImportData::ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types){
    table_file = _table_file;
    sheet = _sheet;
    table_type = _table_type;
    column_types = _column_types;
    mapping_data = 0;

    ifstream infile(table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + table_file + "' could not be found.", FileUnreadable);
    }
    try {
        file_table_handler = new FileTableHandler(table_file, sheet);
    }
    catch (OpenXLSX::XLInputError &e){
        throw LipidSpaceException("Error: file '" + table_file + "' could not be read.", FileUnreadable);
    }
}



ImportData::ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types, FileTableHandler *_file_table_handler){
    table_file = _table_file;
    sheet = _sheet;
    table_type = _table_type;
    column_types = _column_types;
    file_table_handler = _file_table_handler;
    mapping_data = 0;
}



ImportData::ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types, FileTableHandler *_file_table_handler, MappingData *_mapping_data){
    table_file = _table_file;
    sheet = _sheet;
    table_type = _table_type;
    column_types = _column_types;
    file_table_handler = _file_table_handler;
    mapping_data = _mapping_data;
}


ImportData::~ImportData(){
    delete column_types;
    delete mapping_data;
    delete file_table_handler;
}





KeyTableWidget::KeyTableWidget(QWidget *parent) : QTableWidget(parent){
}


void KeyTableWidget::keyPressEvent(QKeyEvent *event) {
    QTableWidget::keyPressEvent(event);
    emit keyPressed(event);
}




StudyVariableSet::StudyVariableSet(string _name, StudyVariableType v_type){
    name = _name;
    study_variable_type = v_type;
}


StudyVariableSet::StudyVariableSet(){
    name = "";
    study_variable_type = NominalStudyVariable;
    numerical_filter = {NoFilter, vector<double>()};
}




StudyVariableSet::StudyVariableSet(StudyVariableSet *study_variable_set){
    name = study_variable_set->name;
    study_variable_type = study_variable_set->study_variable_type;
    for (auto kv : study_variable_set->nominal_values) nominal_values.insert({kv.first, kv.second});
    for (auto value : study_variable_set->numerical_values) numerical_values.insert(value);
    numerical_filter = {study_variable_set->numerical_filter.first, vector<double>()};
    for (auto value : study_variable_set->numerical_filter.second) numerical_filter.second.push_back(value);
}





StudyVariable::StudyVariable(){
    name = "";
    study_variable_type = NominalStudyVariable;
    numerical_value = 0;
    nominal_value = "";
    missing = false;
}


StudyVariable::StudyVariable (string _name, string nom_val, bool _missing){
    name = _name;
    study_variable_type = NominalStudyVariable;
    nominal_value = nom_val;
    numerical_value = 0;
    missing = _missing;
}


StudyVariable::StudyVariable (string _name, double num_val, bool _missing){
    name = _name;
    study_variable_type = NumericalStudyVariable;
    numerical_value = num_val;
    nominal_value = "";
    missing = _missing;
}

StudyVariable::StudyVariable (StudyVariable *f){
    name = f->name;
    study_variable_type = f->study_variable_type;
    numerical_value = f->numerical_value;
    nominal_value = f->nominal_value;
    missing = f->missing;
}




ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags) : QLabel(parent){

}


ClickableLabel::~ClickableLabel(){

}


void ClickableLabel::mouseDoubleClickEvent(QMouseEvent*) {
    emit doubleClicked();
}


void ClickableLabel::mousePressEvent(QMouseEvent*) {
    emit clicked();
}





SignalCombobox::SignalCombobox(QWidget *parent, int _row) : QComboBox(parent) {
    tool_tip = "";
    row = _row;
    connect(this, (void (SignalCombobox::*)(int))&SignalCombobox::currentIndexChanged, this, &SignalCombobox::changedIndex);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SignalCombobox::enterEvent(QEnterEvent *event) {
#else
void SignalCombobox::enterEvent(QEvent *event) {
#endif
	
    QComboBox::enterEvent(event);
    QToolTip::showText(QCursor::pos(), tool_tip);
}


void SignalCombobox::leaveEvent(QEvent *event){
    QComboBox::leaveEvent(event);
    QToolTip::hideText();
}

void SignalCombobox::changedIndex(int ){
    emit comboChanged(this);
}





SignalLineEdit::SignalLineEdit(int _row, StudyVariableType ft, QWidget *parent) : QLineEdit(parent){
    row = _row;
    study_variable_type = ft;
    connect(this, (void (SignalLineEdit::*)(const QString &))&SignalLineEdit::textChanged, this, &SignalLineEdit::changedText);
}


SignalLineEdit::SignalLineEdit(QWidget *parent) : QLineEdit(parent){
    row = 0;
    connect(this, (void (SignalLineEdit::*)(const QString &))&SignalLineEdit::textChanged, this, &SignalLineEdit::changedText);
}


void SignalLineEdit::changedText(const QString &txt){
    emit lineEditChanged(this, txt);
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
        cleaned_name = cleaned_file + (sheet_name.length() > 0 ? "/" + sheet_name : "");
    }
    else {
        string suffix = (cleaned_file.length() > 0) ? " - " + cleaned_file : "";
        if (suffix.length() > 0 && sheet_name.length() > 0) suffix += "/" + sheet_name;
        cleaned_name = lipidome_name + suffix;
    }
    study_variables.insert({FILE_STUDY_VARIABLE_NAME, StudyVariable(FILE_STUDY_VARIABLE_NAME, cleaned_file + (sheet_name.length() > 0 ?  "/" + sheet_name : ""))});
}




Lipidome::Lipidome(Lipidome *lipidome){
    file_name = lipidome->file_name;
    cleaned_name = lipidome->cleaned_name;
    for (auto value : lipidome->species) species.push_back(value);
    for (auto value : lipidome->classes) classes.push_back(value);
    for (auto value : lipidome->categories) categories.push_back(value);
    for (auto value : lipidome->visualization_intensities) visualization_intensities.push_back(value);
    for (auto value : lipidome->selected_lipid_indexes) selected_lipid_indexes.push_back(value);
    for (auto value : lipidome->normalized_intensities) normalized_intensities.push_back(value);
    for (auto value : lipidome->PCA_intensities) PCA_intensities.push_back(value);
    for (auto value : lipidome->original_intensities) original_intensities.push_back(value);
    for (auto kv : lipidome->study_variables) study_variables.insert({kv.first, StudyVariable(&kv.second)});
    m.rewrite(lipidome->m);
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





DendrogramNode::DendrogramNode(int index, map<string, StudyVariableSet> *study_variable_values, Lipidome *lipidome){
    indexes.insert(index);
    order = -1;
    left_child = 0;
    right_child = 0;
    distance = 0;
    x_left = 0;
    x_right = 0;
    y = 0;
    min_distance = {INFINITY, 0};
    depth = 0;


    // initialize empty study variable count table
    for (auto kv : *study_variable_values){
        if (kv.second.study_variable_type == NominalStudyVariable){
            study_variable_count_nominal.insert({kv.first, map<string, int>()});
            for (auto kv_nom : kv.second.nominal_values){
                study_variable_count_nominal[kv.first].insert({kv_nom.first, 0});
            }
        }
        else {
            study_variable_numerical.insert({kv.first, vector<double>()});
        }
    }

    for (auto kv : lipidome->study_variables){
        if (kv.second.study_variable_type == NominalStudyVariable){
            study_variable_count_nominal[kv.first][kv.second.nominal_value] = 1;
        }
        else {
            study_variable_numerical[kv.first].push_back(kv.second.numerical_value);
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
    min_distance = {INFINITY, 0};
    for (auto i : n1->indexes) indexes.insert(i);
    for (auto i : n2->indexes) indexes.insert(i);
    distance = d;
    depth = 1 + max(n1->depth, n2->depth);
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

    // count study variables
    for (auto kv : left_child->study_variable_count_nominal){
        study_variable_count_nominal.insert({kv.first, map<string, int>()});
        for (auto kv2 : kv.second){
            study_variable_count_nominal[kv.first].insert({kv2.first, kv2.second + right_child->study_variable_count_nominal[kv.first][kv2.first]});
        }
    }

    for (auto kv : left_child->study_variable_numerical){
        // find intersection points for numerical study variables
        vector<double> &set1 = kv.second;
        vector<double> &set2 = right_child->study_variable_numerical[kv.first];

        double pos_max = 0, d = 0;
        ks_separation_value(set1, set2, d, pos_max);
        study_variable_numerical_thresholds.insert({kv.first, pos_max});

        study_variable_numerical.insert({kv.first, vector<double>()});
        for(double val : kv.second){
            study_variable_numerical[kv.first].push_back(val);
        }
        for(double val : right_child->study_variable_numerical[kv.first]){
            study_variable_numerical[kv.first].push_back(val);
        }
    }
    return new double[3]{(x_left + x_right) / 2, y, (double)cnt};
}



void DendrogramNode::update_distances(set<DendrogramNode*> &nodes, Matrix &m){
    min_distance = {INFINITY, 0};
    for (auto node : nodes){
        if (node == this) continue;

        double dist = INFINITY;
        if (contains_val(distances, node)){
            dist = distances[node];
        }
        else {
            Linkage lnk = GlobalData::linkage;
            dist = (lnk == SingleLinkage) ? INFINITY : 0;
            if (lnk == AverageLinkage) {
                for (auto index1 : indexes){
                    for (auto index2 : node->indexes){
                        dist += m(index1, index2);
                    }
                }
                dist /= (double)(indexes.size() * node->indexes.size());
            }
            else {
                for (auto index1 : indexes){
                    for (auto index2 : node->indexes){
                        double hd = m(index1, index2);
                        if ((lnk == CompleteLinkage && dist < hd) || (lnk == SingleLinkage && (dist == INFINITY || dist > hd))){
                            dist = hd;
                        }
                    }
                }
            }
            distances.insert({node, dist});
            if (uncontains_val(node->distances, this)) node->distances.insert({this, dist});
        }

        if (min_distance.first == INFINITY || dist < min_distance.first || (dist == min_distance.first && node->depth < min_distance.second->depth)){
            min_distance = {dist, node};
        }
    }
}



double compute_accuracy(vector<Array> &arrays){
    // remove empty arrays
    for (int i = (int)arrays.size() - 1; i >= 0; --i){
        if (arrays[i].size() == 0){
            arrays.erase(arrays.begin() + i);
        }
    }

    set<uint> indexes;
    Indexes medians;
    double all = 0;
    for (uint i = 0; i < arrays.size(); ++i){
        auto &array = arrays[i];
        sort(array.begin(), array.end());
        medians.push_back(array.median(-1, -1, true));
        indexes.insert(i);
        all += array.size();
    }
    // compute all separation positions for each pair of set
    vector< vector<double> > border_matrix(arrays.size(), vector<double>(arrays.size()));
    for (uint i = 0; i < arrays.size() - 1; ++i){
        for (uint j = i + 1; j < arrays.size(); ++j){
            double d = 0;
            ks_separation_value(arrays[i], arrays[j], d, border_matrix[i][j]);
        }
    }

    // define a list of all boundaries for each category
    Array borders;
    Indexes array_order;
    borders.push_back(-INFINITY);
    while (indexes.size() > 1){
        uint min_i = 0;
        uint min_j = 0;
        double min_value = INFINITY;

        for (auto i : indexes){
            for (auto j : indexes){
                if (i >= j) continue;
                if (min_value == INFINITY || min_value > border_matrix[i][j]) {
                    min_i = i;
                    min_j = j;
                    min_value = border_matrix[i][j];
                }
            }
        }

        borders.push_back(min_value);
        if (medians[min_i] < medians[min_j]){
            indexes.erase(min_i);
            array_order.push_back(min_i);
        }
        else {
            indexes.erase(min_j);
            array_order.push_back(min_j);
        }
    }
    array_order.push_back(*indexes.begin());
    borders.push_back(INFINITY);

    // count all true hits
    double true_hits = 0;
    for (uint i = 0; i < borders.size() - 1; ++i){
        double border_lower = borders[i];
        double border_upper = borders[i + 1];
        int left = arrays[array_order[i]].greatest_less(border_lower) + 1;
        int right = arrays[array_order[i]].greatest_less(border_upper);
        true_hits += right - left + 1;
    }

    return true_hits / all;
}



void ks_separation_value(vector<double> &a, vector<double> &b, double &d, double &pos_max, pair<vector<double>, vector<double>> *ROC){
    d = 0;
    pos_max = 0;
    int num1 = a.size();
    int num2 = b.size();
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    double inv_m = 1. / (double)num1, inv_n = 1. / (double)num2;
    double ptr1 = 0, ptr2 = 0;
    double cdf1 = 0, cdf2 = 0;

    while ((ptr1 < num1) && (ptr2 < num2)){
        if (ROC){
            ROC->first.push_back((num1 - ptr1) * inv_m);
            ROC->second.push_back((num2 - ptr2) * inv_n);
        }
        if (a[ptr1] <= b[ptr2]){
            cdf1 += inv_m;
            if (d < __abs(cdf1 - cdf2)){
                d = __abs(cdf1 - cdf2);
                pos_max = a[ptr1];
            }
            ptr1 += 1;
        }
        else {
            cdf2 += inv_n;
            if (d < __abs(cdf1 - cdf2)){
                d = __abs(cdf1 - cdf2);
                pos_max = b[ptr2];
            }
            ptr2 += 1;
        }
    }
}





//////////////////////////////////////////////////////////////////////////
// MIT License
// Copyright (c) 2016 Mengchang Wang
// libAnova
// https://github.com/Amuwa/libAnova
//////////////////////////////////////////////////////////////////////////


double f_distribution_cdf(double F, double m, double n){
    double xx = 0.0, p = 0.0;

    if (m <= 0. || n <= 0.) p = -1.;
    else if (F > 0){
        xx = F / (F + n / m);
        p = betainc(xx, m * 0.5, n * 0.5, true);
    }

    return p;
}



double betainc(double x,double a, double b, bool flip_value) {
    double y, BT, AAA;

    if(x == 0. || x == 1.){
        BT = 0.;
    }
    else {
        AAA = lgamma(a + b) - lgamma(a) - lgamma(b);
        BT = exp(AAA + a * log(x) + b * log(1. - x));
    }
    if(x < (a + 1.) / (a + b + 2.)){
        if (flip_value) y = 1. - BT * beta_cf(a, b, x) / a;
        else y = BT * beta_cf(a, b, x) / a;
    }
    else {
        if (flip_value) y = BT * beta_cf(b, a, 1. - x) / b;
        else y = 1. - BT * beta_cf(b, a, 1. - x) / b;
    }

    return y;
}

double beta_cf(double a,double b,double x) {
    const int count_max = 100;
    const double eps = 1e-7;
    double AM = 1.;
    double BM = 1.;
    double AZ = 1.;
    double EM, TEM, D, AP, BP, AAP, BPP, AOLD;

    double QAB = a + b;
    double QAP = a + 1.;
    double QAM = a - 1.;
    double BZ = 1. - QAB * x / QAP;

    for(int count = 1; count <= count_max; count++){
        EM = count;
        TEM = EM + EM;
        D = EM * (b - count) * x / ((QAM + TEM) * (a + TEM));
        AP = AZ + D * AM;
        BP = BZ + D * BM;
        D = -(a + EM) * (QAB + EM) * x / ((a + TEM) * (QAP + TEM));
        AAP = AP + D * AZ;
        BPP = BP + D * BZ;
        AOLD = AZ;
        AM = AP / BPP;
        BM = BP / BPP;
        AZ = AAP / BPP;
        BZ = 1.;
        if(fabs(AZ - AOLD) < eps * fabs(AZ)) return AZ;
    }
    return AZ;
}






/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-distribution#Cumulative_distribution_function
*/
double t_distribution_cdf(double t_stat, double free_deg){
    double tt = free_deg / (sq(t_stat) + free_deg);
    return max(0., min(1., betainc(tt, free_deg * 0.5, 0.5)));
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

    return max(0., min(1., t_distribution_cdf(t, v)));
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
    double t = __abs(sqrt(n * m / (n + m)) * ((m1 - m2) / s));
    return max(0., min(1., t_distribution_cdf(t, n + m - 2.)));
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
    Array sums;
    Array means;
    double I = arrays.size();
    double N = 0;
    double total_sum = 0;
    double MSB = 0;
    double MSW = 0;
    for (auto a : arrays){
        sums.push_back(a.sum());
        total_sum += sums.back();

        means.push_back(a.mean());
        N += a.size();

        MSB += sums.back() * means.back();
    }
    for (uint i = 0; i < arrays.size(); ++i){
        auto a = arrays[i];
        double m = means[i];
        for (auto v : a){
            MSW += sq(v - m);
        }
    }

    MSB -= sq(total_sum) / N;

    N -= I;
    I -= 1;

    double F = MSB * N / (MSW * I);
    return max(0., min(1., f_distribution_cdf(F, I, N)));
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



double compute_aic(Matrix &data, Array &coefficiants, Array &values){
    Array S;
    S.mult(data, coefficiants);
    double s = 0;
    for (int i = 0; i < (int)S.size(); ++i) s += sq(S[i] - values[i]);
    if (s == 0) return s;

    int k = data.cols;
    int n = data.rows;
    return n * (log(2. * M_PI) + 1. + log(s / n)) + (k * 2.);
}




bool gene_aic(Gene g1, Gene g2){
    return g1.score < g2.score;
}


ListItem::ListItem(string show_name, ListItemType t, QListWidget* parent, string _system_name) : QListWidgetItem(show_name.c_str(), parent) {
    system_name = _system_name.length() > 0 ? _system_name : show_name;
    type = t;
    length = 0;
}




ItemDelegate::ItemDelegate(QObject *parent) : QItemDelegate(parent) {

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


TreeItem::TreeItem(int pos, QString name, string var, QTreeWidgetItem* parent) : QTreeWidgetItem(parent){
    setText(pos, name);
    study_variable = var;
}



TreeItem::TreeItem(int pos, QString name, QTreeWidget* parent) : QTreeWidgetItem(parent){
    setText(pos, name);
    study_variable = "";
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
