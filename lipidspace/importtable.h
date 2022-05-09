#ifndef IMPORTTABLE_H
#define IMPORTTABLE_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/logging.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/select_worksheet.h"
#include <string>
#include <map>
#include <vector>
#include "ui_importtable.h"

using namespace std;

namespace Ui {
class ImportTable;
}

class ImportTable : public QDialog
{
    Q_OBJECT

public:
    explicit ImportTable(QWidget *parent = nullptr);
    ~ImportTable();
    
signals:
    void importTable(string file_name, vector<TableColumnType>* column_types, TableType table_type);
    
public slots:
    void okRow();
    void okCol();
    void okFlat();
    void cancel();
    void oneItemViolated(string field_name, int num);
    
private:
    Ui::ImportTable *ui;
    string data_table_file;
    map<QString, int> original_column_index;
};

#endif // IMPORTTABLE_H
