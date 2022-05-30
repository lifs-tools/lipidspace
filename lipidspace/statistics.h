
#ifndef STATISTICS_H
#define STATISTICS_H


#include <vector>
#include <map>
#include <math.h>

#include <QFileDialog>
#include <QChartView>
#include <QChart>
#include <QMessageBox>
#include <QGraphicsLayout>
#include <QPrinter>
#include <QBrush>

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
    
    Statistics(QWidget *parent = nullptr);
    void set_lipid_space(LipidSpace *_lipid_space);
    double median(vector<double> &lst, int begin, int end);
    static double hyperg_2F1(double a, double b, double c, double d);
    static double t_distribution_cdf(double t_stat, double free_deg);
    static double f_distribution_cdf(double fi_stat, double df1, double df2);
    static double p_value_kolmogorov_smirnov(Array &sample1, Array &sample2);
    static double p_value_student(Array &a, Array &b);
    static double p_value_welch(Array &a, Array &b);
    static double p_value_anova(vector<Array> &v);
    
public slots:
    void updateBoxPlot();
    void updateHistogram();
    void exportAsPdf();
    void exportData();
    void setLegendSizeBoxPlot(int);
    void setLegendSizeHistogram(int);
    void setTickSizeBoxPlot(int);
    void setTickSizeHistogram(int);
    void setBarNumber(int);
};


#endif /* STATISTICS_H */
