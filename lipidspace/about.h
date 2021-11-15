#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include "ui_about.h"
#include <fstream>

namespace Ui {
class About;
}

class About : public QDialog {
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();
    
public slots:
    void ok();

private:
    Ui::About *ui;
};

#endif // ABOUT_H
