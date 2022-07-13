#include "lipidspace/histogramplot.h"

Histogramplot::Histogramplot(Chart *_chart) : Chartplot(_chart) {
}

Histogramplot::~Histogramplot(){
}

void Histogramplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);

    for (auto &box : boxes){
        double x1 = box.x - box.x_width / 2.;
        double y1 = 0;
        double x2 = box.x + box.x_width / 2.;
        double y2 = box.y;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.rect->setRect(x1, y1, x2 - x1, y2 - y1);
        box.rect->setPen(Qt::NoPen);
        box.rect->setBrush(box.color);
        box.rect->setVisible(visible);
    }
}


void Histogramplot::clear(){
    for (auto &box : boxes){
        chart->scene.removeItem(box.rect);
    }
    boxes.clear();
}


void Histogramplot::add(vector<Array> &arrays, vector<QString> &categories, vector<QColor> *colors, uint num_bars){
    chart->show_x_axis = true;
    chart->show_y_axis = true;

    double all_min = 1e100;
    double all_max = -1e100;
    for (auto &array : arrays){
        sort(array.begin(), array.end());
        all_min = min(all_min, array.front());
        all_max = max(all_max, array.back());
    }
    double bar_size = (all_max - all_min) / num_bars;

    int max_hist = 0;
    for (uint a = 0; a < arrays.size(); ++a){
        auto &array = arrays[a];
        QString category = categories[a];
        QColor color = colors ? colors->at(a) : Qt::red;

        vector<int> counts(num_bars + 1, 0);
        for (auto val : array){
            max_hist = max(max_hist, ++counts[int((val - all_min) / bar_size)]);
        }

        for (uint i = 0; i <= num_bars; ++i){
            if (counts[i]) boxes.push_back(HistogramBox(&(chart->scene), all_min + bar_size * (double)i, bar_size, counts[i], color, true));
        }

        chart->legend_categories.push_back(LegendCategory(category, color, &chart->scene));
    }

    chart->xrange.setX(min(all_min - bar_size / 2., chart->xrange.x()));
    chart->xrange.setY(max(all_max + bar_size / 2., chart->xrange.y()));
    chart->yrange.setX(0);
    chart->yrange.setY(max(max_hist * 1.025, chart->yrange.y()));


}
