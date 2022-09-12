
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
#include "lipidspace/plots.h"

using namespace std;
using namespace OpenXLSX;

enum SecondaryType {NoSecondary, NominalSecondary, NumericalSecondary};

class Statistics : public QObject {
    Q_OBJECT

public:
    LipidSpace *lipid_space;
    Chart *chart;
    vector<Array> series;
    vector<string> series_titles;
    map<string, vector<QVariant> > flat_data;
    vector< pair<string, double> > stat_results;
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
    void setLegendSize(int);
    void setTickSize(int);
    void setBarNumber(int);
    void setLogScaleBarPlot();
    void setShowDataBoxPlot();
    void setShowDataBarPlot();
    void lipidEntered(string lipid_name);
    void lipidExited();

signals:
    void enterLipid(string lipid_name);
    void exitLipid();
};


#endif /* STATISTICS_H */
