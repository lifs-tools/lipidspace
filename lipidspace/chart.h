#ifndef CHART_H
#define CHART_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QResizeEvent>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <chrono>
#include <QRect>
#include <QString>
#include <QColor>
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/globaldata.h"
#include "lipidspace/chartplot.h"

#define TICK_NUM 5
#define TICK_SIZE 5
#define TIMER_DURATION 700000.0  // micro seconds

using namespace std;
using namespace chrono;

class Chartplot;

class LegendCategory {
public:
    QColor color;
    QString category_string;
    QGraphicsRectItem *rect;
    QGraphicsTextItem *category;

    LegendCategory(QString _category, QColor _color, QGraphicsScene *scene);
};


class Chart : public QGraphicsView {
    Q_OBJECT

public:
    QGraphicsTextItem *title;
    QGraphicsTextItem *xlabel;
    QGraphicsTextItem *ylabel;
    QRectF title_box;
    QRectF xlabel_box;
    QRectF ylabel_box;
    QRectF legend_box;
    QRectF chart_box;
    QRectF chart_box_inner;
    QFont tick_font;
    QFont label_font;
    QFont title_legend_font;
    QGraphicsScene scene;
    QPointF xrange;
    QPointF yrange;
    int timer_id;
    bool loaded;
    bool show_x_axis;
    bool show_y_axis;
    bool is_x_category_axis;
    bool log_x_axis;
    bool log_y_axis;
    vector<LegendCategory> legend_categories;
    vector<QString> x_categories;
    vector<Chartplot*> chart_plots;
    double animation;
    steady_clock::time_point animation_start;
    QGraphicsRectItem *base;

    vector<QGraphicsLineItem*> v_grid;
    vector<QGraphicsLineItem*> h_grid;
    vector<QGraphicsTextItem*> x_ticks;
    vector<QGraphicsTextItem*> y_ticks;

    Chart(QWidget *parent = nullptr);
    void setTitle(QString t);
    void setXLabel(QString l);
    void setYLabel(QString l);
    void translate(double &x, double &y);
    void back_translate_x(double &x);
    void back_translate_y(double &y);
    void add(Chartplot* plot);
    void add_category(QString);
    void create_x_numerical_axis(bool _log_x = false);
    void create_x_nominal_axis();
    void create_y_numerical_axis(bool _log_y = false);
    void resizeEvent(QResizeEvent *event) override;

    void update_chart();
    void clear();
    void reset_animation();
    void animation_step();
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

protected:
    void timerEvent(QTimerEvent *event) override;

public slots:
    void setTickSize(int i);
    void setTitleSize(int i);
    void wheelEvent(QWheelEvent* event) override;
    void setYLogScale(bool log_scale);
    void setShowDataPoints(bool data_points);
    void mouseMoveEvent(QMouseEvent *event) override;
    void setStatResults(bool show_pvalues);

signals:
    void wheel(QWheelEvent *event);
    void resize();
    void yLogScaleChanged(bool log_scale);
    void showDataPointsChanged(bool data_points);
    void mouseMoved(QMouseEvent *event);
    void settingStatResults(bool show_pvalues);
    void openFiles(const QList<QUrl> &);
};


#endif /* CHART_H */

