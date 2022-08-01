#ifndef STUDYVARIABLEMAPPING_H
#define STUDYVARIABLEMAPPING_H

#include <QDialog>
#include "ui_studyvariablemapping.h"
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/lipidspace.h"
#include <vector>
#include <map>
#include <set>

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

    explicit StudyVariableMapping(FileTableHandler *fth, vector<TableColumnType> *ct, LipidSpace *ls, QWidget *parent = nullptr);
    ~StudyVariableMapping();

private:
    Ui::StudyVariableMapping *ui;

public slots:
    void doContinue();
    void cancel();
};

#endif // STUDYVARIABLEMAPPING_H
