#ifndef SELECT_WORKSHEET_H
#define SELECT_WORKSHEET_H

#include <QDialog>
#include "lipidspace/lipidspace.h"
#include "ui_select_worksheet.h"
#include <string>
#include <OpenXLSX.hpp>


namespace Ui {
class SelectWorksheet;
}

class SelectWorksheet : public QDialog
{
    Q_OBJECT

public:
    string &sheet;
    explicit SelectWorksheet(string, string &, QWidget * = nullptr);
    ~SelectWorksheet();
    
signals:
    void reloadPoints();

private:
    Ui::SelectWorksheet *ui;
    
public slots:
    void ok();
    void cancel();
    
};

#endif // SELECT_WORKSHEET_H
