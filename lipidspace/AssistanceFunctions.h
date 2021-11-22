#ifndef ASSISTANCE_FUNCTIONS_H
#define ASSISTANCE_FUNCTIONS_H

#include <QtCore>
#include <QListWidget>
#include <QDropEvent>
#include <set>
#include <vector>
#include <string>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/Matrix.h"
#include <iostream>

using namespace std;

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


using namespace std;

enum Linkage {SINGLE, COMPLETE};
enum TableColumnType {SampleColumn, QuantColumn, LipidColumn, FeatureColumn, IgnoreColumn};

class Table {
public:
    string file_name;
    vector<string> species;
    vector<string> classes;
    vector<LipidAdduct*> lipids;
    Array intensities;
    Array original_intensities;
    map<string, string> features;
    Matrix m;
    
    Table(string lipid_list_file) : file_name(lipid_list_file) {}
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
