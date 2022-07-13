#include "lipidspace/lineplot.h"

Lineplot::Lineplot(Chart *_chart) : Chartplot(_chart) {
}

Lineplot::~Lineplot(){

}

void Lineplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);

    for (auto &line : lines){
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
        line.line->setVisible(visible);
    }
}


void Lineplot::clear(){
    for (auto &line : lines){
        chart->scene.removeItem(line.line);
    }
    lines.clear();
}



void Lineplot::add(vector< pair< pair<double, double>, pair<double, double> > > &_lines, QString category, QColor _color){
    chart->show_y_axis = true;
    chart->show_x_axis = true;


    for (auto line : _lines){
        double x1 = line.first.first;
        double y1 = line.first.second;
        double x2 = line.second.first;
        double y2 = line.second.second;

        if (lines.size()){
            chart->xrange.setX(min(chart->xrange.x(), min(x1, x2)));
            chart->xrange.setY(max(chart->xrange.y(), max(x1, x2)));
            chart->yrange.setX(min(chart->yrange.x(), min(y1, y2)));
            chart->yrange.setY(max(chart->yrange.y(), max(y1, y2)));
        }
        else {
            chart->xrange.setX(min(x1, x2));
            chart->xrange.setY(max(x1, x2));
            chart->yrange.setX(min(y1, y2));
            chart->yrange.setY(max(y1, y2));
        }

        lines.push_back(Line(x1, y1, x2, y2, _color));
        chart->scene.addItem(lines.back().line);
    }

    chart->legend_categories.push_back(LegendCategory(category, _color, &chart->scene));
}
