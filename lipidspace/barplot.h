#ifndef BARPLOT_H
#define BARPLOT_H

#include "lipidspace/chartplot.h"
#include <vector>

class Chartplot;

using namespace std;

struct BarBox {
    double value;
    double error;

    QGraphicsLineItem *upper_error_line;
    QGraphicsLineItem *lower_error_line;
    QGraphicsLineItem *base_line;
    QGraphicsRectItem *rect;
    QColor color;

    BarBox(QGraphicsScene *scene, double _value, double _error, QColor _color = Qt::white){
        value = _value;
        error = _error;
        color = _color;

        upper_error_line = new QGraphicsLineItem();
        lower_error_line = new QGraphicsLineItem();
        base_line = new QGraphicsLineItem();
        rect = new QGraphicsRectItem();

        scene->addItem(upper_error_line);
        scene->addItem(lower_error_line);
        scene->addItem(base_line);
        scene->addItem(rect);
    }
};

class Barplot : public Chartplot {
public:
    vector< vector<BarBox> > bars;

    Barplot(Chart *_chart);
    ~Barplot();
    void add(vector< vector< pair<double, double> > > &data, vector<QString> &categories, vector<QString> &labels, vector<QColor> *colors = 0);
    void update_chart();
    void clear();
};


#endif /* BOXPLOT_H */

