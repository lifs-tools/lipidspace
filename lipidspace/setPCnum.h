#ifndef SETPCNUM_H
#define SETPCNUM_H

#include <QDialog>
#include "ui_setPCnum.h"
#include "lipidspace/lipidspacegui.h"
#include "lipidspace/lipidspace.h"

namespace Ui {
class SetPCnum;
}

class SetPCnum : public QDialog
{
    Q_OBJECT

public:    
    explicit SetPCnum(QWidget *parent = nullptr);
    ~SetPCnum();
    
    
public slots:
    void ok();
    void cancel();

private:
    Ui::SetPCnum *ui;
};

#endif // SETPCNUM_H
