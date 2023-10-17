#ifndef MANAGELIPIDOMES_H
#define MANAGELIPIDOMES_H

#include <QDialog>
#include "lipidspace/lipidspace.h"
#include "lipidspace/lipidspacegui.h"
#include <QFileInfo>
#include "ui_managelipidomes.h"
#include <vector>
#include <map>
#include <algorithm>

namespace Ui {
class ManageLipidomes;
}

class ManageLipidomes : public QDialog
{
    Q_OBJECT

public:
    explicit ManageLipidomes(QWidget *parent = nullptr);
    ~ManageLipidomes();

signals:
    void runAnalysis();
    void resetAnalysis();
    void reassembleSelection();


public slots:
    void removeSelected();
    void removeAll();
    void ok();
    void cancel();

private:
    Ui::ManageLipidomes *ui;
    LipidSpace *lipid_space;
    vector<int> removeItems;
    map<QString, int> itemIndex;
};

#endif // MANAGELIPIDOMES_H
