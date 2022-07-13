#ifndef LINEPLOT_H
#define LINEPLOT_H

class Chartplot;

#include "lipidspace/chartplot.h"
#include <QGraphicsLineItem>
#include <vector>

using namespace std;


struct Line {
    double x1;
    double y1;
    double x2;
    double y2;
    QGraphicsLineItem* line;
    QColor color;

    Line(double _x1, double _y1, double _x2, double _y2, QColor _color = QColor("#99ca53")){
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
        color = _color;
        line = new QGraphicsLineItem();
    }
};


class Lineplot : public Chartplot {
public:
    vector<Line> lines;

    Lineplot(Chart *_chart);
    ~Lineplot();
    void add(vector< pair< pair<double, double>, pair<double, double> > > &lines, QString category, QColor color = QColor("#99ca53"));
    void update_chart();
    void clear();
};


#endif /* SCATTERPLOT_H */

