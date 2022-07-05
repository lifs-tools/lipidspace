#include "lipidspace/chart.h"


Chart::Chart(QWidget *parent) : QGraphicsView(parent), loaded(false) {
    setFrameStyle(QFrame::NoFrame);
    setRenderHints(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    title = new QGraphicsTextItem("");
    xlabel = new QGraphicsTextItem("");
    ylabel = new QGraphicsTextItem("");

    xrange = QPointF(0, 0);
    yrange = QPointF(0, 0);

    show_x_axis = false;
    show_y_axis = false;

    scene.setSceneRect(0, 0, width(), height());

    for (int i = 0; i < (TICK_NUM << 1); ++i){
        QGraphicsLineItem* l = new QGraphicsLineItem(QLineF(0, 0, 0, 0));
        QGraphicsTextItem* t = new QGraphicsTextItem();

        if (i < TICK_NUM){
            h_grid.push_back(l);
            x_ticks.push_back(t);
        }
        else {
            v_grid.push_back(l);
            y_ticks.push_back(t);
        }
        QPen pen;
        pen.setWidthF(1);
        pen.setColor(QColor("#DDDDDD"));
        l->setPen(pen);

        scene.addItem(l);
        scene.addItem(t);
    }

    scene.addItem(title);
    scene.addItem(xlabel);
    scene.addItem(ylabel);

    setScene(&scene);
    loaded = true;
}


void Chart::clear(){
    title->setPlainText("");
    xlabel->setPlainText("");
    ylabel->setPlainText("");

    legend_categories.clear();

    show_x_axis = false;
    show_y_axis = false;
}

double Chart::median(vector<double> &lst, int begin, int end){
    int count = end - begin;
    if (count & 1) {
        return lst[(count >> 1) + begin];
    } else {
        double right = lst[(count >> 1) + begin];
        double left = lst[(count >> 1) - 1 + begin];
        return (right + left) / 2.0;
    }
}

void Chart::translate(double &x, double &y){
    x = chart_box_inner.x() + chart_box_inner.width() / (xrange.y() - xrange.x()) * (x - xrange.x());
    y = chart_box_inner.y() + chart_box_inner.height() - chart_box_inner.height() / (yrange.y() - yrange.x()) * (y - yrange.x());
}


void Chart::update_chart(){
    if (!loaded) return;
    scene.setSceneRect(0, 0, width(), height());

    QGraphicsTextItem title_test_item("test");
    title_test_item.setFont(title_legend_font);
    QRectF title_rect = title_test_item.boundingRect();

    QGraphicsTextItem tick_item("test");
    tick_item.setFont(tick_font);
    QRectF tick_rect = tick_item.boundingRect();

    title_box.setRect(0, 0, width(), 0);
    if (title->toPlainText().size() > 0){
        title_box.setHeight(title_rect.height());
        title->setPos((title_box.width() - title->boundingRect().width()) / 2., title_box.x());
        title->setVisible(true);
    }
    else {
        title->setVisible(false);
    }


    legend_box.setRect(0, height(), width(), 0);
    if (legend_categories.size() > 0){
        legend_box.setY(legend_box.y() - title_rect.height());
        legend_box.setHeight(title_rect.height());
    }


    xlabel_box.setRect(0, height() - legend_box.height(), width(), 0);
    if (xlabel->toPlainText().size() > 0){
        xlabel_box.setY(xlabel_box.y() - tick_rect.height());
        xlabel_box.setHeight(tick_rect.height());
        xlabel->setPos((xlabel_box.width() - xlabel->boundingRect().width()) / 2., xlabel_box.y());
        xlabel->setVisible(true);
    }
    else {
        xlabel->setVisible(false);
    }

    ylabel->setRotation(0);
    ylabel_box.setRect(0, title_box.height(), 0, height() - title_box.height() - legend_box.height() - xlabel_box.height());
    if (ylabel->toPlainText().size() > 0){
        ylabel_box.setWidth(ylabel_box.y() - tick_rect.height());
        ylabel->setPos((ylabel_box.width() - ylabel->boundingRect().width()) / 2., ylabel_box.y() + (ylabel_box.height() - ylabel->boundingRect().height()) / 2.);
        ylabel->setTransformOriginPoint(ylabel->boundingRect().width() / 2., ylabel->boundingRect().height() / 2.);
        ylabel->setRotation(-90);
        ylabel->setVisible(true);
    }
    else {
        ylabel->setVisible(false);
    }

    chart_box.setRect(ylabel_box.x() + ylabel_box.width(), title_box.y() + title_box.height(), width() - ylabel_box.width(), ylabel_box.height());
    //cout << chart_box.x() << " " << chart_box.y() << " " << chart_box.width() << " " << chart_box.height() << endl;

    double max_tick_width = 0;
    double sum_x_tick_width = 0;
    for (int i = 0; i < TICK_NUM; ++i){
        tick_item.setPlainText(QString("%1").arg(yrange.x() + (double)i / (TICK_NUM - 1.) * (yrange.y() - yrange.x()), 1, 'g'));
        QRectF t_rect = tick_item.boundingRect();
        max_tick_width = max(max_tick_width, t_rect.width());

        QGraphicsTextItem x_tick(QString("%1").arg(xrange.y() + (double)i / (TICK_NUM - 1.) * (xrange.y() - xrange.x()), 1, 'g'));
        sum_x_tick_width += x_tick.boundingRect().width();
    }
    max_tick_width += TICK_SIZE;

    QGraphicsTextItem max_x_tick(QString("%1").arg(xrange.y(), 1, 'g'));
    double max_x_tick_width = max(max_x_tick.boundingRect().width() / 2., 20.);

    chart_box_inner.setRect(chart_box.x() + max_tick_width, chart_box.y() + tick_rect.height() / 2, chart_box.width() - max_tick_width - max_x_tick_width, chart_box.height() - 1.5 * tick_rect.height());


    for (int i = 0; i < TICK_NUM; ++i){
        auto line = h_grid[i];
        if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0 && show_y_axis){
            line->setVisible(true);
            double h = chart_box_inner.y() + (double)i / (TICK_NUM - 1) * chart_box_inner.height();
            line->setLine(chart_box_inner.x() - TICK_SIZE, h, chart_box_inner.x() + chart_box_inner.width(), h);
        }
        else {
            line->setVisible(false);
        }

        auto tick = y_ticks[i];
        if (tick_rect.height() * TICK_NUM * 0.7 < chart_box.height() && chart_box_inner.width() > 0 && chart_box_inner.height() > 0 && show_y_axis){
            tick->setVisible(true);
            tick->setFont(tick_font);
            tick->setPlainText(QString("%1").arg(yrange.x() + (TICK_NUM - 1. - (double)i) / (TICK_NUM - 1) * (yrange.y() - yrange.x()), 1, 'g'));
            double x = chart_box_inner.x() - tick->boundingRect().width() - TICK_SIZE;
            double y = chart_box_inner.y() + (double)i / (TICK_NUM - 1) * chart_box_inner.height() - tick->boundingRect().height() / 2.;
            tick->setPos(x, y);
        }
        else {
            tick->setVisible(false);
        }

        line = v_grid[i];
        if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0 && show_x_axis){
            line->setVisible(true);
            double w = chart_box_inner.x() + (double)i / (TICK_NUM - 1)  * chart_box_inner.width();
            line->setLine(w, chart_box_inner.y(), w, chart_box_inner.y() + chart_box_inner.height() + 5);
        }
        else {
            line->setVisible(false);
        }

        tick = x_ticks[i];
        if (sum_x_tick_width < chart_box.width() && chart_box_inner.width() > 0 && chart_box_inner.height() > 0 && show_x_axis){
            tick->setVisible(true);
            tick->setFont(tick_font);
            tick->setPlainText(QString("%1").arg(xrange.x() + (double)i / (TICK_NUM - 1) * (xrange.y() - xrange.x()), 1, 'g'));
            double x = chart_box_inner.x() + (double)i / (TICK_NUM - 1)  * (chart_box_inner.width() - TICK_NUM) - tick->boundingRect().width() / 2.;
            double y = chart_box_inner.y() + chart_box_inner.height() + TICK_SIZE;
            tick->setPos(x, y);
        }
        else {
            tick->setVisible(false);
        }
    }

    for (auto category : legend_categories){

    }
}

void Chart::setTitle(string t){
    title->setPlainText(t.c_str());
    update_chart();
}

void Chart::setXLabel(string l){
    xlabel->setPlainText(l.c_str());
    update_chart();
}

void Chart::setYLabel(string l){
    ylabel->setPlainText(l.c_str());
    update_chart();
}

void Chart::resizeEvent(QResizeEvent*){
    update_chart();
}
