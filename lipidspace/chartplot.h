#ifndef CHARTPLOT_H
#define CHARTPLOT_H

#include "lipidspace/chart.h"
#include <vector>

enum PlotType {BOX_PLOT, NO_PLOT};

using namespace std;

class Chart;

class Chartplot {
public:
    Chart *chart;
    PlotType plot_type;

    Chartplot(Chart *_chart) : chart(_chart) {
        plot_type = NO_PLOT;
    }
    virtual ~Chartplot(){}
    virtual void update_chart(){};
    virtual void clear(){};
};


#endif /* CHARTPLOT_H */

