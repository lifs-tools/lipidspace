#ifndef MANAGELIPIDOMES_H
#define MANAGELIPIDOMES_H

#include <QDialog>
#include "lipidspace/lipidspace.h"
#include <QFileInfo>
#include "ui_managelipidomes.h"

namespace Ui {
class ManageLipidomes;
}

class ManageLipidomes : public QDialog
{
    Q_OBJECT

public:
    explicit ManageLipidomes(LipidSpace *_lipidSpace, QWidget *parent = nullptr);
    ~ManageLipidomes();
    
    
public slots:
    void removeSelected();
    void removeAll();
    void ok();
    void cancel();

private:
    Ui::ManageLipidomes *ui;
    LipidSpace *lipidSpace;
};

#endif // MANAGELIPIDOMES_H
