#ifndef SELECTLIPIDOMES_H
#define SELECTLIPIDOMES_H

#include <QDialog>
#include <QListWidgetItem>
#include <lipidspace/lipidspacegui.h>

namespace Ui {
    class SelectLipidomes;
}

class SelectLipidomes : public QDialog {
    Q_OBJECT

public:
    explicit SelectLipidomes(QWidget *parent = nullptr);
    ~SelectLipidomes();

    void init();

public slots:
    void cancel();
    void ok();

private:
    Ui::SelectLipidomes *ui;
};

#endif // SELECTLIPIDOMES_H
