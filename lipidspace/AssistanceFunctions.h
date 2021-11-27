#ifndef ASSISTANCE_FUNCTIONS_H
#define ASSISTANCE_FUNCTIONS_H

#include <QtCore>
#include <QListWidget>
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

using namespace std;

enum Linkage {SingleLinkage, CompleteLinkage};
enum ListItemType {SPECIES_ITEM = 0, CLASS_ITEM = 1, CATEGORY_ITEM = 2, SAMPLE_ITEM = 3};
enum TableColumnType {SampleColumn, QuantColumn, LipidColumn, FeatureColumn, IgnoreColumn};
enum LipidSpaceExceptionType {UnspecificException, LipidUnparsable, FileUnreadable, LipidDoublette, NoColumnFound, ColumnNumMismatch, LipidNotRegistered};


double KS_pvalue(vector<double> &sample1, vector<double> &sample2);
void BH_fdr(vector<double> &data);

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
    Array intensities;
    Array original_intensities;
    map<string, string> features;
    Matrix m;
    
    Table(string lipid_list_file) : file_name(lipid_list_file) {
        QFileInfo qFileInfo(file_name.c_str());
        cleaned_name = qFileInfo.baseName().toStdString();
    }
};


class ListItem : public QListWidgetItem {
public:
    LipidAdduct *species;
    ListItemType type;
    
    ListItem(QString name, ListItemType t, QListWidget* w);
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
    void set_step();
    void prepare_steps(int);
    
    
signals:
    void set_current(int);
    void set_max(int);
    void finish();
};


class DendrogramNode {
public:
    set<int> indexes;
    DendrogramNode* left_child;
    DendrogramNode* right_child;
    double distance;
    double x_left;
    double x_right;
    double y;
    map<string, map<string, int>> feature_count;
    
    DendrogramNode(int index, map<string, set<string>> *feature_values, Table *lipidome);
    DendrogramNode(DendrogramNode* n1, DendrogramNode* n2, double d);
    ~DendrogramNode();
    double* execute(int i, Array* points, vector<int>* sorted_ticks);
};


#endif /* ASSISTANCE_FUNCTIONS_H */
