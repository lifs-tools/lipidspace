
#ifndef STATISTICS_H
#define STATISTICS_H


#include <vector>
#include <map>
#include <math.h>
#include <QChartView>
#include <QChart>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPrinter>
#include <QBrush>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QMarginsF>
	#include <QtCharts/QChartView>
#endif
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBoxSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include "lipidspace/lipidspace.h"

using namespace std;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	using namespace QtCharts;
#endif

class Statistics : public QChartView {
    Q_OBJECT

public:
    LipidSpace *lipid_space;
    QChart *chart;
    
    Statistics(QWidget *parent = nullptr);
    void set_lipid_space(LipidSpace *_lipid_space);
    double median(vector<double> &lst, int begin, int end);
    
public slots:
    void updateChart();
    void exportAsPdf();
    void setLegendSize(int);
    void setTickSize(int);
};


#endif /* STATISTICS_H */
