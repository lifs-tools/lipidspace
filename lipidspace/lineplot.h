#ifndef LINEPLOT_H
#define LINEPLOT_H

class Chartplot;

#include "lipidspace/chartplot.h"
#include <QGraphicsLineItem>
#include <vector>

using namespace std;


class LPLine {
public:
    double x1;
    double y1;
    double x2;
    double y2;
    QGraphicsLineItem* line;
    QColor color;

    LPLine(double _x1, double _y1, double _x2, double _y2, QColor _color = QColor("#99ca53"));
};


class Lineplot : public Chartplot {
public:
    vector<LPLine> lines;

    Lineplot(Chart *_chart);
    ~Lineplot();
    void add(vector< pair< pair<double, double>, pair<double, double> > > &lines, QString category, QColor color = QColor("#99ca53"));
    void update_chart();
    void clear();
};


#endif /* SCATTERPLOT_H */

