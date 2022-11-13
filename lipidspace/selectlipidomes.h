#ifndef SELECTLIPIDOMES_H
#define SELECTLIPIDOMES_H

#include <QDialog>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <lipidspace/lipidspacegui.h>


namespace Ui {
    class SelectLipidomes;
}

class SelectLipidomes : public QDialog {
    Q_OBJECT

public:
    bool updating_states;
    map<QTreeWidgetItem*, int> item_to_canvas_position;

    explicit SelectLipidomes(QWidget *parent = nullptr);
    ~SelectLipidomes();

    void init();

public slots:
    void cancel();
    void ok();
    void itemChanged(QTreeWidgetItem *item, int column);

private:
    Ui::SelectLipidomes *ui;
};

#endif // SELECTLIPIDOMES_H
