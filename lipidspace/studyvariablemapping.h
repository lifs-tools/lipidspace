#ifndef STUDYVARIABLEMAPPING_H
#define STUDYVARIABLEMAPPING_H

#include <QDialog>
#include "ui_studyvariablemapping.h"
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/lipidspace.h"
#include <vector>
#include <map>
#include <set>
#include <QComboBox>
#include <QLineEdit>

using namespace std;

namespace Ui {
    class StudyVariableMapping;
}





class StudyVariableMapping : public QDialog {
    Q_OBJECT

public:
    FileTableHandler *file_table_handler;
    LipidSpace *lipid_space;
    vector<TableColumnType> *column_types;
    MappingData *mapping_data;
    map<int, set<int>> variable_to_values;
    map<int, int> value_to_variable;
    vector<string> names_for_registration;

    explicit StudyVariableMapping(FileTableHandler *fth, MappingData *md, vector<TableColumnType> *ct, LipidSpace *ls, QWidget *parent = nullptr);
    ~StudyVariableMapping();

private:
    Ui::StudyVariableMapping *ui;

public slots:
    void doContinue();
    void cancel();
    void comboChanged(SignalCombobox *sc);
    void lineEditChanged(SignalLineEdit *edit, QString txt);
};

#endif // STUDYVARIABLEMAPPING_H
