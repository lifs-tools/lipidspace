#ifndef MANAGELIPIDOMES_H
#define MANAGELIPIDOMES_H

#include <QDialog>

namespace Ui {
class ManageLipidomes;
}

class ManageLipidomes : public QDialog
{
    Q_OBJECT

public:
    explicit ManageLipidomes(QWidget *parent = nullptr);
    ~ManageLipidomes();

private:
    Ui::ManageLipidomes *ui;
};

#endif // MANAGELIPIDOMES_H
