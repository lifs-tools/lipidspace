#include "lipidspace/barplot.h"


HoverRectItem::HoverRectItem(QString _label, QGraphicsItem *parent) : QGraphicsRectItem(parent), label(_label){

}

void HoverRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsRectItem::hoverEnterEvent(event);
    QToolTip::showText(QCursor::pos(), label);
}


void HoverRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsRectItem::hoverLeaveEvent(event);
    QToolTip::hideText();
}


Barplot::Barplot(Chart *_chart) : Chartplot(_chart) {
}

Barplot::~Barplot(){
}


void Barplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);

    for (uint b = 0; b < bars.size(); ++b){
        vector< BarBox > &barset = bars[b];

        for (uint s = 0; s < barset.size(); ++s){
            double xs = b + (double)s / (double)barset.size();
            double xe = b + (double)(s + 1) / (double)barset.size();

            BarBox &bar = barset[s];
            double x1 = xs + (xe - xs) / 4.;
            double y1 = bar.value + bar.error;
            double x2 = xe - (xe - xs) / 4.;
            double y2 = bar.value + bar.error;
            chart->translate(x1, y1);
            chart->translate(x2, y2);
            bool reasonable = (x2 - x1) > 3;
            bar.upper_error_line->setLine(x1, y1, x2, y2);
            bar.upper_error_line->setZValue(100);
            bar.upper_error_line->setVisible(visible & reasonable);

            x1 = xs + (xe - xs) / 4.;
            y1 = bar.value - bar.error;
            x2 = xe - (xe - xs) / 4.;
            y2 = bar.value - bar.error;
            chart->translate(x1, y1);
            chart->translate(x2, y2);
            bar.lower_error_line->setLine(x1, y1, x2, y2);
            bar.lower_error_line->setZValue(100);
            bar.lower_error_line->setVisible(visible & reasonable);

            x1 = (xs + xe) / 2.0;
            y1 = bar.value - bar.error;
            x2 = (xs + xe) / 2.0;
            y2 = bar.value + bar.error;
            chart->translate(x1, y1);
            chart->translate(x2, y2);
            bar.base_line->setLine(x1, y1, x2, y2);
            bar.base_line->setZValue(90);
            bar.base_line->setVisible(visible & reasonable);

            x1 = xs;
            y1 = bar.value;
            x2 = xe;
            y2 = 0;
            chart->translate(x1, y1);
            chart->translate(x2, y2);
            bar.rect->setBrush(QBrush(bar.color));
            bar.rect->setRect(x1, y1, x2 - x1, y2 - y1);
            bar.rect->setZValue(50);
            bar.rect->setVisible(visible);
        }
    }
}


void Barplot::clear(){
    bars.clear();
}


void Barplot::add(vector< vector< pair<double, double> > > &data, vector<QString> &categories, vector<QString> &labels, vector<QColor> *colors){
    if (bars.size() > 0 || data.size() == 0 || data.size() != labels.size() || (colors != 0 && categories.size() != colors->size())) return;

    for (auto category_data : data){
        if (category_data.size() != categories.size()) return;
    }

    chart->create_y_numerical_axis();
    chart->create_x_nominal_axis();


    double ymax = chart->yrange.y();
    for (uint s = 0; s < data.size(); ++s){
        auto data_set = data[s];
        bars.push_back(vector< BarBox >());
        vector< BarBox > &bar_set = bars.back();


        for (uint c = 0; c < data_set.size(); c++){
            QColor color = (colors != 0) ? colors->at(c) : Qt::white;
            auto values = data_set[c];

            double value = values.first;
            double error = values.second;
            if (isnan(value) || isinf(value)) value = 0;
            if (isnan(error) || isinf(error)) error = 0;
            ymax = max(ymax, values.first + values.second);

            bar_set.push_back(BarBox(&(chart->scene), value, error, labels[s], color));
        }
        chart->add_category(labels[s]);
    }

    chart->xrange = QPointF(0, bars.size());
    chart->yrange = QPointF(0, ymax);

    for (uint i = 0; i < categories.size(); ++i){
        QColor color = ((colors != 0) && (colors->size() > i)) ? colors->at(i) : Qt::white;
        chart->legend_categories.push_back(LegendCategory(categories[i], color, &chart->scene));
    }
}
