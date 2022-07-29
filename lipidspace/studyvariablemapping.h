#ifndef STUDYVARIABLEMAPPING_H
#define STUDYVARIABLEMAPPING_H

#include <QDialog>

namespace Ui {
class StudyVariableMapping;
}

class StudyVariableMapping : public QDialog
{
    Q_OBJECT

public:
    explicit StudyVariableMapping(QWidget *parent = nullptr);
    ~StudyVariableMapping();

private:
    Ui::StudyVariableMapping *ui;
};

#endif // STUDYVARIABLEMAPPING_H
