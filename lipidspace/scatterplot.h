#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

class Chartplot;

#include "lipidspace/chartplot.h"
#include <QGraphicsEllipseItem>
#include <vector>
#include <map>

using namespace std;


struct ScPoint {
    double x;
    double y;
    QColor color;
    QGraphicsEllipseItem* p;

    ScPoint(double _x, double _y, QColor _color = QColor("#209fdf")){
        x = _x;
        y = _y;
        color = _color;
        p = new QGraphicsEllipseItem();
    }
};


class Scatterplot : public Chartplot {
public:
    vector<ScPoint> points;

    Scatterplot(Chart *_chart);
    ~Scatterplot();
    void add(vector< pair<double, double> > &data, QString category, QColor color = QColor("#209fdf"));
    void update_chart();
    void clear();
};


#endif /* SCATTERPLOT_H */

