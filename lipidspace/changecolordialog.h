#ifndef CHANGECOLORDIALOG_H
#define CHANGECOLORDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QColorDialog>

#include "lipidspace/lipidspace.h"
#include "lipidspace/lipidspacegui.h"
#include "lipidspace/globaldata.h"

#include <map>

namespace Ui {
class ChangeColorDialog;
}



class ChangeColorDialog : public QDialog
{
    Q_OBJECT

public:
    LipidSpace *lipid_space;
    map<QTreeWidgetItem*, QColor*> items;

    explicit ChangeColorDialog(QWidget *parent = nullptr);
    ~ChangeColorDialog();

private:
    Ui::ChangeColorDialog *ui;

public slots:
    void cancel();
    void ok();
    void doubleClicked(QTreeWidgetItem *item, int column);
};

#endif // CHANGECOLORDIALOG_H
