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

class BarBox {
public:
    double value;
    double error;

    QGraphicsLineItem *upper_error_line;
    QGraphicsLineItem *lower_error_line;
    QGraphicsLineItem *base_line;
    HoverRectItem *rect;
    QColor color;
    vector<pair<double, double>> data;
    vector<QGraphicsEllipseItem*> dots;

    BarBox(QGraphicsScene *scene, double _value, double _error, QString _label, QColor _color = Qt::white, vector< pair<double, double> > *_data = 0);
};

class Barplot : public Chartplot {
public:
    vector< vector<BarBox> > bars;
    bool log_scale;
    bool show_data;

    Barplot(Chart *_chart, bool _log_scale = false, bool _show_data = false);
    ~Barplot();
    void add(vector< vector< Array > > &data, vector<QString> &categories, vector<QString> &labels, vector<QColor> *colors = 0);
    void update_chart();
    void clear();
};


#endif /* BOXPLOT_H */

