#ifndef SELECTPC_H
#define SELECTPC_H

#include <QDialog>

namespace Ui {
class SelectPC;
}

class SelectPC : public QDialog
{
    Q_OBJECT

public:
    explicit SelectPC(QWidget *parent = nullptr);
    ~SelectPC();

private:
    Ui::SelectPC *ui;
};

#endif // SELECTPC_H
