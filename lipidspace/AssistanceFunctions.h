#ifndef ASSISTANCE_FUNCTIONS_H
#define ASSISTANCE_FUNCTIONS_H

#include <QtCore>
#include <QListWidget>
#include <QTreeWidget>
#include <QDropEvent>
#include <set>
#include <vector>
#include <map>
#include <string>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/Matrix.h"
#include "lipidspace/logging.h"
#include <iostream>
#include <algorithm>
#include <math.h>

#define rand() ((double)random() / (double)(RAND_MAX))

using namespace std;

enum Linkage {SingleLinkage, AverageLinkage, CompleteLinkage};
enum FeatureType {NumericalFeature, NominalFeature};
enum ListItemType {SPECIES_ITEM = 0, CLASS_ITEM = 1, CATEGORY_ITEM = 2, SAMPLE_ITEM = 3};
enum TableColumnType {SampleColumn, QuantColumn, LipidColumn, FeatureColumnNumerical, FeatureColumnNominal, IgnoreColumn};
enum LipidSpaceExceptionType {UnspecificException, LipidUnparsable, FileUnreadable, LipidDoublette, NoColumnFound, ColumnNumMismatch, LipidNotRegistered, FeatureNotRegistered, CorruptedFileFormat};
enum FeatureFilter {NoFilter = 0, LessFilter = 1, GreaterFilter = 2, EqualFilter = 3, WithinRange = 4, OutsideRange = 5};



class FeatureSet {
public:
    string name;
    FeatureType feature_type;
    map<string, bool> nominal_values;
    set<double> numerical_values;
    pair<FeatureFilter, vector<double>> numerical_filter;
    
    FeatureSet(string _name, FeatureType f_type){
        name = _name;
        feature_type = f_type;
    }
    
    FeatureSet(){
        name = "";
        feature_type = NominalFeature;
        numerical_filter = {NoFilter, vector<double>()};
    }
};





struct Feature {
    string name;
    FeatureType feature_type;
    double numerical_value;
    string nominal_value;
    
    Feature(){
        name = "";
        feature_type = NominalFeature;
        numerical_value = 0;
        nominal_value = "";
    }
    
    Feature (string _name, string nom_val){
        name = _name;
        feature_type = NominalFeature;
        nominal_value = nom_val;
        numerical_value = 0;
    }
    
    Feature (string _name, double num_val){
        name = _name;
        feature_type = NumericalFeature;
        numerical_value = num_val;
        nominal_value = "";
    }
};


class Gene {
public:
    vector<bool> gene_code;
    double aic;
    
    Gene(int features);
    Gene(Gene *gene);
    Gene(Gene *g1, Gene *g2, double mutation_rate = 0.);
    void get_indexes(Indexes &indexes);
};


double KS_pvalue(vector<double> &sample1, vector<double> &sample2);
void BH_fdr(vector<double> &data);
double compute_aic(Matrix &data, Array &coefficiants, Array &values);
bool gene_aic(Gene g1, Gene g2);


class LipidSpaceException : public std::exception {
public:
    string message;
    LipidSpaceException(string _message, LipidSpaceExceptionType _type = UnspecificException){
        Logging::write_log(message);
        message = _message;
        type = _type;
    }
    
    const char * what() const throw(){
        return message.c_str();
    }
    
    LipidSpaceExceptionType type;
};




class SingleListWidget : public QListWidget {
    Q_OBJECT
    
public:
    SingleListWidget(QWidget *parent = nullptr);
    string field_name;
    
    void addFieldName(string _field_name);
    void setNum(int _num);
    
signals:
    void oneItemViolation(string, int);
    
public slots:
    void dropEvent(QDropEvent *event) override;
    
private:
    int num;
};




class Table {
public:
    string file_name;
    string cleaned_name;
    vector<string> species;
    vector<string> classes;
    vector<string> categories;
    vector<LipidAdduct*> lipids;
    vector<bool> selection;
    Array intensities;
    Array PCA_intensities;
    Array original_intensities;
    map<string, Feature> features;
    Matrix m;
    
    Table(string lipidome_name, string lipidome_file, bool is_file_name = false) : file_name(lipidome_file) {
        QFileInfo qFileInfo(file_name.c_str());
        string cleaned_file = qFileInfo.baseName().toStdString();
        if (is_file_name){
            cleaned_name = cleaned_file;
        }
        else {
            cleaned_name = lipidome_name;
        }
        features.insert({"File", Feature("File", cleaned_file)});
    }
};


class ListItem : public QListWidgetItem {
public:
    ListItemType type;
    ListItem(QString name, ListItemType t, QListWidget* parent);
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
    void increment();
    void set(int);
    
public slots:
    void interrupt();
    void setError(QString);
    void set_step();
    void prepare_steps(int);
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
    
    DendrogramNode(int index, map<string, FeatureSet> *feature_values, Table *lipidome);
    DendrogramNode(DendrogramNode* n1, DendrogramNode* n2, double d);
    ~DendrogramNode();
    double* execute(int i, Array* points, vector<int>* sorted_ticks);
};


#endif /* ASSISTANCE_FUNCTIONS_H */
