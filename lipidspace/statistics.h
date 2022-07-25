
#ifndef STATISTICS_H
#define STATISTICS_H


#include <vector>
#include <map>
#include <math.h>

#include <QPrinter>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsLayout>

#include "lipidspace/lipidspace.h"
#include "lipidspace/chart.h"
#include "lipidspace/boxplot.h"
#include "lipidspace/scatterplot.h"
#include "lipidspace/lineplot.h"
#include "lipidspace/histogramplot.h"
#include "lipidspace/barplot.h"

using namespace std;
using namespace OpenXLSX;



class Statistics : public QObject {
    Q_OBJECT

public:
    LipidSpace *lipid_space;
    Chart *chart;
    vector<Array> series;
    vector<string> series_titles;
    map<string, double> stat_results;
    bool log_scale;
    bool show_data;

    Statistics();
    void load_data(LipidSpace *_lipid_space, Chart *_chart);


public slots:
    void updateBoxPlot();
    void updateROCCurve();
    void updateBarPlot();
    void updateHistogram();
    void exportAsPdf();
    void exportData();
    void setLegendSizeBoxPlot(int);
    void setLegendSizeBarPlot(int);
    void setLegendSizeHistogram(int);
    void setLegendSizeROCCurve(int);
    void setTickSizeBoxPlot(int);
    void setTickSizeBarPlot(int);
    void setTickSizeHistogram(int);
    void setTickSizeROCCurve(int);
    void setBarNumber(int);
    void set_log_scale();
    void set_show_data();
};


#endif /* STATISTICS_H */
