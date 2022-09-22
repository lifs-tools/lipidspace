#include "lipidspace/plots.h"


void HoverRectSignal::sendSignalEnter(string lipid_name){
    emit enterLipid(lipid_name);
}



void HoverRectSignal::sendSignalExit(){
    emit exitLipid();
}




DataCloud::DataCloud(Chart *_chart, SortVector<double, double> *_data) {
    chart = _chart;
    for (auto data_point : *_data){
        data.push_back({data_point.first, data_point.second});
    }
    data.sort_asc();
    x_pos = 0;
    x_offset = 0;
    y_pos = 0;
    y_offset = 0;
}




QRectF DataCloud::boundingRect() const {
    return QRectF(0, 0, chart->width(), chart->height());
}


void DataCloud::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setBrush(QColor(60, 60, 60));
    painter->setPen(Qt::NoPen);
    for (auto point : data){
        double x = x_pos + point.second * x_offset;
        double y = y_pos + (point.first - y_pos) * y_offset;
        chart->translate(x, y);
        painter->drawEllipse(x - 2, y - 2, 4, 4);
    }
}


void DataCloud::setNewPosition(double _x_pos, double _x_offset, double _y_pos, double _y_offset){
    x_pos = _x_pos;
    x_offset = _x_offset;
    y_pos = _y_pos;
    y_offset = _y_offset;
}



BarBox::BarBox(Chart *chart, double _value, double _error, QString _label, QColor _color, SortVector<double, double> *_data) {
    value = _value;
    error = _error;
    color = _color;

    upper_error_line = new QGraphicsLineItem();
    lower_error_line = new QGraphicsLineItem();
    base_line = new QGraphicsLineItem();
    data_cloud = new DataCloud(chart, _data);

    upper_error_line->setZValue(100);
    lower_error_line->setZValue(100);
    base_line->setZValue(100);
    data_cloud->setZValue(100);

    rect = new HoverRectItem(QString("%1\n%2 ± %3").arg(_label).arg(value, 0, 'f', 1).arg(error, 0, 'f', 1), _label.toStdString());
    rect->setZValue(100);
    rect->setAcceptHoverEvents(true);
    connect(&(rect->hover_rect_signal), &HoverRectSignal::enterLipid, this, &BarBox::lipidEntered);
    connect(&(rect->hover_rect_signal), &HoverRectSignal::exitLipid, this, &BarBox::lipidExited);


    chart->scene.addItem(rect);
    chart->scene.addItem(data_cloud);
    chart->scene.addItem(base_line);
    chart->scene.addItem(upper_error_line);
    chart->scene.addItem(lower_error_line);
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
    y_log_scale = _log_scale;
    show_data = _show_data;
    min_log_value = 0;
    mouse_shift_start = QPointF(-1, -1);
    shift_start = QPointF(-1, -1);
    connect(chart, &Chart::yLogScaleChanged, this, &Barplot::setYLogScale);
    connect(chart, &Chart::showDataPointsChanged, this, &Barplot::setShowDataPoints);
    connect(chart, &Chart::mouseMoved, this, &Barplot::mouseMoveEvent);
    zoom = 0;
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
            if (visible && (bar->value > 0) && chart->xrange.x() <= b && b < chart->xrange.y()){
                double xs = b + (double)s / (double)barset.size();
                double xe = b + (double)(s + 1) / (double)barset.size();

                // draw error line
                double x1 = xs + (xe - xs) / 4.;
                double y1 = (bar->value + bar->error) * animation_length;
                double x2 = xe - (xe - xs) / 4.;
                double y2 = (bar->value + bar->error) * animation_length;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bool lines_visible = (x2 - x1) > 3;
                bar->upper_error_line->setLine(x1, y1, x2, y2);
                bar->upper_error_line->setVisible(lines_visible);
                QPen line_pen;
                if (show_data && lines_visible){
                    line_pen.setColor(QColor(200, 0, 0));
                    line_pen.setWidthF(2);
                }
                bar->upper_error_line->setPen(line_pen);

                // draw bar
                x1 = xs;
                y1 = bar->value * animation_length;
                x2 = xe;
                y2 = 0;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar->rect->setBrush(QBrush(bar->color));
                bar->rect->setPen(lines_visible ? QPen(Qt::black) : Qt::NoPen);
                bar->rect->setRect(x1, y1, x2 - x1, y2 - y1);
                bar->rect->setVisible(true);

                // draw lower error line
                x1 = xs + (xe - xs) / 4.;
                y1 = (bar->value - bar->error) * animation_length;
                x2 = xe - (xe - xs) / 4.;
                y2 = (bar->value - bar->error) * animation_length;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar->lower_error_line->setLine(x1, y1, x2, y2);
                bar->lower_error_line->setVisible(lines_visible);
                bar->lower_error_line->setPen(line_pen);

                // draw upper error line
                x1 = (xs + xe) / 2.0;
                y1 = (bar->value - bar->error) * animation_length;
                x2 = (xs + xe) / 2.0;
                y2 = (bar->value + bar->error) * animation_length;
                chart->translate(x1, y1);
                chart->translate(x2, y2);
                bar->base_line->setLine(x1, y1, x2, y2);
                bar->base_line->setVisible(lines_visible);
                bar->base_line->setPen(line_pen);

                // draw data points
                bar->data_cloud->setNewPosition((xs + xe) / 2.0, (xe - xs) / 2.0 * 0.8, 0, animation_length);
                bar->data_cloud->setVisible(show_data && lines_visible);
            }
            else {
                bar->upper_error_line->setVisible(false);
                bar->lower_error_line->setVisible(false);
                bar->base_line->setVisible(false);
                bar->rect->setVisible(false);
                bar->data_cloud->setVisible(false);
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


void Barplot::wheelEvent(QWheelEvent *event){
    if (bars.size() <= 1) return;
    double max_zoom = __min((int)MAX_ZOOM, bars.size() - 1);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF mouse_pos = chart->mapToScene(QPoint(event->position().x(), event->position().y()));
    zoom = __max(0, __min((int)max_zoom, zoom + (2 * (event->angleDelta().y() > 0) - 1)));
#else
    QPointF mouse_pos = chart->mapToScene(event->pos());
    zoom = __max(0, __min((int)max_zoom, zoom + (2 * (event->delta() > 0) - 1)));
#endif

    if (!chart->chart_box_inner.contains(mouse_pos)) return;

    int num_field = bars.size() - zoom;
    if (max_zoom >= MAX_ZOOM){
        double scale = -(max_zoom + 1.) / max_zoom * (1. - (double)bars.size());
        double offset = scale - (double)bars.size();
        num_field = __max(1., scale / ((double)zoom + 1) - offset);
    }

    uint left = 0;
    uint right = bars.size();
    if (num_field < (int)bars.size()){
        double x = mouse_pos.x();
        chart->back_translate(x);
        left = __max(0., round(x - (double)num_field * (x - chart->xrange.x()) / (double)(chart->xrange.y() - chart->xrange.x())));
        right = left + num_field;
        if (right > bars.size()){
            left -= right - bars.size();
            right = bars.size();
        }
    }

    chart->xrange = QPointF(left, right);

    double ymax = -INFINITY;
    for (uint b = left; b < right && b < bars.size(); ++b){
        auto bar_set = bars[b];
        for (auto bar : bar_set){
            ymax = __max(ymax, bar->value + bar->error);
            if (bar->data_cloud && bar->data_cloud->data.size()) ymax = __max(ymax, bar->data_cloud->data.back().first);
        }
    }
    chart->yrange = QPointF(chart->yrange.x(), ymax);
    chart->update_chart();
}


void Barplot::setYLogScale(bool log_scale){
    chart->yrange = QPointF(log_scale ? min_log_value : 0, chart->yrange.y());
}


void Barplot::setShowDataPoints(bool data_points){
    show_data = data_points;
}


void Barplot::mouseMoveEvent(QMouseEvent *event){
    QPointF mouse_pos = chart->mapToScene(event->pos());

    if (event->buttons() & Qt::LeftButton){
        if (chart->chart_box_inner.contains(mouse_pos)){
            if (mouse_shift_start.x() < 0){
                mouse_shift_start = mouse_pos;
                shift_start = chart->xrange;
            }
            QPointF diff = mouse_pos - mouse_shift_start;
            int len = shift_start.y() - shift_start.x();
            double diff_x = (double)diff.x() / (double)chart->chart_box_inner.width() * (double)len;

            if (shift_start.x() < diff_x) return;

            uint left = shift_start.x() - diff_x;
            uint right = left + len;

            if (right <= bars.size()){
                chart->xrange = QPointF(left, __min(right, bars.size()));

                double ymax = -INFINITY;
                for (uint b = left; b < right && b < bars.size(); ++b){
                    auto bar_set = bars[b];
                    for (auto bar : bar_set){
                        ymax = __max(ymax, bar->value + bar->error);
                        if (bar->data_cloud && bar->data_cloud->data.size()) ymax = __max(ymax, bar->data_cloud->data.back().first);
                    }
                }
                chart->yrange = QPointF(chart->yrange.x(), ymax);
                chart->update_chart();
            }
        }
    }
    else {
        mouse_shift_start = QPointF(-1, -1);
        shift_start = QPointF(-1, -1);
    }
}



void Barplot::add(vector< vector< Array > > *data, vector<QString> *categories, vector<QString> *labels, vector<QColor> *colors){
    if (bars.size() > 0 || data->size() == 0 || data->size() != labels->size() || (colors != 0 && categories->size() != colors->size())){
        delete data;
        delete categories;
        delete labels;
        delete colors;
        return;
    }

    for (auto category_data : *data){
        if (category_data.size() != categories->size()) return;
    }

    SortVector<string, int> sorted_indexes;
    for (uint i = 0; i < labels->size(); ++i) sorted_indexes.push_back({labels->at(i).toStdString(), i});
    sorted_indexes.sort_asc();

    double ymin = chart->yrange.x();
    double ymax = chart->yrange.y();
    for (uint si = 0; si < data->size(); ++si){
        uint s = sorted_indexes[si].second;
        auto data_set = data->at(s);
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

            SortVector<double, double> orig_data;
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

            int n4 = n;
            while (n4 & 3){
                X.push_back(0);
                Y.push_back(0);
                n4++;
            }

            Array xx(n, 0);
            const __m256d one = {1., 1., 1., 1.};
            const __m256d o_three = {0.30482918, 0.30482918, 0.30482918, 0.30482918};
            for (int i = 0; i < n; ++i){
                __m256d fx4 = {0., 0., 0., 0., };
                __m256d xi = {X[i], X[i], X[i], X[i]};
                __m256d yi = {Y[i], Y[i], Y[i], Y[i]};
                for (int j = 0; j < n4; j += 4){

                    __m256d xj = _mm256_loadu_pd(&X[j]);
                    __m256d yj = _mm256_loadu_pd(&Y[j]);

                    __m256d x_diff = _mm256_sub_pd(xi, xj);
                    __m256d y_diff = _mm256_sub_pd(yi, yj);

                    __m256d d = {0., 0., 0., 0.};
                    d = _mm256_fmadd_pd(x_diff, x_diff, d);
                    d = _mm256_fmadd_pd(y_diff, y_diff, d);

                    x_diff = _mm256_sub_pd(xj, xi);

                    d = _mm256_mul_pd(-d, o_three);
                    d = _mm256_sub_pd(d, one);
                    d = _mm256_mul_pd(d, d);
                    d = _mm256_mul_pd(d, d);
                    d = _mm256_div_pd(one, d);

                    fx4 = _mm256_fmadd_pd(x_diff, d, fx4);
                }
                double fx = fx4[0] + fx4[1] + fx4[2] + fx4[3];
                xx[i] = X[i] - fx / sq(sq(0.30482918 * (-sq(fx)) - 1.));
                max_x = max(max_x, abs(xx[i]));
            }

            for (int i = 0; i < n; ++i){
                orig_data.push_back({values[i], xx[i] / max_x});
                ymax = max(ymax, values[i]);
            }
            bar_set.push_back(new BarBox(chart, mean, error, labels->at(s), color, &orig_data));
            connect(bar_set.back(), &BarBox::enterLipid, this, &Barplot::lipidEntered);
            connect(bar_set.back(), &BarBox::exitLipid, this, &Barplot::lipidExited);
        }
        chart->add_category(labels->at(s));
    }

    chart->xrange = QPointF(0, bars.size());
    ymin = pow(10, floor(log(ymin) / log(10)));
    chart->yrange = QPointF(y_log_scale ? ymin : 0, ymax);
    min_log_value = ymin;

    for (uint i = 0; i < categories->size(); ++i){
        QColor color = ((colors != 0) && (colors->size() > i)) ? colors->at(i) : Qt::white;
        chart->legend_categories.push_back(LegendCategory(categories->at(i), color, &(chart->scene)));
    }


    uint lc_n = chart->legend_categories.size();
    for (uint i = 0; i < lc_n; ++i){
        for (uint j = 0; j < lc_n - 1 - i; ++j){
            if (chart->legend_categories[j].category_string.toStdString() > chart->legend_categories[j + 1].category_string.toStdString()){
                swap(chart->legend_categories[j], chart->legend_categories[j + 1]);
                for (auto &bar_set : bars){
                    swap(bar_set[j], bar_set[j + 1]);
                }
            }
        }
    }

    delete data;
    delete categories;
    delete labels;
    delete colors;

    chart->create_y_numerical_axis(y_log_scale);
    chart->create_x_nominal_axis();
    chart->reset_animation();
}











WhiskerBox::WhiskerBox(Chart *chart, SortVector<double, double> *data){
    upper_extreme_line = new QGraphicsLineItem();
    lower_extreme_line = new QGraphicsLineItem();
    median_line = new QGraphicsLineItem();
    base_line = new QGraphicsLineItem();
    rect = new QGraphicsRectItem();
    data_cloud = new DataCloud(chart, data);

    upper_extreme_line->setZValue(100);
    lower_extreme_line->setZValue(100);
    median_line->setZValue(100);
    base_line->setZValue(100);
    rect->setZValue(100);
    data_cloud->setZValue(100);

    chart->scene.addItem(upper_extreme_line);
    chart->scene.addItem(lower_extreme_line);
    chart->scene.addItem(base_line);
    chart->scene.addItem(rect);
    chart->scene.addItem(median_line);
    chart->scene.addItem(data_cloud);
    color = Qt::white;
}



Boxplot::Boxplot(Chart *_chart, bool _show_data) : Chartplot(_chart) {
    show_data = _show_data;
    connect(chart, &Chart::showDataPointsChanged, this, &Boxplot::setShowDataPoints);
}


Boxplot::~Boxplot(){
}


void Boxplot::setShowDataPoints(bool data_points){
    show_data = data_points;
}


void Boxplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = pow(chart->animation, 0.25);

    const double box_len = 0.375;
    const double whisker_len = 0.25;

    for (uint b = 0; b < boxes.size(); ++b){
        auto &box = boxes[b];
        double x1 = b - whisker_len;
        double y1 = box.median + (box.upper_extreme - box.median) * animation_length;
        double x2 = b + whisker_len;
        double y2 = box.median + (box.upper_extreme - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.upper_extreme_line->setLine(x1, y1, x2, y2);
        box.upper_extreme_line->setVisible(visible);

        x1 = b - whisker_len;
        y1 = box.median + (box.lower_extreme - box.median) * animation_length;
        x2 = b + whisker_len;
        y2 = box.median + (box.lower_extreme - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.lower_extreme_line->setLine(x1, y1, x2, y2);
        box.lower_extreme_line->setVisible(visible);

        x1 = b;
        y1 = box.median + (box.upper_extreme - box.median) * animation_length;
        x2 = b;
        y2 = box.median + (box.lower_extreme - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.base_line->setLine(x1, y1, x2, y2);
        box.base_line->setVisible(visible);

        x1 = b - box_len;
        y1 = box.median + (box.lower_quartile - box.median) * animation_length;
        x2 = b + box_len;
        y2 = box.median + (box.upper_quartile - box.median) * animation_length;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.rect->setBrush(QBrush(box.color));
        box.rect->setRect(x1, y1, x2 - x1, y2 - y1);
        box.rect->setVisible(visible);

        x1 = b - box_len;
        y1 = box.median;
        x2 = b + box_len;
        y2 = box.median;
        chart->translate(x1, y1);
        chart->translate(x2, y2);
        box.median_line->setPen(QPen());
        box.median_line->setLine(x1, y1, x2, y2);
        box.median_line->setVisible(visible);


        // draw data points
        box.data_cloud->setNewPosition(b, 1., box.median, animation_length);
        box.data_cloud->setVisible(visible && show_data);
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



    SortVector<double, double> data_points;
    for (int i = 0; i < n; ++i){
        data_points.push_back({array[i], X[i] / max_x * 0.2});
    }


    boxes.push_back(WhiskerBox(chart, &data_points));
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

    uint lc_n = chart->legend_categories.size();
    for (uint i = 0; i < lc_n; ++i){
        for (uint j = 0; j < lc_n - 1 - i; ++j){
            if (chart->legend_categories[j].category_string.toStdString() > chart->legend_categories[j + 1].category_string.toStdString()){
                swap(chart->legend_categories[j], chart->legend_categories[j + 1]);
                swap(boxes[j], boxes[j + 1]);
            }
        }
    }


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
}

Scatterplot::~Scatterplot(){

}

void Scatterplot::update_chart(){

    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
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
        points.back().p->setParentItem(chart->base);
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









HistogramBox::HistogramBox(QGraphicsScene *scene, Histogramplot *histogram_plot, double _x, double _x_width, double _y, QColor _color, bool transparent){
    x = _x;
    x_width = _x_width;
    y = _y;
    color = _color;
    if (transparent) color.setAlpha(128);

    rect = new QGraphicsRectItem();
    scene->addItem(rect);
    rect->setParentItem(histogram_plot->chart->base);
}



Histogramplot::Histogramplot(Chart *_chart) : Chartplot(_chart) {
}

Histogramplot::~Histogramplot(){
}


void Histogramplot::wheelEvent(QWheelEvent *event){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF mouse_pos = chart->mapToScene(QPoint(event->position().x(), event->position().y()));
    double zoom = (event->angleDelta().y() > 0) ? 1. / 1.1 : 1.1;
#else
    QPointF mouse_pos = chart->mapToScene(event->pos());
    double zoom = (event->delta() > 0) ? 1. / 1.1 : 1.1;
#endif

    if (!chart->chart_box_inner.contains(mouse_pos)) return;
    double x = mouse_pos.x();
    chart->back_translate(x);

    double left = chart->xrange.x();
    double right = chart->xrange.y();
    left = __max(borders.x(), x + zoom * (left - x));
    right = __min(borders.y(), x + zoom * (right - x));

    chart->xrange = QPointF(left, right);
    chart->update_chart();
}


void Histogramplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = pow(chart->animation, 0.25);

    for (auto &box : boxes){
        double x1 = box.x - box.x_width * 0.5;
        double y1 = 0;
        double x2 = box.x + box.x_width * 0.5;
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
    if (arrays.size() != categories.size() || arrays.size() != colors->size()) return;

    double all_min = INFINITY;
    double all_max = -INFINITY;
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
            int pos = __min(num_bars, __max(0, int((val - all_min) / bar_size)));
            max_hist = max(max_hist, ++counts[pos]);
        }

        for (uint i = 0; i <= num_bars; ++i){
            if (counts[i]) boxes.push_back(HistogramBox(&(chart->scene), this, all_min + bar_size * (double)i, bar_size, counts[i], color, true));
        }

        chart->legend_categories.push_back(LegendCategory(category, color, &chart->scene));
    }

    uint lc_n = chart->legend_categories.size();
    for (uint i = 0; i < lc_n; ++i){
        for (uint j = 0; j < lc_n - 1 - i; ++j){
            if (chart->legend_categories[j].category_string.toStdString() > chart->legend_categories[j + 1].category_string.toStdString()){
                swap(chart->legend_categories[j], chart->legend_categories[j + 1]);
            }
        }
    }


    borders = QPointF(min(all_min - bar_size / 2., chart->xrange.x()), max(all_max + bar_size / 2., chart->xrange.y()));
    chart->xrange.setX(min(all_min - bar_size / 2., chart->xrange.x()));
    chart->xrange.setY(max(all_max + bar_size / 2., chart->xrange.y()));
    chart->yrange.setX(0);
    chart->yrange.setY(max(max_hist * 1.025, chart->yrange.y()));

    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();
    chart->reset_animation();
}
