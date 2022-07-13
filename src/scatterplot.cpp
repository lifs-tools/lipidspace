#include "lipidspace/scatterplot.h"

Scatterplot::Scatterplot(Chart *_chart) : Chartplot(_chart) {
}

Scatterplot::~Scatterplot(){

}

void Scatterplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);

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
    chart->show_y_axis = true;
    chart->show_x_axis = true;


    for (auto xy_point : data){
        if (points.size()){
            chart->xrange.setX(min(chart->xrange.x(), xy_point.first));
            chart->xrange.setY(max(chart->xrange.y(), xy_point.first));
            chart->yrange.setX(min(chart->yrange.x(), xy_point.second));
            chart->yrange.setY(max(chart->yrange.y(), xy_point.second));
        }
        else {
            chart->xrange.setX(xy_point.first);
            chart->xrange.setY(xy_point.first);
            chart->yrange.setX(xy_point.second);
            chart->yrange.setY(xy_point.second);
        }

        points.push_back(ScPoint(xy_point.first, xy_point.second, color));
        chart->scene.addItem(points.back().p);
    }

    double x_offset = (chart->xrange.y() - chart->xrange.x()) * 0.05;
    double y_offset = (chart->yrange.y() - chart->yrange.x()) * 0.05;

    chart->xrange.setX(chart->xrange.x() - x_offset);
    chart->xrange.setY(chart->xrange.y() + x_offset);
    chart->yrange.setX(chart->yrange.x() - y_offset);
    chart->yrange.setY(chart->yrange.y() + y_offset);

    chart->legend_categories.push_back(LegendCategory(category, color, &chart->scene));
}
