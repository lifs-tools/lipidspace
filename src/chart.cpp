#include "lipidspace/chart.h"


Chart::Chart(QWidget *parent) : QGraphicsView(parent), loaded(false) {
    setFrameStyle(QFrame::NoFrame);
    setRenderHints(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(&timer, &QTimer::timeout, this, &Chart::animation_step);

    tick_font = QFont("Helvetica", GlobalData::gui_num_var["tick_size"]);
    label_font = QFont("Helvetica", GlobalData::gui_num_var["tick_size"], QFont::Bold);
    title_legend_font = QFont("Helvetica", GlobalData::gui_num_var["legend_size"]);

    title = new QGraphicsTextItem("");
    xlabel = new QGraphicsTextItem("");
    ylabel = new QGraphicsTextItem("");

    xrange = QPointF(1e100, -1e100);
    yrange = QPointF(1e100, -1e100);

    log_x_axis = false;
    log_y_axis = false;

    show_x_axis = false;
    show_y_axis = false;
    is_x_category_axis = false;

    scene.setSceneRect(0, 0, width(), height());

    scene.addItem(title);
    scene.addItem(xlabel);
    scene.addItem(ylabel);

    setScene(&scene);
    loaded = true;

    animation = 0;
}



void Chart::resizeEvent(QResizeEvent *event){
    reset_animation();
    update_chart();
    QGraphicsView::resizeEvent(event);
}


void Chart::reset_animation(){
    animation = 0;
    animation_start = chrono::steady_clock::now();
    timer.start(10);
}


void Chart::animation_step(){
    if (animation < 1){
        update_chart();
        auto animation_now = chrono::steady_clock::now();
        animation = min(1., ((double)chrono::duration_cast<chrono::microseconds>(animation_now - animation_start).count()) / TIMER_DURATION);
    }
    else {
        update_chart();
        timer.stop();
    }
}


void Chart::create_x_numerical_axis(bool _log_x){
    if (show_x_axis) return;

    show_x_axis = true;
    log_x_axis = _log_x;
    is_x_category_axis = false;
    for (int i = 0; i < TICK_NUM; ++i){
        QGraphicsLineItem* l = new QGraphicsLineItem(QLineF(0, 0, 0, 0));
        QGraphicsTextItem* t = new QGraphicsTextItem();

        x_ticks.push_back(t);
        v_grid.push_back(l);

        QPen pen;
        pen.setWidthF(1);
        pen.setColor(QColor("#DDDDDD"));
        l->setPen(pen);

        scene.addItem(l);
        scene.addItem(t);
    }
}



void Chart::create_x_nominal_axis(){
    if (show_x_axis) return;

    is_x_category_axis = true;
    show_x_axis = true;
    QGraphicsLineItem* l = new QGraphicsLineItem(QLineF(0, 0, 0, 0));
    v_grid.push_back(l);
    QPen pen;
    pen.setWidthF(1);
    pen.setColor(QColor("#DDDDDD"));
    l->setPen(pen);
    scene.addItem(l);
}



void Chart::create_y_numerical_axis(bool _log_y){
    if (show_y_axis) return;

    show_y_axis = true;
    log_y_axis = _log_y;
    if (log_y_axis){
        int l = yrange.x() > 0 ? ceil(log(yrange.x()) / log(10.)) : 0;
        int u = floor(log(yrange.y()) / log(10.));

        for (int i = l; i <= u; ++i){
            QGraphicsLineItem* l = new QGraphicsLineItem(QLineF(0, 0, 0, 0));
            QGraphicsTextItem* t = new QGraphicsTextItem();

            y_ticks.push_back(t);
            h_grid.push_back(l);

            QPen pen;
            pen.setWidthF(1);
            pen.setColor(QColor("#DDDDDD"));
            l->setPen(pen);

            scene.addItem(l);
            scene.addItem(t);
        }
    }
    else {
        for (int i = 0; i < TICK_NUM; ++i){
            QGraphicsLineItem* l = new QGraphicsLineItem(QLineF(0, 0, 0, 0));
            QGraphicsTextItem* t = new QGraphicsTextItem();

            y_ticks.push_back(t);
            h_grid.push_back(l);

            QPen pen;
            pen.setWidthF(1);
            pen.setColor(QColor("#DDDDDD"));
            l->setPen(pen);

            scene.addItem(l);
            scene.addItem(t);
        }
    }
}




void Chart::clear(){
    title->setPlainText("");
    xlabel->setPlainText("");
    ylabel->setPlainText("");

    xrange = QPointF(1e100, -1e100);
    yrange = QPointF(1e100, -1e100);
    log_x_axis = false;
    log_y_axis = false;

    show_x_axis = false;
    show_y_axis = false;
    is_x_category_axis = false;

    legend_categories.clear();

    x_ticks.clear();
    y_ticks.clear();
    h_grid.clear();
    v_grid.clear();
    x_categories.clear();

    for (auto plot : chart_plots){
        plot->clear();
        delete plot;
    }
    chart_plots.clear();

    scene.removeItem(title);
    scene.removeItem(xlabel);
    scene.removeItem(ylabel);
    scene.clear();
    scene.addItem(title);
    scene.addItem(xlabel);
    scene.addItem(ylabel);

    update_chart();
}

void Chart::translate(double &x, double &y){
    if (log_x_axis){
        if (x > xrange.x()){
            x = chart_box_inner.x() + chart_box_inner.width() / (log(xrange.y() / xrange.x()) / log(10)) * (log(x / xrange.x())) / log(10);
        }
        else {
            x = chart_box_inner.x();
        }
    }
    else {
        x = chart_box_inner.x() + chart_box_inner.width() / (xrange.y() - xrange.x()) * (x - xrange.x());
    }


    if (log_y_axis){
        if (y > yrange.x()){
            y = chart_box_inner.y() + chart_box_inner.height() - chart_box_inner.height() / (log(yrange.y() / yrange.x()) / log(10)) * (log(y / yrange.x())) / log(10);
        }
        else {
            y = chart_box_inner.y() + chart_box_inner.height();
        }
    }
    else {
        y = chart_box_inner.y() + chart_box_inner.height() - chart_box_inner.height() / (yrange.y() - yrange.x()) * (y - yrange.x());
    }
}


void Chart::add(Chartplot *plot){
    chart_plots.push_back(plot);
    update_chart();
}


void Chart::add_category(QString category){
    x_categories.push_back(category);

    QGraphicsLineItem* l = new QGraphicsLineItem(QLineF(0, 0, 0, 0));
    QGraphicsTextItem* t = new QGraphicsTextItem();

    x_ticks.push_back(t);
    v_grid.push_back(l);
    QPen pen;
    pen.setWidthF(1);
    pen.setColor(QColor("#DDDDDD"));
    l->setPen(pen);

    scene.addItem(l);
    scene.addItem(t);
}


void Chart::update_chart(){
    if (!loaded) return;
    scene.setSceneRect(0, 0, width(), height());
    setBackgroundBrush(QBrush());

    QGraphicsTextItem title_test_item("test");
    title_test_item.setFont(title_legend_font);
    QRectF title_rect = title_test_item.boundingRect();

    QGraphicsTextItem tick_item("...");
    tick_item.setFont(tick_font);
    QRectF tick_rect = tick_item.boundingRect();

    QGraphicsTextItem label_item("test");
    label_item.setFont(label_font);
    QRectF label_rect = label_item.boundingRect();



    title_box.setRect(0, 0, width(), 0);
    if (title->toPlainText().size() > 0){
        title_box.setHeight(title_rect.height());
        title->setFont(title_legend_font);
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
        xlabel_box.setY(xlabel_box.y() - label_rect.height());
        xlabel_box.setHeight(label_rect.height());
        xlabel->setFont(label_font);
        xlabel->setPos((xlabel_box.width() - xlabel->boundingRect().width()) / 2., xlabel_box.y());
        xlabel->setVisible(true);
    }
    else {
        xlabel->setVisible(false);
    }

    ylabel->setRotation(0);
    ylabel_box.setRect(0, title_box.height(), 0, height() - title_box.height() - legend_box.height() - xlabel_box.height());
    if (ylabel->toPlainText().size() > 0){
        ylabel->setFont(label_font);
        ylabel_box.setWidth(label_rect.height());
        ylabel->setPos((ylabel_box.width() - ylabel->boundingRect().width()) / 2., ylabel_box.y() + (ylabel_box.height() - ylabel->boundingRect().height()) / 2.);
        ylabel->setTransformOriginPoint(ylabel->boundingRect().width() / 2., ylabel->boundingRect().height() / 2.);
        ylabel->setRotation(-90);
        ylabel->setVisible(true);
    }
    else {
        ylabel->setVisible(false);
    }

    chart_box.setRect(ylabel_box.x() + ylabel_box.width(), title_box.y() + title_box.height(), width() - ylabel_box.width(), ylabel_box.height());

    double max_tick_width = 0;
    double sum_x_tick_width = 0;
    for (int i = 0; i < TICK_NUM; ++i){
        tick_item.setPlainText(QString("%1").arg(yrange.x() + (double)i / (TICK_NUM - 1.) * (yrange.y() - yrange.x()), 0, 'f', 1));
        QRectF t_rect = tick_item.boundingRect();
        max_tick_width = max(max_tick_width, t_rect.width());

        QGraphicsTextItem x_tick(QString("%1").arg(xrange.y() + (double)i / (TICK_NUM - 1.) * (xrange.y() - xrange.x()), 0, 'f', 1));
        sum_x_tick_width += x_tick.boundingRect().width();
    }
    max_tick_width += TICK_SIZE;

    QGraphicsTextItem max_x_tick(QString("%1").arg(xrange.y(), 1, 'g'));
    double max_x_tick_width = max(max_x_tick.boundingRect().width() / 2., 20.);

    chart_box_inner.setRect(chart_box.x() + max_tick_width, chart_box.y() + tick_rect.height() / 2, chart_box.width() - max_tick_width - max_x_tick_width, chart_box.height() - 1.5 * tick_rect.height());





    if (show_y_axis){
        if (!log_y_axis){

            for (uint i = 0; i < h_grid.size(); ++i){
                auto line = h_grid[i];
                if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
                    line->setVisible(true);
                    double h = chart_box_inner.y() + (double)i / (h_grid.size() - 1) * chart_box_inner.height();
                    line->setLine(chart_box_inner.x() - TICK_SIZE, h, chart_box_inner.x() + chart_box_inner.width(), h);
                }
                else {
                    line->setVisible(false);
                }

                auto tick = y_ticks[i];
                if (tick_rect.height() * h_grid.size() * 0.7 < chart_box.height() && chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
                    tick->setVisible(true);
                    tick->setFont(tick_font);

                    double tick_value = yrange.x() + (TICK_NUM - 1. - (double)i) / (TICK_NUM - 1) * (yrange.y() - yrange.x());

                    tick->setPlainText(QString("%1").arg(tick_value, 0, 'f', 1));
                    double x = chart_box_inner.x() - tick->boundingRect().width() - TICK_SIZE;
                    double y = chart_box_inner.y() + (double)i / (h_grid.size() - 1) * chart_box_inner.height() - tick->boundingRect().height() / 2.;
                    tick->setPos(x, y);
                }
                else {
                    tick->setVisible(false);
                }
            }
        }
        else {
            int u = floor(log(yrange.y()) / log(10.));
            double tick_value = log_y_axis ? pow(10., u) : 0;
            for (uint i = 0; i < h_grid.size(); ++i){
                auto line = h_grid[i];
                if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
                    line->setVisible(true);

                    double h = chart_box_inner.y() + chart_box_inner.height() - chart_box_inner.height() / (log(yrange.y() / yrange.x()) / log(10)) * (log(tick_value / yrange.x())) / log(10);

                    line->setLine(chart_box_inner.x() - TICK_SIZE, h, chart_box_inner.x() + chart_box_inner.width(), h);
                }
                else {
                    line->setVisible(false);
                }

                auto tick = y_ticks[i];
                if (tick_rect.height() * h_grid.size() * 0.7 < chart_box.height() && chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
                    tick->setVisible(true);
                    tick->setFont(tick_font);

                    tick->setPlainText(QString("%1").arg(tick_value, 0, 'f', 1));
                    double x = chart_box_inner.x() - tick->boundingRect().width() - TICK_SIZE;
                    double y = chart_box_inner.y() + chart_box_inner.height() - chart_box_inner.height() / (log(yrange.y() / yrange.x()) / log(10)) * (log(tick_value / yrange.x())) / log(10) - tick->boundingRect().height() / 2.;
                    tick->setPos(x, y);
                }
                else {
                    tick->setVisible(false);
                }

                tick_value /= 10.;
            }
        }
    }

    if (show_x_axis && !is_x_category_axis){
        for (uint i = 0; i < v_grid.size(); ++i){
            auto line = v_grid[i];
            if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
                line->setVisible(true);
                double w = chart_box_inner.x() + (double)i / (TICK_NUM - 1)  * chart_box_inner.width();
                line->setLine(w, chart_box_inner.y(), w, chart_box_inner.y() + chart_box_inner.height() + 5);
            }
            else {
                line->setVisible(false);
            }

            auto tick = x_ticks[i];
            if (sum_x_tick_width < chart_box.width() && chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
                tick->setVisible(true);
                tick->setFont(tick_font);
                tick->setPlainText(QString("%1").arg(xrange.x() + (double)i / (TICK_NUM - 1) * (xrange.y() - xrange.x()), 0, 'f', 1));
                double x = chart_box_inner.x() + (double)i / (TICK_NUM - 1)  * (chart_box_inner.width() - TICK_NUM) - tick->boundingRect().width() / 2.;
                double y = chart_box_inner.y() + chart_box_inner.height() + TICK_SIZE;
                tick->setPos(x, y);
            }
            else {
                tick->setVisible(false);
            }
        }
    }

    if (show_x_axis && is_x_category_axis){
        double single_group_width = chart_box_inner.width() / (double)x_categories.size();
        for (uint i = 0; i < x_categories.size(); ++i){
            auto x_tick = x_ticks[i];
            x_tick->setFont(tick_font);
            x_tick->setHtml(x_categories[i]);
            if (x_tick->boundingRect().width() <= single_group_width){
                x_tick->setVisible(true);
                double x = chart_box_inner.x() + i * single_group_width + (single_group_width - x_tick->boundingRect().width()) / 2.;
                double y = chart_box_inner.y() + chart_box_inner.height() + TICK_SIZE;
                x_tick->setPos(x, y);

            }
            else if (tick_rect.width() <= single_group_width){
                x_tick->setVisible(true);
                x_tick->setHtml("...");
                double x = chart_box_inner.x() + i * single_group_width + (single_group_width - x_tick->boundingRect().width()) / 2.;
                double y = chart_box_inner.y() + chart_box_inner.height() + TICK_SIZE;
                x_tick->setPos(x, y);

            }
            else {
                x_tick->setVisible(false);
            }

            auto line = v_grid[i];
            if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0 && (single_group_width > 5 || i == 0)){
                line->setVisible(true);
                double w = chart_box_inner.x() + (double)i * single_group_width;
                line->setLine(w, chart_box_inner.y(), w, chart_box_inner.y() + chart_box_inner.height() + 5);
            }
            else {
                line->setVisible(false);
            }
        }
        auto line = v_grid[x_categories.size()];
        if (chart_box_inner.width() > 0 && chart_box_inner.height() > 0){
            line->setVisible(true);
            double w = chart_box_inner.x() + chart_box_inner.width();
            line->setLine(w, chart_box_inner.y(), w, chart_box_inner.y() + chart_box_inner.height() + 5);
        }
        else {
            line->setVisible(false);
        }
    }





    double legend_size = GlobalData::gui_num_var["legend_size"];
    double xlegend_width = (GlobalData::gui_num_var["legend_size"] + 5) * legend_categories.size();
    int include_legend = legend_categories.size();
    for (auto category : legend_categories){
        category.category->setHtml(category.category_string);
        category.category->setFont(title_legend_font);
        xlegend_width += category.category->boundingRect().width();
    }

    QGraphicsTextItem ppp("...");
    while (include_legend > 0 && xlegend_width > legend_box.width()){
        auto category = legend_categories[--include_legend];
        xlegend_width += ppp.boundingRect().width() - category.category->boundingRect().width();
    }

    int included_legends = 0;
    double x = (legend_box.width() - xlegend_width) / 2.;
    for (auto category : legend_categories){
        double y_box = legend_box.y() + (legend_box.height() - legend_size) / 2.;
        double y_text = legend_box.y();

        category.rect->setBrush(QColor(category.color));
        category.rect->setRect(x, y_box, legend_size, legend_size);
        x += legend_size + 5;
        category.category->setHtml(included_legends++ < include_legend ? category.category_string : "...");
        category.category->setFont(title_legend_font);
        category.category->setPos(x, y_text);
        x += category.category->boundingRect().width() + 10;
    }

    for (auto plot : chart_plots) plot->update_chart();
}

void Chart::setTitle(QString t){
    title->setHtml(t);
    update_chart();
}

void Chart::setXLabel(QString l){
    xlabel->setHtml(l);
    update_chart();
}

void Chart::setYLabel(QString l){
    ylabel->setHtml(l);
    update_chart();
}


void Chart::set_tick_size(int s){
    tick_font.setPointSizeF(s);
    label_font.setPointSizeF(s);
    update_chart();
}


void Chart::set_title_size(int s){
    title_legend_font.setPointSizeF(s);
    update_chart();
}
