
#ifndef STATISTICS_H
#define STATISTICS_H


#include <vector>
#include <map>
#include <math.h>

#include <QFileDialog>
#include <QChartView>
#include <QToolTip>
#include <QChart>
#include <QMessageBox>
#include <QGraphicsLayout>
#include <QPrinter>
#include <QBrush>
#include <QLogValueAxis>
#include <QBarCategoryAxis>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QMarginsF>
	#include <QtCharts/QChartView>
#endif
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBoxSet>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include "lipidspace/lipidspace.h"

using namespace std;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	using namespace QtCharts;
#endif
    
    
using namespace OpenXLSX;



class Statistics : public QChartView {
    Q_OBJECT

public:
    LipidSpace *lipid_space;
    QChart *chart;
    vector<Array> series;
    vector<string> series_titles;
    map<string, double> stat_results;
    bool log_scale;
    
    Statistics(QWidget *parent = nullptr);
    void set_lipid_space(LipidSpace *_lipid_space);
    double median(vector<double> &lst, int begin, int end);
    
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
    void bar_plot_hovered(bool, QBoxSet*);
};


#endif /* STATISTICS_H */
