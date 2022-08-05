#include "lipidspace/scatterplot.h"


ScPoint::ScPoint(double _x, double _y, QColor _color){
    x = _x;
    y = _y;
    color = _color;
    p = new QGraphicsEllipseItem();
}



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

    double animation_length = chart->animation * points.size();

    for (uint i = 0; i < points.size(); ++i){
        auto &p = points[i];
        double x = p.x;
        double y = p.y;
        chart->translate(x, y);

        QPen pen(Qt::white);
        pen.setWidthF(1.5);
        p.p->setPen(pen);
        p.p->setRect(x - 7, y - 7, 14, 14);
        p.p->setBrush(QBrush(p.color));
        p.p->setVisible(visible && (i <= animation_length));
    }
}


void Scatterplot::clear(){
    points.clear();
}



void Scatterplot::add(vector< pair<double, double> > &data, QString category, QColor color){


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

    chart->legend_categories.push_back(LegendCategory(category, color, &chart->scene));
    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();
    chart->reset_animation();
}
