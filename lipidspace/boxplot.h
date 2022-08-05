#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "lipidspace/chartplot.h"
#include <vector>

class Chartplot;

using namespace std;

class WhiskerBox {
public:
    double lower_extreme;
    double lower_quartile;
    double median;
    double upper_quartile;
    double upper_extreme;
    vector<pair<double, double>> data;

    QGraphicsLineItem *upper_extreme_line;
    QGraphicsLineItem *lower_extreme_line;
    QGraphicsLineItem *median_line;
    QGraphicsLineItem *base_line;
    QGraphicsRectItem *rect;
    vector<QGraphicsEllipseItem*> dots;
    QColor color;

    WhiskerBox(QGraphicsScene *scene);
};


class Boxplot : public Chartplot {
public:
    bool show_data;
    vector<WhiskerBox> boxes;

    Boxplot(Chart *_chart, bool _show_data = false);
    ~Boxplot();
    static double median(vector<double> &lst, int begin, int end);
    void add(Array &data, QString category, QColor color = Qt::white);
    void update_chart();
    void clear();
};


#endif /* BOXPLOT_H */

