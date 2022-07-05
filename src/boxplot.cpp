#include "lipidspace/boxplot.h"

Boxplot::Boxplot(QWidget *parent) : Chart(parent){

}

void Boxplot::update_chart(){
    Chart::update_chart();

    for (uint b = 0; b < boxes.size(); ++b){
        auto &box = boxes[b];
        double x1 = b - 0.25;
        double y1 = box.upper_extreme;
        double x2 = b + 0.25;
        double y2 = box.upper_extreme;
        translate(x1, y1);
        translate(x2, y2);
        box.upper_extreme_line->setLine(x1, y1, x2, y2);
    }
}


void Boxplot::clear(){
    for (auto &box : boxes){
        scene.removeItem(box.upper_extreme_line);
        scene.removeItem(box.lower_extreme_line);
        scene.removeItem(box.base_line);
        scene.removeItem(box.median_line);
        scene.removeItem(box.rect);
    }

    Chart::clear();
}


void Boxplot::add(Array &array, string category){
    show_y_axis = true;
    show_x_axis = true;

    sort(array.begin(), array.end());
    int count = array.size();

    boxes.push_back(WhiskerBox(&scene));
    WhiskerBox &box = boxes.back();
    box.lower_extreme = array.front();
    box.upper_extreme = array.back();
    box.median = median(array, 0, count);
    box.lower_quartile = median(array, 0, count >> 1);
    box.upper_quartile = median(array, (count >> 1) + (count & 1), count);

    xrange = QPointF(-0.5, boxes.size() - 0.5);
    yrange = QPointF(boxes.size() > 1 ? min(yrange.x(), array.front()) : array.front(), boxes.size() > 1 ? max(yrange.y(), array.back()) : array.back());


    update_chart();
}
