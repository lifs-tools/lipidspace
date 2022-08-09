#ifndef PLOTS_H
#define PLOTS_H

#include "lipidspace/chartplot.h"
#include <QToolTip>
#include <QCursor>
#include <vector>

class Chartplot;

using namespace std;




class HoverRectItem : public QGraphicsRectItem {
public:
    QString label;

    HoverRectItem(QString _label, QGraphicsItem *parent = nullptr);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

class BarBox {
public:
    double value;
    double error;

    QGraphicsLineItem *upper_error_line;
    QGraphicsLineItem *lower_error_line;
    QGraphicsLineItem *base_line;
    HoverRectItem *rect;
    QColor color;
    vector<pair<double, double>> data;
    vector<QGraphicsEllipseItem*> dots;

    BarBox(QGraphicsScene *scene, double _value, double _error, QString _label, QColor _color = Qt::white, vector< pair<double, double> > *_data = 0);
};

class Barplot : public Chartplot {
public:
    vector< vector<BarBox> > bars;
    bool log_scale;
    bool show_data;

    Barplot(Chart *_chart, bool _log_scale = false, bool _show_data = false);
    ~Barplot();
    void add(vector< vector< Array > > &data, vector<QString> &categories, vector<QString> &labels, vector<QColor> *colors = 0);
    void update_chart();
    void clear();
};



class WhiskerBox {
public:
    double lower_extreme;
    double lower_quartile;
    double median;
    double upper_quartile;
    double upper_extreme;
    vector<pair<double, double>> data;

    QGraphicsLineItem *upper_extreme_line;
    QGraphicsLineItem *lower_extreme_line;
    QGraphicsLineItem *median_line;
    QGraphicsLineItem *base_line;
    QGraphicsRectItem *rect;
    vector<QGraphicsEllipseItem*> dots;
    QColor color;

    WhiskerBox(QGraphicsScene *scene);
};


class Boxplot : public Chartplot {
public:
    bool show_data;
    vector<WhiskerBox> boxes;

    Boxplot(Chart *_chart, bool _show_data = false);
    ~Boxplot();
    static double median(vector<double> &lst, int begin, int end);
    void add(Array &data, QString category, QColor color = Qt::white);
    void update_chart();
    void clear();
};






class LPLine {
public:
    double x1;
    double y1;
    double x2;
    double y2;
    QGraphicsLineItem* line;
    QColor color;

    LPLine(double _x1, double _y1, double _x2, double _y2, QColor _color = QColor("#99ca53"));
};


class Lineplot : public Chartplot {
public:
    vector<LPLine> lines;

    Lineplot(Chart *_chart);
    ~Lineplot();
    void add(vector< pair< pair<double, double>, pair<double, double> > > &lines, QString category, QColor color = QColor("#99ca53"));
    void update_chart();
    void clear();
};





class ScPoint {
public:
    double x;
    double y;
    QColor color;
    QGraphicsEllipseItem* p;

    ScPoint(double _x, double _y, QColor _color = QColor("#209fdf"));
};


class Scatterplot : public Chartplot {
public:
    vector<ScPoint> points;
    QGraphicsRectItem *base;

    Scatterplot(Chart *_chart);
    ~Scatterplot();
    void add(vector< pair<double, double> > &data, QString category, QColor color = QColor("#209fdf"));
    void update_chart();
    void clear();
};





class HistogramBox {
public:
    double x;
    double x_width;
    double y;
    QGraphicsRectItem *rect;
    QColor color;

    HistogramBox(QGraphicsScene *scene, double _x, double _x_width, double _y, QColor _color = Qt::red, bool transparent = true);
};

class Histogramplot : public Chartplot {
public:
    vector<HistogramBox> boxes;

    Histogramplot(Chart *_chart);
    ~Histogramplot();
    void add(vector<Array> &arrays, vector<QString> &categories, vector<QColor> *colors = 0, uint num_bars = 20);
    void update_chart();
    void clear();
};

#endif /* PLOTS_H */
