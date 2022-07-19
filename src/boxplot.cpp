#include "lipidspace/boxplot.h"

Boxplot::Boxplot(Chart *_chart) : Chartplot(_chart) {
}

Boxplot::~Boxplot(){
}

void Boxplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);

    for (uint b = 0; b < boxes.size(); ++b){
        auto &box = boxes[b];
        double x1 = b - 0.25;
        double y1 = box.upper_extreme;
        double x2 = b + 0.25;
        double y2 = box.upper_extreme;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.upper_extreme_line->setLine(x1, y1, x2, y2);
        box.upper_extreme_line->setZValue(0);
        box.upper_extreme_line->setVisible(visible);

        x1 = b - 0.25;
        y1 = box.lower_extreme;
        x2 = b + 0.25;
        y2 = box.lower_extreme;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.lower_extreme_line->setLine(x1, y1, x2, y2);
        box.lower_extreme_line->setZValue(0);
        box.lower_extreme_line->setVisible(visible);

        x1 = b;
        y1 = box.upper_extreme;
        x2 = b;
        y2 = box.lower_extreme;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.base_line->setLine(x1, y1, x2, y2);
        box.base_line->setZValue(0);
        box.base_line->setVisible(visible);

        x1 = b - 0.25;
        y1 = box.lower_quartile;
        x2 = b + 0.25;
        y2 = box.upper_quartile;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.rect->setBrush(QBrush(box.color));
        box.rect->setRect(x1, y1, x2 - x1, y2 - y1);
        box.rect->setZValue(50);
        box.rect->setVisible(visible);

        x1 = b - 0.25;
        y1 = box.median;
        x2 = b + 0.25;
        y2 = box.median;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.median_line->setPen(QPen());
        box.median_line->setLine(x1, y1, x2, y2);
        box.median_line->setZValue(100);
        box.median_line->setVisible(visible);
    }
}


void Boxplot::clear(){
    for (auto &box : boxes){
        chart->scene.removeItem(box.upper_extreme_line);
        chart->scene.removeItem(box.lower_extreme_line);
        chart->scene.removeItem(box.base_line);
        chart->scene.removeItem(box.median_line);
        chart->scene.removeItem(box.rect);
    }
    boxes.clear();
}


double Boxplot::median(vector<double> &lst, int begin, int end){
    int count = end - begin;
    if (count & 1) {
        return lst[(count >> 1) + begin];
    } else {
        double right = lst[(count >> 1) + begin];
        double left = lst[(count >> 1) - 1 + begin];
        return (right + left) / 2.0;
    }
}


void Boxplot::add(Array &array, QString category, QColor color){
    chart->create_y_numerical_axis();

    sort(array.begin(), array.end());
    int count = array.size();

    boxes.push_back(WhiskerBox(&chart->scene));
    WhiskerBox &box = boxes.back();
    box.color = color;
    box.lower_extreme = array.front();
    box.upper_extreme = array.back();
    box.median = median(array, 0, count);
    box.lower_quartile = median(array, 0, count >> 1);
    box.upper_quartile = median(array, (count >> 1) + (count & 1), count);

    chart->xrange = QPointF(-0.5, boxes.size() - 0.5);
    chart->yrange = QPointF(boxes.size() > 1 ? min(chart->yrange.x(), array.front()) : array.front(), boxes.size() > 1 ? max(chart->yrange.y(), array.back()) : array.back());

    chart->legend_categories.push_back(LegendCategory(category, box.color, &chart->scene));
}
