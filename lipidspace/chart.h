#ifndef CHART_H
#define CHART_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QString>
#include <QColor>
#include "lipidspace/AssistanceFunctions.h"

#define TICK_NUM 5
#define TICK_SIZE 10

using namespace std;


class Chart : public QGraphicsView {
public:
    QGraphicsTextItem *title;
    QGraphicsTextItem *xlabel;
    QGraphicsTextItem *ylabel;
    QRect title_box;
    QRect xlabel_box;
    QRect ylabel_box;
    QRect legend_box;
    QRect chart_box;
    QRectF chart_box_inner;
    QFont tick_font;
    QFont title_legend_font;
    QGraphicsScene scene;
    bool loaded;
    bool show_x_axis;
    bool show_y_axis;
    vector<string> legend_categories;
    QPointF xrange;
    QPointF yrange;


    vector<QGraphicsLineItem*> v_grid;
    vector<QGraphicsLineItem*> h_grid;
    vector<QGraphicsTextItem*> x_ticks;
    vector<QGraphicsTextItem*> y_ticks;

    Chart(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *);
    void setTitle(string t);
    void setXLabel(string l);
    void setYLabel(string l);
    double median(vector<double> &lst, int begin, int end);
    void translate(double &x, double &y);

    virtual void update_chart();
    virtual void clear();
};


#endif /* CHART_H */

