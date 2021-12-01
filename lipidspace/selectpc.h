#ifndef SELECTPC_H
#define SELECTPC_H

#include <QDialog>
#include "lipidspace/lipidspace.h"
#include "lipidspace/globaldata.h"
#include "ui_selectpc.h"

namespace Ui {
class SelectPC;
}

class SelectPC : public QDialog
{
    Q_OBJECT

public:
    explicit SelectPC(QWidget *parent = nullptr);
    ~SelectPC();
    
signals:
    void reloadPoints();

private:
    Ui::SelectPC *ui;
    
public slots:
    void ok();
    void cancel();
    
};

#endif // SELECTPC_H
