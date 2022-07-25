#ifndef BARPLOT_H
#define BARPLOT_H

#include "lipidspace/chartplot.h"
#include <QToolTip>
#include <QCursor>
#include <vector>

class Chartplot;

using namespace std;

class HoverRectItem : public QGraphicsRectItem {
public:
    QString label;

    HoverRectItem(QString _label, QGraphicsItem *parent = nullptr);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

struct BarBox {
    double value;
    double error;

    QGraphicsLineItem *upper_error_line;
    QGraphicsLineItem *lower_error_line;
    QGraphicsLineItem *base_line;
    HoverRectItem *rect;
    QColor color;

    BarBox(QGraphicsScene *scene, double _value, double _error, QString _label, QColor _color = Qt::white){
        value = _value;
        error = _error;
        color = _color;

        upper_error_line = new QGraphicsLineItem();
        lower_error_line = new QGraphicsLineItem();
        base_line = new QGraphicsLineItem();
        rect = new HoverRectItem(QString("%1\n%2 ± %3").arg(_label).arg(value, 0, 'f', 1).arg(error, 0, 'f', 1));
        rect->setAcceptHoverEvents(true);

        scene->addItem(upper_error_line);
        scene->addItem(lower_error_line);
        scene->addItem(base_line);
        scene->addItem(rect);
    }
};

class Barplot : public Chartplot {
public:
    vector< vector<BarBox> > bars;
    bool log_scale;

    Barplot(Chart *_chart, bool _log_scale);
    ~Barplot();
    void add(vector< vector< pair<double, double> > > &data, vector<QString> &categories, vector<QString> &labels, vector<QColor> *colors = 0);
    void update_chart();
    void clear();
};


#endif /* BOXPLOT_H */

