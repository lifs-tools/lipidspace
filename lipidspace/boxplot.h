#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "lipidspace/chartplot.h"
#include <vector>

class Chartplot;

using namespace std;

struct WhiskerBox {
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

    WhiskerBox(QGraphicsScene *scene){
        upper_extreme_line = new QGraphicsLineItem();
        lower_extreme_line = new QGraphicsLineItem();
        median_line = new QGraphicsLineItem();
        base_line = new QGraphicsLineItem();
        rect = new QGraphicsRectItem();

        scene->addItem(upper_extreme_line);
        scene->addItem(lower_extreme_line);
        scene->addItem(median_line);
        scene->addItem(base_line);
        scene->addItem(rect);
        color = Qt::white;
    }
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

