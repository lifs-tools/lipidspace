#ifndef CBTABLEWIDGET_H
#define CBTABLEWIDGET_H

#include <QTableWidget>
#include <QAbstractButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>


class CBTableWidget : public QTableWidget {
    Q_OBJECT
    
public:
    explicit CBTableWidget(QWidget *parent = 0);
    
signals:
    void cornerButtonClick();
    
    
private slots:
    void cornerButtonClicked();
};

#endif // CBTABLEWIDGET_H
