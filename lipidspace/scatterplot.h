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
    QGraphicsEllipseItem* p;

    ScPoint(double _x, double _y){
        x = _x;
        y = _y;
        p = new QGraphicsEllipseItem();
    }
};


class Scatterplot : public Chartplot {
public:
    QColor color;
    vector<ScPoint> points;

    Scatterplot(Chart *_chart);
    ~Scatterplot();
    void add(vector< pair<double, double> > &data, QString category, QColor color = Qt::white);
    void update_chart();
    void clear();
};


#endif /* SCATTERPLOT_H */

