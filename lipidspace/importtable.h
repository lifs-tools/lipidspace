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
#include "lipidspace/studyvariablemapping.h"
#include <string>
#include <map>
#include <vector>
#include "ui_importtable.h"
#include "lipidspace/lipidspace.h"

using namespace std;

namespace Ui {
    class ImportTable;
}

class ImportTable : public QDialog
{
    Q_OBJECT

public:
    string sheet;
    Ui::ImportTable *ui;
    string data_table_file;
    QString file_name;
    LipidSpace *lipid_space;
    map<QString, int> original_column_index;
    bool mapping_of_study_variables;
    FileTableHandler *file_table_handler;

    explicit ImportTable(QWidget *parent = nullptr);
    ~ImportTable();
    void show(LipidSpace *_lipid_space);
    bool checkStudyVariables();

signals:
    void importTable(ImportData *import_data);
    void importOpened();

public slots:
    void okRow();
    void okCol();
    void okFlat();
    void cancel();
    void oneItemViolated(string field_name, int num);
    void checkBoxCol(int state);
    void checkBoxFlat(int state);

};

#endif // IMPORTTABLE_H
