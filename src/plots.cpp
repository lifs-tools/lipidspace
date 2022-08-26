#include "lipidspace/plots.h"


void HoverRectSignal::sendSignalEnter(string lipid_name){
    emit enterLipid(lipid_name);
}



void HoverRectSignal::sendSignalExit(){
    emit exitLipid();
}



BarBox::BarBox(QGraphicsScene *scene, double _value, double _error, QString _label, QColor _color, vector< pair<double, double> > *_data) {
    value = _value;
    error = _error;
    color = _color;

    upper_error_line = new QGraphicsLineItem();
    lower_error_line = new QGraphicsLineItem();
    base_line = new QGraphicsLineItem();
    rect = new HoverRectItem(QString("%1\n%2 ± %3").arg(_label).arg(value, 0, 'f', 1).arg(error, 0, 'f', 1), _label.toStdString());
    rect->setAcceptHoverEvents(true);
    connect(&(rect->hover_rect_signal), &HoverRectSignal::enterLipid, this, &BarBox::lipidEntered);
    connect(&(rect->hover_rect_signal), &HoverRectSignal::exitLipid, this, &BarBox::lipidExited);

    scene->addItem(upper_error_line);
    scene->addItem(lower_error_line);
    scene->addItem(base_line);
    scene->addItem(rect);

    if (_data){
        for (uint i = 0; i < _data->size(); ++i){
            QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem();
            scene->addItem(ellipse);
            dots.push_back(ellipse);
            data.push_back({_data->at(i).first, _data->at(i).second});
        }
    }
}



void BarBox::lipidEntered(string lipid_name){
    emit enterLipid(lipid_name);
}

void BarBox::lipidExited(){
    emit exitLipid();
}


HoverRectItem::HoverRectItem(QString _label, string _lipid_name, QGraphicsItem *parent) : QGraphicsRectItem(parent), label(_label), lipid_name(_lipid_name) {

}

void HoverRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsRectItem::hoverEnterEvent(event);
    QToolTip::showText(QCursor::pos(), label);
    hover_rect_signal.sendSignalEnter(lipid_name);
}


void HoverRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsRectItem::hoverLeaveEvent(event);
    QToolTip::hideText();
    hover_rect_signal.sendSignalExit();
}


Barplot::Barplot(Chart *_chart, bool _log_scale, bool _show_data) : Chartplot(_chart) {
    log_scale = _log_scale;
    show_data = _show_data;
}

Barplot::~Barplot(){
    clear();
}


void Barplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = pow(chart->animation, 0.25);

    for (uint b = 0; b < bars.size(); ++b){
        vector< BarBox* > &barset = bars[b];

        for (uint s = 0; s < barset.size(); ++s){
            BarBox *bar = barset[s];
            if (visible && (bar->value > 0)){
                double xs = b + (double)s / (double)barset.size();
                double xe = b + (double)(s + 1) / (double)barset.size();

                double x1 = xs + (xe - xs) / 4.;
                double y1 = (bar->value + bar->error) * animation_length;
                double x2 = xe - (xe - xs) / 4.;
                double y2 = (bar->value + bar->error) * animation_length;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bool lines_visible = (x2 - x1) > 3;
                bar->upper_error_line->setLine(x1, y1, x2, y2);
                bar->upper_error_line->setZValue(100);
                bar->upper_error_line->setVisible(visible & lines_visible);

                x1 = xs + (xe - xs) / 4.;
                y1 = (bar->value - bar->error) * animation_length;
                x2 = xe - (xe - xs) / 4.;
                y2 = (bar->value - bar->error) * animation_length;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar->lower_error_line->setLine(x1, y1, x2, y2);
                bar->lower_error_line->setZValue(100);
                bar->lower_error_line->setVisible(visible & lines_visible);

                x1 = (xs + xe) / 2.0;
                y1 = (bar->value - bar->error) * animation_length;
                x2 = (xs + xe) / 2.0;
                y2 = (bar->value + bar->error) * animation_length;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar->base_line->setLine(x1, y1, x2, y2);
                bar->base_line->setZValue(90);
                bar->base_line->setVisible(visible & lines_visible);

                x1 = xs;
                y1 = bar->value * animation_length;
                x2 = xe;
                y2 = 0;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar->rect->setBrush(QBrush(bar->color));
                bar->rect->setPen(lines_visible ? QPen(Qt::black) : Qt::NoPen);
                bar->rect->setRect(x1, y1, x2 - x1, y2 - y1);
                bar->rect->setZValue(50);
                bar->rect->setVisible(visible);

                if (bar->data.size()){
                    for (uint d = 0; d < bar->data.size(); ++d){
                        auto ellipse = bar->dots[d];
                        x1 = (xs + xe) / 2.0 + bar->data[d].second * (xe - xs) / 2.0 * 0.8;
                        y1 = bar->data[d].first * animation_length;
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
                bar->upper_error_line->setVisible(false);
                bar->lower_error_line->setVisible(false);
                bar->base_line->setVisible(false);
                bar->rect->setVisible(false);
                for (auto ellipse : bar->dots) ellipse->setVisible(false);
            }
        }
    }
}


void Barplot::clear(){
    for (auto bar_set : bars){
        for (auto bar : bar_set) delete bar;
    }
    bars.clear();
}


void Barplot::lipidEntered(string lipid_name){
    emit enterLipid(lipid_name);
}

void Barplot::lipidExited(){
    emit exitLipid();
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
        bars.push_back(vector< BarBox* >());
        vector< BarBox* > &bar_set = bars.back();


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

            bar_set.push_back(new BarBox(&(chart->scene), mean, error, labels[s], color, orig_data));
            connect(bar_set.back(), &BarBox::enterLipid, this, &Barplot::lipidEntered);
            connect(bar_set.back(), &BarBox::exitLipid, this, &Barplot::lipidExited);

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
    chart->reset_animation();
}







WhiskerBox::WhiskerBox(QGraphicsScene *scene){
    upper_extreme_line = new QGraphicsLineItem();
    lower_extreme_line = new QGraphicsLineItem();
    median_line = new QGraphicsLineItem();
    base_line = new QGraphicsLineItem();
    rect = new QGraphicsRectItem();

    scene->addItem(upper_extreme_line);
    scene->addItem(lower_extreme_line);
    scene->addItem(median_line);
    scene->addItem(base_line);
    scene->addItem(rect);
    color = Qt::white;
}



Boxplot::Boxplot(Chart *_chart, bool _show_data) : Chartplot(_chart) {
    show_data = _show_data;
}

Boxplot::~Boxplot(){
}

void Boxplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = pow(chart->animation, 0.25);

    for (uint b = 0; b < boxes.size(); ++b){
        auto &box = boxes[b];
        double x1 = b - 0.125;
        double y1 = box.median + (box.upper_extreme - box.median) * animation_length;
        double x2 = b + 0.125;
        double y2 = box.median + (box.upper_extreme - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.upper_extreme_line->setLine(x1, y1, x2, y2);
        box.upper_extreme_line->setZValue(0);
        box.upper_extreme_line->setVisible(visible);

        x1 = b - 0.125;
        y1 = box.median + (box.lower_extreme - box.median) * animation_length;
        x2 = b + 0.125;
        y2 = box.median + (box.lower_extreme - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.lower_extreme_line->setLine(x1, y1, x2, y2);
        box.lower_extreme_line->setZValue(0);
        box.lower_extreme_line->setVisible(visible);

        x1 = b;
        y1 = box.median + (box.upper_extreme - box.median) * animation_length;
        x2 = b;
        y2 = box.median + (box.lower_extreme - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.base_line->setLine(x1, y1, x2, y2);
        box.base_line->setZValue(0);
        box.base_line->setVisible(visible);

        x1 = b - 0.25;
        y1 = box.median + (box.lower_quartile - box.median) * animation_length;
        x2 = b + 0.25;
        y2 = box.median + (box.upper_quartile - box.median) * animation_length;
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

        if (show_data){
            for (uint i = 0; i < box.data.size(); ++i){
                auto ellipse = box.dots[i];
                x1 = b + box.data[i].second;
                y1 = box.median + (box.data[i].first - box.median) * animation_length;
                chart->translate(x1, y1);
                ellipse->setBrush(QBrush(QColor(0, 0, 0, 128)));
                ellipse->setPen(QPen(QColor(0, 0, 0, 128)));
                ellipse->setRect(x1 - 3, y1 - 3, 6, 6);
                ellipse->setZValue(110);
                ellipse->setVisible(visible);
            }
        }
        else {
            for (auto ellipse : box.dots) ellipse->setVisible(false);
        }
    }
}


void Boxplot::clear(){
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

    if (show_data){
        // distribute the data around the center
        int n = array.size();
        Array X(n, 0);
        Array Y;
        for (auto value : array) Y.push_back(value);

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
            box.data.push_back({array[i], X[i] / max_x * 0.2});
            QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem();
            box.dots.push_back(ellipse);
            chart->scene.addItem(ellipse);
        }
    }

    chart->xrange = QPointF(-0.5, boxes.size() - 0.5);
    chart->yrange = QPointF(boxes.size() > 1 ? min(chart->yrange.x(), array.front()) : array.front(), boxes.size() > 1 ? max(chart->yrange.y(), array.back()) : array.back());

    chart->legend_categories.push_back(LegendCategory(category, box.color, &chart->scene));
    chart->create_y_numerical_axis();
    chart->reset_animation();
}








LPLine::LPLine(double _x1, double _y1, double _x2, double _y2, QColor _color){
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
    color = _color;
    line = new QGraphicsLineItem();
}



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

        lines.push_back(LPLine(x1, y1, x2, y2, _color));
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









HistogramBox::HistogramBox(QGraphicsScene *scene, double _x, double _x_width, double _y, QColor _color, bool transparent){
    x = _x;
    x_width = _x_width;
    y = _y;
    color = _color;
    if (transparent) color.setAlpha(128);

    rect = new QGraphicsRectItem();
    scene->addItem(rect);
}



Histogramplot::Histogramplot(Chart *_chart) : Chartplot(_chart) {
}

Histogramplot::~Histogramplot(){
}

void Histogramplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = pow(chart->animation, 0.25);

    for (auto &box : boxes){
        double x1 = box.x - box.x_width / 2.;
        double y1 = 0;
        double x2 = box.x + box.x_width / 2.;
        double y2 = box.y * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.rect->setRect(x1, y1, x2 - x1, y2 - y1);
        box.rect->setPen(Qt::NoPen);
        box.rect->setBrush(box.color);
        box.rect->setVisible(visible);
    }
}


void Histogramplot::clear(){
    boxes.clear();
}


void Histogramplot::add(vector<Array> &arrays, vector<QString> &categories, vector<QColor> *colors, uint num_bars){

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

    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();
    chart->reset_animation();
}
