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


Barplot::Barplot(Chart *_chart, bool _log_scale, bool _show_data) : Chartplot(_chart) {
    log_scale = _log_scale;
    show_data = _show_data;
}

Barplot::~Barplot(){
}


void Barplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);

    for (uint b = 0; b < bars.size(); ++b){
        vector< BarBox > &barset = bars[b];

        for (uint s = 0; s < barset.size(); ++s){
            BarBox &bar = barset[s];
            if (visible && (bar.value > 0)){
                double xs = b + (double)s / (double)barset.size();
                double xe = b + (double)(s + 1) / (double)barset.size();

                double x1 = xs + (xe - xs) / 4.;
                double y1 = bar.value + bar.error;
                double x2 = xe - (xe - xs) / 4.;
                double y2 = bar.value + bar.error;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bool lines_visible = (x2 - x1) > 3;
                bar.upper_error_line->setLine(x1, y1, x2, y2);
                bar.upper_error_line->setZValue(100);
                bar.upper_error_line->setVisible(visible & lines_visible);

                x1 = xs + (xe - xs) / 4.;
                y1 = bar.value - bar.error;
                x2 = xe - (xe - xs) / 4.;
                y2 = bar.value - bar.error;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar.lower_error_line->setLine(x1, y1, x2, y2);
                bar.lower_error_line->setZValue(100);
                bar.lower_error_line->setVisible(visible & lines_visible);

                x1 = (xs + xe) / 2.0;
                y1 = bar.value - bar.error;
                x2 = (xs + xe) / 2.0;
                y2 = bar.value + bar.error;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar.base_line->setLine(x1, y1, x2, y2);
                bar.base_line->setZValue(90);
                bar.base_line->setVisible(visible & lines_visible);

                x1 = xs;
                y1 = bar.value;
                x2 = xe;
                y2 = 0;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar.rect->setBrush(QBrush(bar.color));
                bar.rect->setPen(lines_visible ? QPen(Qt::black) : Qt::NoPen);
                bar.rect->setRect(x1, y1, x2 - x1, y2 - y1);
                bar.rect->setZValue(50);
                bar.rect->setVisible(visible);

                if (bar.data.size()){
                    for (uint d = 0; d < bar.data.size(); ++d){
                        auto ellipse = bar.dots[d];
                        x1 = (xs + xe) / 2.0 + bar.data[d].second * (xe - xs) / 2.0 * 0.8;
                        y1 = bar.data[d].first;
                        chart->translate(x1, y1);
                        ellipse->setBrush(QBrush(QColor(0, 0, 0, 128)));
                        ellipse->setPen(QPen(QColor(0, 0, 0, 128)));
                        ellipse->setRect(x1 - 3, y1 - 3, 6, 6);
                        ellipse->setZValue(110);
                        ellipse->setVisible(visible & lines_visible);
                    }
                }
            }
            else {
                bar.upper_error_line->setVisible(false);
                bar.lower_error_line->setVisible(false);
                bar.base_line->setVisible(false);
                bar.rect->setVisible(false);
                for (auto ellipse : bar.dots) ellipse->setVisible(false);
            }
        }
    }
}


void Barplot::clear(){
    bars.clear();
}



void Barplot::add(vector< vector< Array > > &data, vector<QString> &categories, vector<QString> &labels, vector<QColor> *colors){
    if (bars.size() > 0 || data.size() == 0 || data.size() != labels.size() || (colors != 0 && categories.size() != colors->size())) return;

    for (auto category_data : data){
        if (category_data.size() != categories.size()) return;
    }


    double ymin = chart->yrange.x();
    double ymax = chart->yrange.y();
    for (uint s = 0; s < data.size(); ++s){
        auto data_set = data[s];
        bars.push_back(vector< BarBox >());
        vector< BarBox > &bar_set = bars.back();


        for (uint c = 0; c < data_set.size(); c++){
            QColor color = (colors != 0) ? colors->at(c) : Qt::white;
            auto values = data_set[c];

            double mean = values.mean();
            double error = values.stdev();
            if (isnan(mean) || isinf(mean)) mean = 0;
            if (isnan(error) || isinf(error)) error = 0;
            if (mean > 0) ymin = min(ymin, mean);
            ymax = max(ymax, mean + error);

            vector< pair<double, double> > *orig_data = 0;

            if (show_data){
                orig_data = new vector< pair<double, double> >();
                // distribute the data around the center
                int n = values.size();
                Array X(n, 0);
                Array Y;
                for (auto value : values) Y.push_back(value);

                double mue = Y.mean();
                double sigma = Y.sample_stdev();

                for (int i = 0; i < n; ++i){
                    X[i] = randnum() * 0.002 - 0.001;
                    Y[i] = (Y[i] - mue) / sigma;
                }
                double max_x = 0;
                Array xx(n, 0);
                for (int i = 0; i < n; ++i){
                    double fx = 0;
                    for (int j = 0; j < n; ++j){
                        if (i == j) continue;

                        double d = sq(X[i] - X[j]) + sq(Y[i] - Y[j]);
                        fx += (X[j] - X[i]) * exp(-d);
                    }
                    xx[i] = X[i] - fx * exp(-sq(fx));
                }
                for (int i = 0; i < n; ++i){
                    X[i] = xx[i];
                    max_x = max(max_x, abs(X[i]));
                }

                for (int i = 0; i < n; ++i){
                    orig_data->push_back({values[i], X[i] / max_x});
                    ymax = max(ymax, values[i]);
                }
            }

            bar_set.push_back(BarBox(&(chart->scene), mean, error, labels[s], color, orig_data));

            if (show_data) delete orig_data;
        }
        chart->add_category(labels[s]);
    }

    chart->xrange = QPointF(0, bars.size());
    if (log_scale && ymin > 0) ymin = pow(10, floor(log(ymin) / log(10)));
    chart->yrange = QPointF(log_scale ? ymin : 0, ymax);

    for (uint i = 0; i < categories.size(); ++i){
        QColor color = ((colors != 0) && (colors->size() > i)) ? colors->at(i) : Qt::white;
        chart->legend_categories.push_back(LegendCategory(categories[i], color, &chart->scene));
    }

    chart->create_y_numerical_axis(log_scale);
    chart->create_x_nominal_axis();
}
