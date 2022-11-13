#ifndef SELECTLIPIDOMES_H
#define SELECTLIPIDOMES_H

#include <QDialog>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <lipidspace/lipidspacegui.h>

/*
class SelectItem : public QTreeWidgetItem {
    Q_OBJECT
public:
    childGetUpdated(Qt::CheckState);
    parentGetUpdated(Qt::CheckState);

signals():
    updateParent(Qt::CheckState);
    updateChildren(Qt::CheckState);
};
*/

namespace Ui {
    class SelectLipidomes;
}

class SelectLipidomes : public QDialog {
    Q_OBJECT

public:
    bool updating_states;

    explicit SelectLipidomes(QWidget *parent = nullptr);
    ~SelectLipidomes();

    void init();

public slots:
    void cancel();
    void ok();
    void foo(QTreeWidgetItem *item, int column);

private:
    Ui::SelectLipidomes *ui;
};

#endif // SELECTLIPIDOMES_H
