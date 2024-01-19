#ifndef ASSISTANCE_FUNCTIONS_H
#define ASSISTANCE_FUNCTIONS_H

#include <QtCore>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeView>
#include <QPen>
#include <QTableWidget>
#include <QBrush>
#include <QComboBox>
#include <QCursor>
#include <QLineEdit>
#include <QPainter>
#include <QLabel>
#include <QToolTip>
#include <QItemDelegate>
#include <OpenXLSX.hpp>
#include <QDropEvent>
#include <QApplication>
#include <QStylePainter>
#include <QCheckBox>
#include <QVBoxLayout>
#include <set>
#include <thread>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <stdlib.h>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/Matrix.h"
#include "lipidspace/logging.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <math.h>
#include <immintrin.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define randnum() ((double)rand() / (double)(RAND_MAX))
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#define __abs(a) (((a) >= 0) ? (a) : (-(a)))

#define UNDEFINED_LIPID "UNDEFINED"
#define FILE_STUDY_VARIABLE_NAME "Origin"
#define NO_VALUE_CHAR "Ø"
#define MASK32 4294967295ull
#define ulong unsigned long long
#define BENFORD_THRESHOLD 0.01

template<typename Base, typename T>
inline bool instanceof(const T *ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

using namespace OpenXLSX;
using namespace std;
using json = nlohmann::json;

enum Linkage {SingleLinkage, AverageLinkage, CompleteLinkage};
enum StudyVariableType {NumericalStudyVariable, NominalStudyVariable, NominalValue};
enum ListItemType {SPECIES_ITEM = 0, CLASS_ITEM = 1, CATEGORY_ITEM = 2, SAMPLE_ITEM = 3};
enum TableColumnType {SampleColumn, QuantColumn, LipidColumn, StudyVariableColumnNumerical, StudyVariableColumnNominal, IgnoreColumn};
enum LipidSpaceExceptionType {UnspecificException, LipidUnparsable, FileUnreadable, LipidDoublette, NoColumnFound, ColumnNumMismatch, LipidNotRegistered, StudyVariableNotRegistered, CorruptedFileFormat};
enum StudyVariableFilter {NoFilter = 0, LessFilter = 1, GreaterFilter = 2, EqualFilter = 3, WithinRange = 4, OutsideRange = 5};
enum TableType {ROW_PIVOT_TABLE, COLUMN_PIVOT_TABLE, FLAT_TABLE};
enum LipidNameState {NORMALIZED_NAME = 0, TRANSLATED_NAME = 1, IMPORT_NAME = 2};
enum MappingAction {NoAction, RegisterNewNaN, RegisterNewDefault, RenameAction, MappingTo, InsertNaN, InsertDefault};
enum CanvasType {UndefinedCanvasType, DendrogramCanvas, GlobalSpaceCanvas, GroupSpaceCanvas, SampleSpaceCanvas};
enum StatLevel {LipidomeLevel = 0, LipidClassLevel = 1, LipidSpeciesLevel = 2, FattyAcylLipid = 3};

static const map<string, TableType> TableTypeMap{{"ROW_PIVOT_TABLE", ROW_PIVOT_TABLE}, {"COLUMN_PIVOT_TABLE", COLUMN_PIVOT_TABLE}, {"FLAT_TABLE", FLAT_TABLE}};
static const map<string, TableColumnType> TableColumnTypeMap{{"SampleColumn", SampleColumn}, {"QuantColumn", QuantColumn}, {"LipidColumn", LipidColumn}, {"StudyVariableColumnNumerical", StudyVariableColumnNumerical}, {"StudyVariableColumnNominal", StudyVariableColumnNominal}, {"IgnoreColumn", IgnoreColumn}};
static const set<string> NA_VALUES{"NA", "nan", "N/A", "", "n/a", "NaN", NO_VALUE_CHAR};

class Lipidome;

class Mapping {
public:
    string name;
    string parent;
    MappingAction action;
    string mapping;
    StudyVariableType study_variable_type;

    Mapping();
    Mapping(string _name, StudyVariableType v_type);
};


typedef map< StudyVariableType, map<string, Mapping> > MappingData;



class KeyTableWidget : public QTableWidget {
    Q_OBJECT

public:
    KeyTableWidget(QWidget *parent = nullptr);

signals:
    void keyPressed(QKeyEvent *event);

public slots:
    void keyPressEvent(QKeyEvent *event) override;

};



class MultiSelectComboBox : public QComboBox
{
    Q_OBJECT

public:
    MultiSelectComboBox(QWidget* aParent = Q_NULLPTR);
    void addItem(const QString& aText, const QVariant& aUserData = QVariant());
    void addItems(const QStringList& aTexts);
    QStringList currentText();
    int count() const;
    void hidePopup() override;
    void SetSearchBarPlaceHolderText(const QString& aPlaceHolderText);
    void SetPlaceHolderText(const QString& aPlaceHolderText);
    void ResetSelection();

signals:
    void selectionChanged();

public slots:
    void clear();
    void setCurrentText(const QString& aText);
    void setCurrentText(const QStringList& aText);

protected:
    void wheelEvent(QWheelEvent* aWheelEvent) override;
    bool eventFilter(QObject* aObject, QEvent* aEvent) override;
    void keyPressEvent(QKeyEvent* aEvent) override;

private:
    void stateChanged(int aState);
    void onSearch(const QString& aSearchString);
    void itemClicked(int aIndex);

    QStringList stringList;
    QListWidget* mListWidget;
    QLineEdit* mLineEdit;
    QLineEdit* mSearchBar;
};



class FADTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    bool mouseOver = false;

    explicit FADTreeWidget(QWidget *parent = 0);
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void hoverEnter(QEvent* event);
    void hoverLeave(QEvent* event);
};



class CBTableView : public QTableView {
    Q_OBJECT

public:
    QLabel *transpose_label;

    explicit CBTableView(QWidget *parent = 0);
    void wheelEvent(QWheelEvent*) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

signals:
    void openFiles(const QList<QUrl> &);
    void cornerButtonClick();
    void zooming();


private slots:
    void cornerButtonClicked();
};





class FileTableHandler {
public:
    vector<string> headers;
    vector<vector<string>> rows;
    FileTableHandler(string, string = "");
};




class ImportData {
public:
    string table_file;
    vector<TableColumnType> *column_types;
    string sheet;
    TableType table_type;
    MappingData *mapping_data;
    FileTableHandler *file_table_handler;

    ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types);
    ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types, FileTableHandler *_file_table_handler);
    ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types, FileTableHandler *_file_table_handler, MappingData *_mapping_data);
    ~ImportData();
};


class SignalLineEdit : public QLineEdit {
    Q_OBJECT

public:
    int row;
    StudyVariableType study_variable_type;

    SignalLineEdit(int _row, StudyVariableType ft, QWidget *parent = nullptr);
    SignalLineEdit(QWidget *parent = nullptr);

signals:
    void lineEditChanged(SignalLineEdit *, QString);

private slots:
    void changedText(const QString &txt);
};





class SignalCombobox : public QComboBox {
    Q_OBJECT

public:
    QString tool_tip;
    int row;

    SignalCombobox(QWidget *parent = nullptr, int _row = -1);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

signals:
    void comboChanged(SignalCombobox *);

private slots:
    void changedIndex(int );
};






class StudyVariableSet {
public:
    string name;
    StudyVariableType study_variable_type;
    map<string, bool> nominal_values;
    set<double> numerical_values;
    pair<StudyVariableFilter, vector<double>> numerical_filter;

    StudyVariableSet(string _name, StudyVariableType f_type);
    StudyVariableSet();
    StudyVariableSet(StudyVariableSet *study_varable_set);
    StudyVariableSet(json &container);
    void save(json &container);
};


class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags = Qt::WindowFlags());
    ~ClickableLabel();

signals:
    void clicked();
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);

};


class StudyVariable {
public:
    string name;
    StudyVariableType study_variable_type;
    double numerical_value;
    string nominal_value;
    bool missing;

    StudyVariable();
    StudyVariable(string _name, string nom_val, bool _missing = false);
    StudyVariable(string _name, double num_val, bool _missing = false);
    StudyVariable(StudyVariable *f);
    StudyVariable(json &container);
    void save(json &container);
};


class Gene {
public:
    vector<bool> gene_code;
    double score;

    Gene(int features);
    Gene(Gene *gene);
    Gene(Gene *g1, Gene *g2, double mutation_rate = 0.);
    void get_indexes(Indexes &indexes);
};

double compute_accuracy(vector<Array> &v);
double compute_aic(Matrix &data, Array &coefficiants, Array &values);

bool gene_aic(Gene g1, Gene g2);
double betainc(double x,double a, double b, bool flip_value = false);
double beta_cf(double a,double b,double x);
double p_value_kolmogorov_smirnov(Array &sample1, Array &sample2);
void ks_separation_value(vector<double> &a, vector<double> &b, double &d, double &pos_max, pair<vector<double>, vector<double>> *ROC = 0);
double t_distribution_cdf(double t_stat, double free_deg);
double f_distribution_cdf(double fi_stat, double df1, double df2);

double p_value_student(Array &a, Array &b);
double p_value_welch(Array &a, Array &b);
double p_value_anova(vector<Array> &v);
double p_value_chi_sq(Array &a, Array &b);
double cosine_similarity(Array &a, Array &b);
void multiple_correction_bonferoni(Array &a);
void multiple_correction_bh(Array &a);
bool test_benford(Array &a);
bool test_benford(Matrix &m);
bool test_benford(vector<Lipidome*> &l);


class Analytics : public QObject {
    Q_OBJECT


public slots:
    void analytics(string action);

private:
    QNetworkAccessManager qnam;
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply;
};




inline double compute_l2_norm(const double *a, const double *b, const int rows){
    __m256d dist = {0., 0., 0., 0.};
    for (int r = 0; r < rows; r += 4){
        __m256d val1 = *(__m256d*)(a + r);
        __m256d val2 = *(__m256d*)(b + r);
        __m256d sub = val1 - val2;
        dist += sub * sub;
    }
    //dist = _mm256_hadd_pd(dist, dist);
    return dist[0] + dist[1] + dist[2] + dist[3];
}




template <class T, class U>
class SortVector : public vector<pair<T, U>> {
public:
    SortVector();
    void sort_asc();
    void sort_desc();

private:
    static bool sort_pair_asc (pair<T, U> i, pair<T, U> j);
    static bool sort_pair_desc (pair<T, U> i, pair<T, U> j);
};

template<class T, class U>
SortVector<T, U>::SortVector(){

}

template <class T, class U>
bool SortVector<T, U>::sort_pair_asc (pair<T, U> i, pair<T, U> j){
    return (i.first < j.first);
}

template <class T, class U>
bool SortVector<T, U>::sort_pair_desc (pair<T, U> i, pair<T, U> j){
    return (i.first > j.first);
}

template <class T, class U>
void SortVector<T, U>::sort_asc (){
    sort(this->begin(), this->end(), sort_pair_asc);
}

template <class T, class U>
void SortVector<T, U>::sort_desc (){
    sort(this->begin(), this->end(), sort_pair_desc);
}




class LipidSpaceException : public std::exception {
public:
    string message;
    LipidSpaceException(string _message, LipidSpaceExceptionType _type = UnspecificException);

    const char * what() const throw(){
        return message.c_str();
    }

    LipidSpaceExceptionType type;
};




class SingleListWidget : public QListWidget {
    Q_OBJECT

public:
    string field_name;

    SingleListWidget(QWidget *parent = nullptr);
    void addFieldName(string _field_name);
    void setNum(int _num);

signals:
    void oneItemViolation(string, int);

public slots:
    void dropEvent(QDropEvent *event) override;

private:
    int num;
};






class Lipidome {
public:
    string file_name;
    string cleaned_name;
    string lipidome_name;
    string suffix;
    vector<string> species;
    vector<string> classes;
    vector<string> categories;
    vector<LipidAdduct*> lipids;
    Array visualization_intensities;
    vector<int> selected_lipid_indexes;
    Array normalized_intensities;
    Array PCA_intensities;
    Array original_intensities;
    map<string, StudyVariable> study_variables;
    Matrix m;

    Lipidome(string lipidome_name, string lipidome_file, string sheet_name = "", bool is_file_name = false);
    Lipidome(Lipidome *lipidome);
    Lipidome(json &container, map<string, LipidAdduct*> &all_lipids);
    void save(json &container);
    string to_json(map<string, string> *imported_lipid_names = 0);
};


class ListItem : public QListWidgetItem {
public:
    ListItemType type;
    double length;
    string system_name;

    ListItem(string name, ListItemType t, QListWidget* parent, string _system_name = "");
};





class ItemDelegate : public QItemDelegate
{
public:
    explicit ItemDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};




class TreeItem : public QTreeWidgetItem {
public:
    string study_variable;
    TreeItem(int pos, QString name, string f, QTreeWidgetItem* parent);
    TreeItem(int pos, QString name, QTreeWidget* parent);
};





class Progress : public QObject {
    Q_OBJECT

public:
    int current_progress;
    int max_progress;
    bool stop_progress;
    int step_size;
    bool connected;
    Progress();

public slots:
    void interrupt();
    void increment();
    void setError(QString);
    void prepare(int max);
    void reset();



signals:
    void set_current(int);
    void set_max(int);
    void finish();
    void error(QString);
};


class DendrogramNode {
public:
    set<int> indexes;
    int order;
    DendrogramNode* left_child;
    DendrogramNode* right_child;
    double distance;
    double x_left;
    double x_right;
    double y;
    pair<double, DendrogramNode*> min_distance;
    int depth = 0;


    map<string, map<string, int>> study_variable_count_nominal;
    map<string, vector<double>> study_variable_numerical;
    map<string, double> study_variable_numerical_thresholds;
    map<DendrogramNode*, double> distances;

    DendrogramNode(int index, map<string, StudyVariableSet> *study_variable_values, Lipidome *lipidome);
    DendrogramNode(DendrogramNode* n1, DendrogramNode* n2, double d);
    DendrogramNode(json &container);

    ~DendrogramNode();
    double* execute(int i, Array* points, vector<int>* sorted_ticks);
    void update_distances(set<DendrogramNode*> &nodes, Matrix &m);
    int get_linkages(vector<vector<double>> &linkages, int &index);
    void save(json &container);
};


#endif /* ASSISTANCE_FUNCTIONS_H */
