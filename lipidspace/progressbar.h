#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QDialog>
#include "ui_progressbar.h"
#include <iostream>

using namespace std;

namespace Ui {
class Progressbar;
}

class Progressbar : public QDialog
{
    Q_OBJECT

public:
    explicit Progressbar(QWidget *parent = nullptr);
    ~Progressbar();
    
public slots:
    void finish();
    void set_max(int m);
    void set_current(int c);
    void abort();
    void reject();
    
signals:
    void resetAnalysis();
    void interrupt();
    void refreshCanvas();

private:
    Ui::Progressbar *ui;
    void closeWindow(bool);
};

#endif // PROGRESSBAR_H
