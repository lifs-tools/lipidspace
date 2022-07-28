#include "lipidspace/lineplot.h"

Lineplot::Lineplot(Chart *_chart) : Chartplot(_chart) {
}

Lineplot::~Lineplot(){

}

void Lineplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = chart->animation * lines.size();

    for (uint i = 0; i < lines.size(); ++i){
        auto &line = lines[i];
        double x1 = line.x1;
        double y1 = line.y1;
        double x2 = line.x2;
        double y2 = line.y2;
        chart->translate(x1, y1);
        chart->translate(x2, y2);

        QPen pen(line.color);
        pen.setWidthF(2);
        line.line->setPen(pen);
        line.line->setLine(x1, y1, x2, y2);
        line.line->setVisible(visible && (i <= animation_length));
    }
}


void Lineplot::clear(){
    lines.clear();
}



void Lineplot::add(vector< pair< pair<double, double>, pair<double, double> > > &_lines, QString category, QColor _color){


    double xmin = chart->xrange.x();
    double xmax = chart->xrange.y();
    double ymin = chart->yrange.x();
    double ymax = chart->yrange.y();
    for (auto line : _lines){
        double x1 = line.first.first;
        double y1 = line.first.second;
        double x2 = line.second.first;
        double y2 = line.second.second;

        xmin = min(xmin, min(x1, x2));
        xmax = max(xmax, max(x1, x2));
        ymin = min(ymin, min(y1, y2));
        ymax = max(ymax, max(y1, y2));

        lines.push_back(Line(x1, y1, x2, y2, _color));
        chart->scene.addItem(lines.back().line);
    }
    chart->xrange.setX(xmin);
    chart->xrange.setY(xmax);
    chart->yrange.setX(ymin);
    chart->yrange.setY(ymax);

    if (category.length() > 0) chart->legend_categories.push_back(LegendCategory(category, _color, &chart->scene));
    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();
    chart->reset_animation();
}
