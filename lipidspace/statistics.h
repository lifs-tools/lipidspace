
#ifndef STATISTICS_H
#define STATISTICS_H


#include <vector>
#include <map>
#include <math.h>

#include <QPrinter>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsLayout>
#include <QSvgGenerator>

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
    bool show_pvalues;
    map<string, vector<string>> volcano_data;
    void clean();

    Statistics();
    void load_data(LipidSpace *_lipid_space, Chart *_chart);


public slots:
    void updateBoxPlot();
    void updateFAD();
    void updateROCCurve();
    void updateBarPlot();
    void updateBarPlotClasses();
    void updateHistogram();
    void updateSpeciesCV();
    void updatePCA();
    void updatePVal();
    void updateVolcano();
    void updateEnrichment();
    void exportAsPdf();
    void exportAsSvg();
    void exportData();
    void setLegendSize(int);
    void setTickSize(int);
    void setBarNumberHistogram(int);
    void setBarNumberSpeciesCV(int);
    void setBarNumberPvalues(int bar_number);
    void setLogScaleBarPlot();
    void setShowDataBoxPlot();
    void setShowDataBarPlot();
    void lipidEntered(string lipid_name);
    void lipidExited();
    void setStatResults();
    void highlightPoints(QListWidget*);
    void highlightBars(QListWidget* speciesList);
    void lipidMarked();
    void lipidsMarked(set<string> *lipids);

signals:
    void enterLipid(string lipid_name);
    void exitLipid();
    void markLipid();
    void markLipids(set<string> *lipids);
    void createdEnrichmentList();
};


#endif /* STATISTICS_H */
