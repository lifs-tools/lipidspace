#ifndef ASSISTANCE_FUNCTIONS_H
#define ASSISTANCE_FUNCTIONS_H

#include <QtCore>
#include <set>
#include <vector>
#include <string>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/Matrix.h"

using namespace std;

enum Linkage {SINGLE, COMPLETE};

class Table {
public:
    string file_name;
    vector<string> species;
    vector<string> classes;
    vector<LipidAdduct*> lipids;
    Array intensities;
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


class Node {
public:
    set<int> indexes;
    Node* left_child;
    Node* right_child;
    double distance;
    
    Node(int index);
    Node(Node* n1, Node* n2, double d);
    ~Node();
    double* execute(int i, Array* points, vector<int>* sorted_ticks);
};


#endif /* ASSISTANCE_FUNCTIONS_H */
