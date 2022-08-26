#ifndef CHARTPLOT_H
#define CHARTPLOT_H

#include "lipidspace/chart.h"
#include <vector>

using namespace std;

class Chart;

class Chartplot : public QObject {

public:
    Chart *chart;

    Chartplot(Chart *_chart);
    virtual ~Chartplot();
    virtual void update_chart() = 0;
    virtual void clear() = 0;
};


#endif /* CHARTPLOT_H */

