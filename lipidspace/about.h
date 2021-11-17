#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include "ui_about.h"
#include <fstream>
#include "lipidspace/logging.h"

namespace Ui {
class About;
}

class About : public QDialog {
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr, bool log_view = false);
    ~About();
    
public slots:
    void ok();

private:
    Ui::About *ui;
};

#endif // ABOUT_H
