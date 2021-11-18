#ifndef IMPORTDATATABLE_H
#define IMPORTDATATABLE_H

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
#include "lipidspace/AssistanceFunctions.h"

using namespace std;

namespace Ui {
class ImportDataTable;
}

class ImportDataTable : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDataTable(QWidget *parent = nullptr);
    ~ImportDataTable();
    
signals:
    void importTable(string file_name, vector<TableColumnType>* column_types);
    
public slots:
    void ok();
    void cancel();
    void updateTables(int i);

private:
    Ui::ImportDataTable *ui;
    QString measurements_column;
    string data_table_file;
    map<QString, int> original_column_index;
};

#endif // IMPORTDATATABLE_H
