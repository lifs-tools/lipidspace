#include "lipidspace/scatterplot.h"

Scatterplot::Scatterplot(Chart *_chart) : Chartplot(_chart) {
    base = new QGraphicsRectItem();
    base->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    chart->scene.addItem(base);
}

Scatterplot::~Scatterplot(){

}

void Scatterplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    base->setRect(chart->chart_box_inner.x(), chart->chart_box_inner.y(), chart->chart_box_inner.width(), chart->chart_box_inner.height());
    base->setPen(Qt::NoPen);
    base->setBrush(Qt::NoBrush);

    for (auto &p : points){
        double x = p.x;
        double y = p.y;
        chart->translate(x, y);

        QPen pen(Qt::white);
        pen.setWidthF(1.5);
        p.p->setPen(pen);
        p.p->setRect(x - 7, y - 7, 14, 14);
        p.p->setBrush(QBrush(p.color));
        p.p->setVisible(visible);
    }
}


void Scatterplot::clear(){
    for (auto &p : points){
        chart->scene.removeItem(p.p);
    }
    points.clear();
}



void Scatterplot::add(vector< pair<double, double> > &data, QString category, QColor color){
    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();


    double xmin = chart->xrange.x();
    double xmax = chart->xrange.y();
    double ymin = chart->yrange.x();
    double ymax = chart->yrange.y();
    for (auto xy_point : data){
        xmin = min(xmin, xy_point.first);
        xmax = max(xmax, xy_point.first);
        ymin = min(ymin, xy_point.second);
        ymax = max(ymax, xy_point.second);

        points.push_back(ScPoint(xy_point.first, xy_point.second, color));
        chart->scene.addItem(points.back().p);
        points.back().p->setParentItem(base);
    }
    chart->xrange.setX(xmin);
    chart->xrange.setY(xmax);
    chart->yrange.setX(ymin);
    chart->yrange.setY(ymax);

    /*
    double x_offset = (chart->xrange.y() - chart->xrange.x()) * 0.025;
    double y_offset = (chart->yrange.y() - chart->yrange.x()) * 0.025;

    chart->xrange.setX(chart->xrange.x() - x_offset);
    chart->xrange.setY(chart->xrange.y() + x_offset);
    chart->yrange.setX(chart->yrange.x() - y_offset);
    chart->yrange.setY(chart->yrange.y() + y_offset);
    */

    chart->legend_categories.push_back(LegendCategory(category, color, &chart->scene));
}
