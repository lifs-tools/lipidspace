#ifndef IMPORTPIVOTTABLE_H
#define IMPORTPIVOTTABLE_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_importdatatable.h"
#include <fstream>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/logging.h"
#include <string>
#include <map>
#include <vector>

using namespace std;

namespace Ui {
class ImportPivotTable;
}

class ImportPivotTable : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPivotTable(QWidget *parent = nullptr);
    ~ImportPivotTable();
    
signals:
    void importTable(string file_name, vector<TableColumnType>* column_types);
    
public slots:
    void ok();
    void cancel();
    void oneItemViolated(string field_name, int num);
    
private:
    Ui::ImportPivotTable *ui;
    string data_table_file;
    map<QString, int> original_column_index;
};

#endif // IMPORTPIVOTTABLE_H
