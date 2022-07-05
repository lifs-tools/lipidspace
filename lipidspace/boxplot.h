#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "lipidspace/chart.h"
#include <vector>

using namespace std;

struct WhiskerBox {
    double lower_extreme;
    double lower_quartile;
    double median;
    double upper_quartile;
    double upper_extreme;

    QGraphicsLineItem *upper_extreme_line;
    QGraphicsLineItem *lower_extreme_line;
    QGraphicsLineItem *median_line;
    QGraphicsLineItem *base_line;
    QGraphicsRectItem *rect;

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
    }
};

class Boxplot : public Chart {
public:
    vector<string> categories;
    vector<WhiskerBox> boxes;

    Boxplot(QWidget *parent = nullptr);
    void add(Array &data, string category);
    void update_chart();
    void clear();
};


#endif /* BOXPLOT_H */

