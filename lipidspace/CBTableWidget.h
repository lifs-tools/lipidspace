#ifndef CBTABLEWIDGET_H
#define CBTABLEWIDGET_H

#include <QTableWidget>
#include <QAbstractButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <lipidspace/globaldata.h>

class CBTableWidget : public QTableWidget {
    Q_OBJECT

public:
    QLabel *transpose_label;

    explicit CBTableWidget(QWidget *parent = 0);
    void wheelEvent(QWheelEvent*) override;

signals:
    void cornerButtonClick();
    void zooming();


private slots:
    void cornerButtonClicked();
};

#endif // CBTABLEWIDGET_H
