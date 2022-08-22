#ifndef ASSISTANCE_FUNCTIONS_H
#define ASSISTANCE_FUNCTIONS_H

#include <QtCore>
#include <QListWidget>
#include <QTreeWidget>
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
#include <QCheckBox>
#include <QVBoxLayout>
#include <set>
#include <vector>
#include <map>
#include <string>
#include <stdlib.h>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/Matrix.h"
#include "lipidspace/logging.h"
#include <iostream>
#include <algorithm>
#include <math.h>

#define randnum() ((double)rand() / (double)(RAND_MAX))


#define UNDEFINED_LIPID "UNDEFINED"
#define FILE_FEATURE_NAME "Origin"
#define NO_VALUE_CHAR "Ø"

using namespace OpenXLSX;
using namespace std;

enum Linkage {SingleLinkage, AverageLinkage, CompleteLinkage};
enum FeatureType {NumericalFeature, NominalFeature, NominalValue};
enum ListItemType {SPECIES_ITEM = 0, CLASS_ITEM = 1, CATEGORY_ITEM = 2, SAMPLE_ITEM = 3};
enum TableColumnType {SampleColumn, QuantColumn, LipidColumn, FeatureColumnNumerical, FeatureColumnNominal, IgnoreColumn};
enum LipidSpaceExceptionType {UnspecificException, LipidUnparsable, FileUnreadable, LipidDoublette, NoColumnFound, ColumnNumMismatch, LipidNotRegistered, FeatureNotRegistered, CorruptedFileFormat};
enum FeatureFilter {NoFilter = 0, LessFilter = 1, GreaterFilter = 2, EqualFilter = 3, WithinRange = 4, OutsideRange = 5};
enum TableType {ROW_PIVOT_TABLE, COLUMN_PIVOT_TABLE, FLAT_TABLE};
enum LipidNameState {TRANSLATED_NAME = 0, IMPORT_NAME = 1};
enum MappingAction {NoAction, RegisterNewNaN, RegisterNewDefault, RenameAction, MappingTo, InsertNaN, InsertDefault};
enum NormalizationType {AbsoluteNormalization, RelativeNormalization, GroupNormalization, UndefinedNormalization};
enum CanvasType {UndefinedCanvasType, DendrogramCanvas, GlobalSpaceCanvas, StudySpaceCanvas, SampleSpaceCanvas};

static const map<string, TableType> TableTypeMap{{"ROW_PIVOT_TABLE", ROW_PIVOT_TABLE}, {"COLUMN_PIVOT_TABLE", COLUMN_PIVOT_TABLE}, {"FLAT_TABLE", FLAT_TABLE}};
static const map<string, TableColumnType> TableColumnTypeMap{{"SampleColumn", SampleColumn}, {"QuantColumn", QuantColumn}, {"LipidColumn", LipidColumn}, {"FeatureColumnNumerical", FeatureColumnNumerical}, {"FeatureColumnNominal", FeatureColumnNominal}, {"IgnoreColumn", IgnoreColumn}};
static const set<string> NA_VALUES{"NA", "nan", "N/A", "", "n/a", "NaN", NO_VALUE_CHAR};



class Mapping {
public:
    string name;
    string parent;
    MappingAction action;
    string mapping;
    FeatureType feature_type;

    Mapping();
    Mapping(string _name, FeatureType _feature_type);
};


typedef map< FeatureType, map<string, Mapping> > MappingData;



class KeyTableWidget : public QTableWidget {
    Q_OBJECT

public:
    KeyTableWidget(QWidget *parent = nullptr);

signals:
    void keyPressed(QKeyEvent *event);

public slots:
    void keyPressEvent(QKeyEvent *event) override;

};





class CBTableWidget : public QTableWidget {
    Q_OBJECT

public:
    QLabel *transpose_label;

    explicit CBTableWidget(QWidget *parent = 0);
    void wheelEvent(QWheelEvent*) override;

signals:
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
    FeatureType feature_type;

    SignalLineEdit(int _row, FeatureType ft, QWidget *parent = nullptr);
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
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void comboChanged(SignalCombobox *);

private slots:
    void changedIndex(int );
};






class FeatureSet {
public:
    string name;
    FeatureType feature_type;
    map<string, bool> nominal_values;
    set<double> numerical_values;
    pair<FeatureFilter, vector<double>> numerical_filter;

    FeatureSet(string _name, FeatureType f_type);
    FeatureSet();
    FeatureSet(FeatureSet *feature_set);
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


class Feature {
public:
    string name;
    FeatureType feature_type;
    double numerical_value;
    string nominal_value;
    bool missing;

    Feature();
    Feature (string _name, string nom_val, bool _missing = false);
    Feature (string _name, double num_val, bool _missing = false);
    Feature (Feature *f);
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


void BH_fdr(vector<double> &data);
double compute_aic(Matrix &data, Array &coefficiants, Array &values);
bool gene_aic(Gene g1, Gene g2);
void ks_separation_value(vector<double> &a, vector<double> &b, double &d, double &pos_max, double &separation_score, pair<vector<double>, vector<double>> *ROC = 0);
double hyperg_2F1(double a, double b, double c, double d);
double t_distribution_cdf(double t_stat, double free_deg);
double f_distribution_cdf(double fi_stat, double df1, double df2);
double p_value_kolmogorov_smirnov(Array &sample1, Array &sample2);
double p_value_student(Array &a, Array &b);
double p_value_welch(Array &a, Array &b);
double p_value_anova(vector<Array> &v);
double compute_accuracy(vector<Array> &v);



bool sort_double_string_desc (pair<double, string> i, pair<double, string> j);
bool sort_double_double_asc (pair<double, double> i, pair<double, double>);


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
    vector<string> species;
    vector<string> classes;
    vector<string> categories;
    vector<LipidAdduct*> lipids;
    Array visualization_intensities;
    vector<int> selected_lipid_indexes;
    Array normalized_intensities;
    Array PCA_intensities;
    Array original_intensities;
    map<string, Feature> features;
    Matrix m;

    Lipidome(string lipidome_name, string lipidome_file, string sheet_name = "", bool is_file_name = false);
    Lipidome(Lipidome *lipidome);
    string to_json();
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
    string feature;
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
    map<string, map<string, int>> feature_count_nominal;
    map<string, vector<double>> feature_numerical;
    map<string, double> feature_numerical_thresholds;

    DendrogramNode(int index, map<string, FeatureSet> *feature_values, Lipidome *lipidome);
    DendrogramNode(DendrogramNode* n1, DendrogramNode* n2, double d);
    DendrogramNode(DendrogramNode* n);
    ~DendrogramNode();
    double* execute(int i, Array* points, vector<int>* sorted_ticks);
};


#endif /* ASSISTANCE_FUNCTIONS_H */
