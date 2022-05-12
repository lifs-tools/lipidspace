
#ifndef STATISTICS_H
#define STATISTICS_H


#include <vector>
#include <map>
#include <math.h>
#include <QChartView>
#include <QChart>
#include <QBrush>
#include <QtCharts/QChartView>
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QBoxSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include "lipidspace/lipidspace.h"

using namespace std;
using namespace QtCharts;

class Statistics : public QChartView {
    Q_OBJECT

public:
    LipidSpace *lipid_space;
    QChart *chart;
    
    Statistics(QWidget *parent = nullptr);
    void set_lipid_space(LipidSpace *_lipid_space);
    
public slots:
    void updateChart();
};


#endif /* STATISTICS_H */
