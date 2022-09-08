#ifndef PLOTS_H
#define PLOTS_H

#include "lipidspace/chartplot.h"
#include "lipidspace/AssistanceFunctions.h"
#include <QToolTip>
#include <QCursor>
#include <vector>
#include <QObject>

#define MAX_ZOOM 20.

class Chartplot;

using namespace std;


class HoverRectSignal : public QObject {
    Q_OBJECT

public:
    void sendSignalEnter(string lipid_name);
    void sendSignalExit();

signals:
    void enterLipid(string lipid_name);
    void exitLipid();
};










class HoverRectItem : public QGraphicsRectItem {

public:
    QString label;
    string lipid_name;
    HoverRectSignal hover_rect_signal;

    HoverRectItem(QString _label, string _lipid_name, QGraphicsItem *parent = nullptr);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

};




class DataCloud : public QGraphicsItem {

public:
    SortVector<double, double> data;
    Chart *chart;
    double x_pos;
    double x_offset;
    double y_pos;
    double y_offset;

    DataCloud(Chart *_chart, SortVector<double, double> *_data);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    void setNewPosition(double _x, double _x_offset, double _y_pos, double _y_offset);
};




class BarBox : public QObject {
    Q_OBJECT

public:
    double value;
    double error;

    QGraphicsLineItem *upper_error_line;
    QGraphicsLineItem *lower_error_line;
    QGraphicsLineItem *base_line;
    DataCloud *data_cloud;
    HoverRectItem *rect;
    QColor color;

    BarBox(Chart *_chart, double _value, double _error, QString _label, QColor _color, SortVector<double, double> *_data);


signals:
    void enterLipid(string lipid_name);
    void exitLipid();

public slots:
    void lipidEntered(string lipid_name);
    void lipidExited();

};


class Barplot : public Chartplot {
    Q_OBJECT

public:
    vector< vector<BarBox*> > bars;
    bool y_log_scale;
    bool show_data;
    double min_log_value;
    QPointF mouse_shift_start;
    QPointF shift_start;
    bool shifting;
    QThread loadingThread;
    int zoom;

    Barplot(Chart *_chart, bool _log_scale = false, bool _show_data = false);
    ~Barplot();
    void add(vector< vector< Array > > *_data, vector<QString> *categories, vector<QString> *labels, vector<QColor> *colors);
    void update_chart();
    void clear();


signals:
    void enterLipid(string lipid_name);
    void exitLipid();
    void startLoading(Barplot *barplot, vector< vector< Array > > *_data, vector<QString> *categories, vector<QString> *labels, vector<QColor> *colors);

public slots:
    void lipidEntered(string lipid_name);
    void lipidExited();
    void wheelEvent(QWheelEvent *event) override;
    void setYLogScale(bool log_scale);
    void setShowDataPoints(bool data_points);
    void mouseMoveEvent(QMouseEvent *event);
};












class WhiskerBox {
public:
    double lower_extreme;
    double lower_quartile;
    double median;
    double upper_quartile;
    double upper_extreme;

    QGraphicsLineItem *upper_extreme_line;
    QGraphicsLineItem *lower_extreme_line;
    QGraphicsLineItem *median_line;
    QGraphicsLineItem *base_line;
    QGraphicsRectItem *rect;
    DataCloud *data_cloud;
    QColor color;

    WhiskerBox(Chart *chart, SortVector<double, double> *data);
};


class Boxplot : public Chartplot {
    Q_OBJECT

public:
    bool show_data;
    vector<WhiskerBox> boxes;

    Boxplot(Chart *_chart, bool _show_data = false);
    ~Boxplot();
    static double median(vector<double> &lst, int begin, int end);
    void add(Array &data, QString category, QColor color = Qt::white);
    void update_chart();
    void clear();

public slots:
    void setShowDataPoints(bool data_points);
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
