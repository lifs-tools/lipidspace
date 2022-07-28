#ifndef HISTOGRAMPLOT_H
#define HISTOGRAMPLOT_H

#include "lipidspace/chartplot.h"
#include <vector>

class Chartplot;

using namespace std;

struct HistogramBox {
    double x;
    double x_width;
    double y;

    QGraphicsRectItem *rect;
    QColor color;

    HistogramBox(QGraphicsScene *scene, double _x, double _x_width, double _y, QColor _color = Qt::red, bool transparent = true){
        x = _x;
        x_width = _x_width;
        y = _y;
        color = _color;
        if (transparent) color.setAlpha(128);

        rect = new QGraphicsRectItem();
        scene->addItem(rect);
    }
};

class Histogramplot : public Chartplot {
public:
    vector<HistogramBox> boxes;

    Histogramplot(Chart *_chart);
    ~Histogramplot();
    void add(vector<Array> &arrays, vector<QString> &categories, vector<QColor> *colors = 0, uint num_bars = 20);
    void update_chart();
    void clear();
};


#endif /* HISTOGRAMPLOT_H */
