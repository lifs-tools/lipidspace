#ifndef CHARTPLOT_H
#define CHARTPLOT_H

#include "lipidspace/chart.h"
#include <vector>

using namespace std;

class Chart;

class Chartplot : public QObject {
    Q_OBJECT

public:
    Chart *chart;

    Chartplot(Chart *_chart);
    virtual ~Chartplot();
    virtual void update_chart() = 0;
    virtual void clear() = 0;

public slots:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void resizeEvent();
};


#endif /* CHARTPLOT_H */

