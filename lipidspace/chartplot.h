#ifndef CHARTPLOT_H
#define CHARTPLOT_H

#include "lipidspace/chart.h"
#include <vector>

using namespace std;

class Chart;

class Chartplot {
public:
    Chart *chart;

    Chartplot(Chart *_chart) : chart(_chart) {}
    virtual ~Chartplot(){}
    virtual void update_chart(){};
    virtual void clear(){};
};


#endif /* CHARTPLOT_H */

