#ifndef SETALPHA_H
#define SETALPHA_H

#include <QDialog>
#include "ui_setalpha.h"
#include "lipidspace/lipidspacegui.h"

namespace Ui {
class SetAlpha;
}

class SetAlpha : public QDialog
{
    Q_OBJECT

public:
    explicit SetAlpha(QWidget *parent = nullptr);
    ~SetAlpha();
    
    
public slots:
    void ok();
    void cancel();

private:
    Ui::SetAlpha *ui;
};

#endif // SETALPHA_H
