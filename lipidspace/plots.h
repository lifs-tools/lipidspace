#ifndef PLOTS_H
#define PLOTS_H

#include "lipidspace/chartplot.h"
#include "lipidspace/AssistanceFunctions.h"
#include <QToolTip>
#include <QCursor>
#include <vector>
#include <QObject>
#include <QGraphicsSceneMouseEvent>

#define MAX_ZOOM 20.

class Chartplot;

using namespace std;


class HoverSignal : public QObject {
    Q_OBJECT

public:
    void sendSignalEnter(string lipid_name);
    void sendSignalExit();

signals:
    void enterLipid(string lipid_name);
    void exitLipid();
    void markLipid();
};







class HoverRectItem : public QGraphicsRectItem {

public:
    QString label;
    string lipid_name;
    HoverSignal hover_signal;

    HoverRectItem(QString _label, string _lipid_name, QGraphicsItem *parent = nullptr);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

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
    bool highlight = false;

    BarBox(Chart *_chart, double _value, double _error, QString _label, QColor _color, SortVector<double, double> *_data);


signals:
    void enterLipid(string lipid_name);
    void exitLipid();
    void markLipid();

public slots:
    void lipidEntered(string lipid_name);
    void lipidExited();
    void lipidMarked();

};


class StatTestLine {

public:
    QGraphicsLineItem *line;
    QGraphicsSimpleTextItem *stars;
    double pvalue;
    double line_y;

    StatTestLine(Chart *_chart, double _pvalue, double _line_y);
};


class Barplot : public Chartplot {
    Q_OBJECT

public:
    vector< vector<BarBox*>* > bars;
    map<QString, vector<BarBox*>* > bar_map;
    bool y_log_scale;
    bool show_data;
    double min_log_value;
    QPointF mouse_shift_start;
    QPointF shift_start;
    int zoom;
    vector< StatTestLine > stat_test_lines;
    static QFont stars_font;
    double stars_offset;
    double show_pvalues;

    Barplot(Chart *_chart, bool _log_scale = false, bool _show_data = false, bool _show_pvalues = false);
    ~Barplot();
    void add(vector< vector< Array > > *_data, vector<QString> *categories, vector<QString> *labels, vector<QColor> *colors);
    void update_chart();
    void clear();
    void recompute_hights();


signals:
    void enterLipid(string lipid_name);
    void exitLipid();
    void markLipid();
    void startLoading(Barplot *barplot, vector< vector< Array > > *_data, vector<QString> *categories, vector<QString> *labels, vector<QColor> *colors);

public slots:
    void lipidEntered(string lipid_name);
    void lipidExited();
    void lipidMarked();
    void wheelEvent(QWheelEvent *event) override;
    void setYLogScale(bool log_scale);
    void setShowDataPoints(bool data_points);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent() override;
    void setStatResults(bool _show_pvalues) override;
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
    bool is_border;

    Lineplot(Chart *_chart, bool _is_border = false);
    ~Lineplot();
    void add(vector< pair< pair<double, double>, pair<double, double> > > &lines, QString category, QColor color = QColor("#99ca53"));
    void update_chart();
    void clear();
};











class ScPoint : public QGraphicsEllipseItem {
public:
    double x;
    double y;
    QColor color;
    QString label;
    bool highlight = false;
    HoverSignal hover_signal;

    ScPoint(double _x, double _y, QColor _color = QColor("#209fdf"), QString _label = "", QGraphicsItem *parent = nullptr);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};


class Scatterplot : public Chartplot {
    Q_OBJECT

public:
    QPointF mouse_shift_start;
    QPointF shift_start_x;
    QPointF shift_start_y;
    QPointF zoom_start;
    vector<ScPoint*> points;
    map<QString, ScPoint*> point_map;
    QGraphicsRectItem *selection_rect;
    bool lipid_plot = false;

    Scatterplot(Chart *_chart, bool _lipid_plot = false);
    ~Scatterplot();
    void add(vector< pair<double, double> > &data, QString category, QColor color = QColor("#209fdf"), vector<QString> *data_labels = 0);
    void update_chart();
    void clear();
    void wheelEvent(QWheelEvent *event) override;
    void highlight_lipids(QPointF);

signals:
    void enterLipid(string lipid_name);
    void exitLipid();
    void markLipid();
    void markLipids(set<string> *lipids);

public slots:
    void lipidEntered(string lipid_name);
    void lipidExited();
    void lipidMarked();
    void lipidsMarked(set<string> *lipids);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};








class Histogramplot;


class HistogramBox {
public:
    double x;
    double x_width;
    double y;
    QGraphicsRectItem *rect;
    QGraphicsRectItem *background_rect;
    QColor color;

    HistogramBox(Histogramplot *histogram_plot, double _x, double _x_width, double _y, QColor _color = Qt::red, bool transparent = true);
};

class Histogramplot : public Chartplot {
public:
    vector<HistogramBox> boxes;
    QPointF borders;

    Histogramplot(Chart *_chart);
    ~Histogramplot();
    void add(vector<Array> &arrays, vector<QString> &categories, vector<QColor> *colors = 0, uint num_bars = 20);
    void update_chart();
    void clear();
    void wheelEvent(QWheelEvent *event) override;
};





class PlottingFunction : public QGraphicsItem {
public:
    Chart* chart;
    std::function<double(double, vector<double>)> plotting_function;
    vector<double> plotting_parameters;
    QColor color;

    PlottingFunction(Chart* chart, std::function<double(double, vector<double>)>, vector<double>, QColor _color = Qt::red);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
};

class FunctionPlot : public Chartplot {
public:
    vector<PlottingFunction*> functions;

    FunctionPlot(Chart *_chart);
    ~FunctionPlot();
    void add(std::function<double(double, vector<double>)>, vector<double>, QString category = "", QColor _color = Qt::red);
    void update_chart();
    void clear();
};

#endif /* PLOTS_H */
