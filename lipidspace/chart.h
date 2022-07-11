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
#include "lipidspace/globaldata.h"
#include "lipidspace/chartplot.h"
//#include "lipidspace/boxplot.h"
//#include "lipidspace/scatterplot.h"

#define TICK_NUM 5
#define TICK_SIZE 5

using namespace std;

class Chartplot;

struct LegendCategory {
    QColor color;
    QString category_string;

    QGraphicsRectItem *rect;
    QGraphicsTextItem *category;

    LegendCategory(QString _category, QColor _color, QGraphicsScene *scene){
        color = _color;
        category_string = _category;
        rect = new QGraphicsRectItem();
        category = new QGraphicsTextItem(_category);
        scene->addItem(rect);
        scene->addItem(category);
    }
};


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
    vector<LegendCategory> legend_categories;
    QPointF xrange;
    QPointF yrange;
    vector<Chartplot*> chart_plots;


    vector<QGraphicsLineItem*> v_grid;
    vector<QGraphicsLineItem*> h_grid;
    vector<QGraphicsTextItem*> x_ticks;
    vector<QGraphicsTextItem*> y_ticks;

    Chart(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *);
    void setTitle(QString t);
    void setXLabel(QString l);
    void setYLabel(QString l);
    void translate(double &x, double &y);
    void add(Chartplot* plot);

    virtual void update_chart();
    virtual void clear();

public slots:
    void set_tick_size(int i);
    void set_title_size(int i);
};


#endif /* CHART_H */

