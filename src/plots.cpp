#include "lipidspace/plots.h"


QFont Barplot::stars_font = QFont("Calibri", 14);


void HoverSignal::sendSignalEnter(string lipid_name){
    emit enterLipid(lipid_name);
}



void HoverSignal::sendSignalExit(){
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
    connect(&(rect->hover_signal), &HoverSignal::enterLipid, this, &BarBox::lipidEntered);
    connect(&(rect->hover_signal), &HoverSignal::exitLipid, this, &BarBox::lipidExited);


    rect->setParentItem(chart->base);
    base_line->setParentItem(chart->base);
    lower_error_line->setParentItem(chart->base);
    upper_error_line->setParentItem(chart->base);
    data_cloud->setParentItem(chart->base);
}



void BarBox::lipidEntered(string lipid_name){
    emit enterLipid(lipid_name);
}

void BarBox::lipidExited(){
    emit exitLipid();
}


StatTestLine::StatTestLine(Chart *_chart, double _pvalue, double _line_y){
    line = new QGraphicsLineItem();
    stars = new QGraphicsSimpleTextItem();
    pvalue = _pvalue;
    line_y = _line_y;
    line->setZValue(110);
    stars->setZValue(110);

    if (pvalue < 0.001) stars->setText("***");
    else if (pvalue < 0.01) stars->setText("**");
    else if (pvalue < 0.05) stars->setText("*");

    line->setParentItem(_chart->base);
    stars->setParentItem(_chart->base);
}


HoverRectItem::HoverRectItem(QString _label, string _lipid_name, QGraphicsItem *parent) : QGraphicsRectItem(parent), label(_label), lipid_name(_lipid_name) {

}


void HoverRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsRectItem::hoverEnterEvent(event);
    QToolTip::showText(QCursor::pos(), label);
    hover_signal.sendSignalEnter(lipid_name);
}


void HoverRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsRectItem::hoverLeaveEvent(event);
    QToolTip::hideText();
    hover_signal.sendSignalExit();
}




HoverEllipseItem::HoverEllipseItem(QString _label, string _lipid_name, QGraphicsItem *parent) : QGraphicsEllipseItem(parent), label(_label), lipid_name(_lipid_name) {

}

void HoverEllipseItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsEllipseItem::hoverEnterEvent(event);
    QToolTip::showText(QCursor::pos(), label);
    hover_signal.sendSignalEnter(lipid_name);
}


void HoverEllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsEllipseItem::hoverLeaveEvent(event);
    QToolTip::hideText();
    hover_signal.sendSignalExit();
}


Barplot::Barplot(Chart *_chart, bool _log_scale, bool _show_data, bool _show_pvalues) : Chartplot(_chart) {
    y_log_scale = _log_scale;
    show_data = _show_data;
    show_pvalues = _show_pvalues;
    min_log_value = 0.1;
    mouse_shift_start = QPointF(-1, -1);
    shift_start = QPointF(-1, -1);
    connect(chart, &Chart::yLogScaleChanged, this, &Barplot::setYLogScale);
    connect(chart, &Chart::showDataPointsChanged, this, &Barplot::setShowDataPoints);
    connect(chart, &Chart::mouseMoved, this, &Barplot::mouseMoveEvent);
    zoom = 0;
    stars_offset = 0;
    stars_font.setKerning(true);
}

Barplot::~Barplot(){
    clear();
}


void Barplot::update_chart(){
    bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
    double animation_length = pow(chart->animation, 0.25);

    QFontMetrics fm(stars_font);
    int stars_width = fm.horizontalAdvance("***");

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

        if (b >= stat_test_lines.size()) continue;
        StatTestLine &stat_test_line = stat_test_lines[b];
        QPen stat_pen;
        if (show_pvalues && stat_test_line.pvalue <= 0.05 && barset.size() >= 2 && chart->xrange.x() <= b && b < chart->xrange.y()){
            double xs = b;
            double xe = b + 1. / (double)barset.size();
            double x1 = xs + (xe - xs) / 2.;

            xs = b + ((double)barset.size() - 1.) / (double)barset.size();
            xe = b + 1.;
            double x2 = xe - (xe - xs) / 2.;

            double y1 = stat_test_line.line_y;
            double y2 = stat_test_line.line_y;

            chart->translate(x1, y1);
            chart->translate(x2, y2);
            stat_test_line.line->setLine(x1, y1, x2, y2);
            stat_test_line.line->setPen(stat_pen);
            bool lines_visible = (x2 - x1) > stars_width;

            stat_test_line.stars->setFont(stars_font);
            stat_test_line.stars->setX(x1 + (x2 - x1 - stat_test_line.stars->boundingRect().width()) * 0.5);
            stat_test_line.stars->setY(y1 - stat_test_line.stars->boundingRect().height());

            stat_test_line.line->setVisible(lines_visible);
            stat_test_line.stars->setVisible(lines_visible);
        }
        else {
            stat_test_line.line->setVisible(false);
            stat_test_line.stars->setVisible(false);
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


void Barplot::setStatResults(bool _show_pvalues){
    show_pvalues = _show_pvalues;
    update_chart();
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
        chart->back_translate_x(x);
        left = __max(0., round(x - (double)num_field * (x - chart->xrange.x()) / (double)(chart->xrange.y() - chart->xrange.x())));
        right = left + num_field;
        if (right > bars.size()){
            left -= right - bars.size();
            right = bars.size();
        }
    }

    chart->xrange = QPointF(left, right);
    recompute_hights();
}


void Barplot::setYLogScale(bool _log_scale){
    y_log_scale = _log_scale;
    chart->yrange = QPointF(y_log_scale ? min_log_value : 0, chart->yrange.y());
    recompute_hights();
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
                recompute_hights();
            }
        }
    }
    else {
        mouse_shift_start = QPointF(-1, -1);
        shift_start = QPointF(-1, -1);
    }
}



void Barplot::resizeEvent(){
    recompute_hights();
}



void Barplot::recompute_hights(){
    bool update_stat_lines = (bars.front().size() > 1) && (bars.size() == stat_test_lines.size());

    double ymax = -INFINITY;
    double all_group_max = 0;
    for (int b = chart->xrange.x(); b < chart->xrange.y(); ++b){
        auto bar_set = bars[b];
        double group_max = 0;
        for (auto bar : bar_set){
            ymax = __max(ymax, bar->value + bar->error);
            all_group_max = __max(all_group_max, bar->value + bar->error);
            group_max = __max(group_max, bar->value + bar->error);
            if (bar->data_cloud && bar->data_cloud->data.size()) ymax = __max(ymax, bar->data_cloud->data.back().first);
        }
        if (update_stat_lines) stat_test_lines[b].line_y = group_max;
    }

    QGraphicsSimpleTextItem stars;
    stars.setFont(stars_font);
    stars.setText("*");
    stars_offset = stars.boundingRect().height();
    if (y_log_scale){
        double growth = pow(all_group_max / chart->yrange.x(), 1. / (chart->chart_box_inner.height() - 2 * stars_offset));
        ymax = __max(chart->yrange.x() * pow(growth, chart->chart_box_inner.height()), ymax);
        if (update_stat_lines){
            for (int b = chart->xrange.x(); b < chart->xrange.y(); ++b){
                double exponent = log(stat_test_lines[b].line_y / chart->yrange.x()) / log(growth);
                stat_test_lines[b].line_y = chart->yrange.x() * pow(growth, exponent + stars_offset);
            }
        }
    }
    else {
        double h = all_group_max - chart->yrange.x();
        double offset = (h / (chart->chart_box_inner.height() - 2 * stars_offset) * chart->chart_box_inner.height() - h) / 2.;
        ymax = __max(all_group_max + 2 * offset, ymax);
        if (update_stat_lines){
            for (int b = chart->xrange.x(); b < chart->xrange.y(); ++b) stat_test_lines[b].line_y += offset;
        }
    }
    chart->yrange = QPointF(y_log_scale ? min_log_value : 0, ymax * 1.01);

    chart->update_chart();
}



void Barplot::add(vector< vector< Array > > *data, vector<QString> *categories, vector<QString> *labels, vector<QColor> *colors){
    if (bars.size() > 0 || data->size() == 0 || data->size() != labels->size() || (colors != 0 && categories->size() != colors->size())){
        delete data;
        delete categories;
        delete labels;
        delete colors;
        return;
    }

    QFontMetrics fm(stars_font);

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

        double group_max = 0;
        for (uint c = 0; c < data_set.size(); c++){
            QColor color = (colors != 0) ? colors->at(c) : Qt::white;
            auto values = data_set[c];

            double mean = values.mean();
            double error = values.stdev();
            if (isnan(mean) || isinf(mean)) mean = 0;
            if (isnan(error) || isinf(error)) error = 0;
            if (mean > 0) ymin = __min(ymin, mean);
            ymax = __max(ymax, mean + error);
            group_max = __max(group_max, mean + error);

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

                    __m256d xj = *(__m256d*)(X.data() + j);
                    __m256d yj = *(__m256d*)(Y.data() + j);

                    __m256d x_diff = xi - xj;
                    __m256d y_diff = yi - yj;

                    __m256d d = {0., 0., 0., 0.};
                    d = d + x_diff * x_diff;
                    d = d + y_diff * y_diff;

                    x_diff = xj - xi;

                    d = -d * o_three;
                    d = d - one;
                    d = d * d;
                    d = d * d;
                    d = one / d;

                    fx4 = fx4 + x_diff * d;
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

        if (data_set.size() == 2){
            if (data_set[0].size() && data_set[1].size()) {
                stat_test_lines.push_back(StatTestLine(chart, p_value_welch(data_set[0], data_set[1]), group_max));
            }
            else {
                stat_test_lines.push_back(StatTestLine(chart, 1, group_max));
            }
        }

        else if (data_set.size() > 2){
            vector<Array> valid_data;
            for (auto d : data_set) {
                if (d.size()) valid_data.push_back(d);
            }

            if (valid_data.size() > 1){
                stat_test_lines.push_back(StatTestLine(chart, p_value_anova(valid_data), group_max));
            }
            else {
                stat_test_lines.push_back(StatTestLine(chart, 1, group_max));
            }
        }
        chart->add_category(labels->at(s));
    }

    chart->xrange = QPointF(0, bars.size());
    ymin = pow(10, floor(log(ymin) / log(10)));
    chart->yrange = QPointF(y_log_scale ? ymin : 0, ymax * 1.01);
    recompute_hights();
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
    if (lst.size() == 1) return lst.front();

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
    if (array.empty()) return;

    sort(array.begin(), array.end());


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
    box.median = median(array, 0, n);
    box.lower_quartile = median(array, 0, n >> 1);
    box.upper_quartile = median(array, (n >> 1) + (n & 1), n);

    double lower = INFINITY;
    double upper = -INFINITY;
    for (auto &box : boxes){
        lower = __min(lower, box.lower_extreme);
        upper = __max(upper, box.upper_extreme);
    }
    double diff = upper - lower;
    lower -= diff * 1e-2;
    upper += diff * 1e-2;


    chart->xrange = QPointF(-0.5, boxes.size() - 0.5);
    chart->yrange = QPointF(lower, upper);

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



Lineplot::Lineplot(Chart *_chart, bool _is_border) : Chartplot(_chart) {
    is_border = _is_border;
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

        if (is_border) line.color.setAlpha(128);
        QPen pen(line.color);
        if (is_border){
            pen.setStyle(Qt::DashLine);
            line.line->setZValue(10000);
        }
        else pen.setWidthF(2);
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
        lines.back().line->setParentItem(chart->base);
    }

    if (!is_border){
        chart->xrange.setX(xmin);
        chart->xrange.setY(xmax);
        chart->yrange.setX(ymin);
        chart->yrange.setY(ymax);
    }

    if (category.length() > 0) chart->legend_categories.push_back(LegendCategory(category, _color, &chart->scene));
    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();
    chart->reset_animation();
}










ScPoint::ScPoint(double _x, double _y, QColor _color, QString _label, QGraphicsItem *parent) : QGraphicsEllipseItem(parent) {
    x = _x;
    y = _y;
    color = _color;
    label = _label;
}


void ScPoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsEllipseItem::hoverEnterEvent(event);
    QToolTip::showText(QCursor::pos(), label, nullptr, {}, 5000);
    emit hover_signal.enterLipid(label.toStdString());
}


void ScPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsEllipseItem::hoverLeaveEvent(event);
    QToolTip::hideText();
    emit hover_signal.exitLipid();
}


void ScPoint::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsEllipseItem::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit hover_signal.markLipid();
    }
}


Scatterplot::Scatterplot(Chart *_chart) : Chartplot(_chart) {
    connect(chart, &Chart::mouseMoved, this, &Scatterplot::mouseMoveEvent);
    mouse_shift_start = QPointF(INFINITY, INFINITY);
    shift_start_x = QPointF(INFINITY, INFINITY);
    shift_start_y = QPointF(INFINITY, INFINITY);
    zoom_start = QPointF(INFINITY, INFINITY);
}


Scatterplot::~Scatterplot(){
    clear();
}

void Scatterplot::lipidEntered(string lipid_name){
    emit enterLipid(lipid_name);
}

void Scatterplot::lipidExited(){
    emit exitLipid();
}

void Scatterplot::lipidMarked(){
    emit markLipid();
}


void Scatterplot::mouseMoveEvent(QMouseEvent *event){
    QPointF mouse_pos = chart->mapToScene(event->pos());

    if (event->buttons() & Qt::LeftButton){
        if (chart->chart_box_inner.contains(mouse_pos)){

            if (mouse_shift_start.x() == INFINITY){
                double zx = chart->chart_box_inner.width() / (chart->xrange.y() - chart->xrange.x());
                double zy = chart->chart_box_inner.height() / (chart->yrange.y() - chart->yrange.x());
                zoom_start = QPointF(zx, zy);

                shift_start_x = chart->xrange;
                shift_start_y = chart->yrange;

                mouse_shift_start = mouse_pos;
            }
            QPointF mouse_diff = mouse_pos - mouse_shift_start;
            double xl = shift_start_x.x() - mouse_diff.x() / zoom_start.x();
            double xr = shift_start_x.y() - mouse_diff.x() / zoom_start.x();
            chart->xrange = QPointF(xl, xr);

            double yt = shift_start_y.x() + mouse_diff.y() / zoom_start.y();
            double yb = shift_start_y.y() + mouse_diff.y() / zoom_start.y();
            chart->yrange = QPointF(yt, yb);
            chart->update_chart();
        }
    }
    else {
        mouse_shift_start = QPointF(INFINITY, INFINITY);
        shift_start_x = QPointF(INFINITY, INFINITY);
        shift_start_y = QPointF(INFINITY, INFINITY);
        zoom_start = QPointF(INFINITY, INFINITY);
    }
}

void Scatterplot::wheelEvent(QWheelEvent *event){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF mouse_pos = chart->mapToScene(QPoint(event->position().x(), event->position().y()));
    double zoom = (event->angleDelta().y() > 0) ? 1. / 1.1 : 1.1;
#else
    QPointF mouse_pos = chart->mapToScene(event->pos());
    double zoom = (event->delta() > 0) ? 1. / 1.1 : 1.1;
#endif



    bool update_x = false;
    bool update_y = false;
    if (chart->chart_box_inner.contains(mouse_pos)){
        update_x = true;
        update_y = true;
    }
    else if (chart->xlabel_box.contains(mouse_pos)) update_x = true;
    else if (chart->ylabel_box.contains(mouse_pos)) update_y = true;
    else if (chart->chart_box.contains(mouse_pos) && mouse_pos.x() < chart->chart_box_inner.x()) update_y = true;
    else if (chart->chart_box.contains(mouse_pos) && mouse_pos.y() > chart->chart_box_inner.y()) update_x = true;

    if (!update_x && !update_y) return;

    double x = mouse_pos.x();
    double y = mouse_pos.y();
    chart->back_translate_x(x);
    chart->back_translate_y(y);

    double left = chart->xrange.x();
    double right = chart->xrange.y();
    double top = chart->yrange.x();
    double bottom = chart->yrange.y();

    left = x + zoom * (left - x);
    right = x + zoom * (right - x);
    top = y + zoom * (top - y);
    bottom = y + zoom * (bottom - y);

    if (update_x) chart->xrange = QPointF(left, right);
    if (update_y) chart->yrange = QPointF(top, bottom);
    chart->update_chart();
}


void Scatterplot::update_chart(){
    if (points.size()){
        bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
        double animation_length = chart->animation * points.size();

        for (uint i = 0; i < points.size(); ++i){
            auto p = points[i];
            double x = p->x;
            double y = p->y;
            chart->translate(x, y);

            QPen pen(p->highlight ? Qt::black : Qt::white);
            pen.setWidthF(1.5);
            p->setPen(pen);
            p->setRect(x - 7, y - 7, 14, 14);
            p->setBrush(QBrush(p->color));
            p->setVisible(visible && (i <= animation_length));
        }
    }
    else if (point_map.size()){
        bool visible = (chart->chart_box_inner.width() > 0 && chart->chart_box_inner.height() > 0);
        double animation_length = chart->animation * point_map.size();

        int i = 0;
        for (auto &kv : point_map){
            auto p = kv.second;
            double x = p->x;
            double y = p->y;
            chart->translate(x, y);

            QPen pen(p->highlight ? Qt::black : Qt::white);
            p->setZValue(99 + p->highlight);
            pen.setWidthF(1.5);
            p->setPen(pen);
            p->setRect(x - 7, y - 7, 14, 14);
            p->setBrush(QBrush(p->color));
            p->setVisible(visible && (i <= animation_length));
        }
    }
}


void Scatterplot::clear(){
    for (auto point : points) delete point;
    for (auto &kv : point_map) delete kv.second;
    points.clear();
    point_map.clear();
}



void Scatterplot::add(vector< pair<double, double> > &data, QString category, QColor color, vector<QString>* data_labels){
    if (data_labels && data_labels->size() != data.size()) return;

    double xmin = chart->xrange.x();
    double xmax = chart->xrange.y();
    double ymin = chart->yrange.x();
    double ymax = chart->yrange.y();
    for (uint i = 0; i < data.size(); ++i){
        auto xy_point = data[i];
        QString data_label = data_labels ? data_labels->at(i) : "";

        xmin = min(xmin, xy_point.first);
        xmax = max(xmax, xy_point.first);
        ymin = min(ymin, xy_point.second);
        ymax = max(ymax, xy_point.second);

        ScPoint *sc_point = new ScPoint(xy_point.first, xy_point.second, color, data_label);
        sc_point->setAcceptHoverEvents(true);
        if (data_labels) point_map.insert({data_labels->at(i), sc_point});
        else points.push_back(sc_point);
        chart->scene.addItem(sc_point);
        sc_point->setParentItem(chart->base);
        if(data_labels){
            sc_point->setAcceptHoverEvents(true);
            connect(&(sc_point->hover_signal), &HoverSignal::enterLipid, this, &Scatterplot::lipidEntered);
            connect(&(sc_point->hover_signal), &HoverSignal::exitLipid, this, &Scatterplot::lipidExited);
            connect(&(sc_point->hover_signal), &HoverSignal::markLipid, this, &Scatterplot::lipidMarked);
        }
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









HistogramBox::HistogramBox(Histogramplot *histogram_plot, double _x, double _x_width, double _y, QColor _color, bool transparent){
    x = _x;
    x_width = _x_width;
    y = _y;
    color = _color;
    if (transparent) color.setAlpha(128);

    rect = new QGraphicsRectItem();
    rect->setZValue(100);
    rect->setParentItem(histogram_plot->chart->base);

    background_rect = new QGraphicsRectItem();
    background_rect->setZValue(0);
    background_rect->setParentItem(histogram_plot->chart->base);
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
    chart->back_translate_x(x);

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

        box.background_rect->setRect(x1, y1, x2 - x1, y2 - y1);
        box.background_rect->setPen(Qt::NoPen);
        box.background_rect->setBrush(QBrush(Qt::white));
        box.background_rect->setVisible(visible);
    }
}


void Histogramplot::clear(){
    boxes.clear();
}


void Histogramplot::add(vector<Array> &arrays, vector<QString> &categories, vector<QColor> *colors, uint num_bars){
    if (arrays.size() != categories.size() || arrays.size() != colors->size() || num_bars == 0) return;

    double all_min = INFINITY;
    double all_max = -INFINITY;

    for (auto &array : arrays){
        if (array.empty()) continue;
        sort(array.begin(), array.end());
        all_min = __min(all_min, array.front());
        all_max = __max(all_max, array.back());
    }
    if (all_min == INFINITY || all_max == -INFINITY) return;

    double bar_size = (all_max - all_min) / num_bars;
    int max_hist = 0;
    for (uint a = 0; a < arrays.size(); ++a){
        auto &array = arrays[a];
        QString category = categories[a];
        QColor color = colors ? colors->at(a) : Qt::red;

        vector<int> counts(num_bars + 1, 0);
        for (auto val : array){
            int pos = __min(num_bars, __max(int(0), int((val - all_min) / bar_size)));
            max_hist = __max(max_hist, ++counts[pos]);
        }

        for (uint i = 0; i <= num_bars; ++i){
            if (counts[i]) boxes.push_back(HistogramBox(this, all_min + bar_size * (double)i, bar_size, counts[i], color, true));
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


    borders = QPointF(__min(all_min - bar_size / 2., chart->xrange.x()), __max(all_max + bar_size / 2., chart->xrange.y()));
    chart->xrange.setX(__min(all_min - bar_size / 2., chart->xrange.x()));
    chart->xrange.setY(__max(all_max + bar_size / 2., chart->xrange.y()));
    chart->yrange.setX(0);
    chart->yrange.setY(__max(max_hist, chart->yrange.y()));

    chart->create_x_numerical_axis();
    chart->create_y_numerical_axis();
    chart->reset_animation();
}













PlottingFunction::PlottingFunction(Chart* _chart, std::function<double(double, vector<double>)> _plotting_function, vector<double> _plotting_parameters, QColor _color){
    chart = _chart;
    plotting_function = _plotting_function;
    plotting_parameters = _plotting_parameters;
    color = _color;
}


void PlottingFunction::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*){
    QPen pen(color);
    pen.setWidthF(1.5);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, true);

    double interval = (chart->xrange.y() - chart->xrange.x()) / chart->chart_box_inner.width();
    if (isnan(interval) || isinf(interval) || chart->xrange.y() <= chart->xrange.x()) return;
    QPainterPath path;
    for (double x = chart->xrange.x(); x + interval <= chart->xrange.y(); x += interval){
        double x_plot = x;
        double y_plot = plotting_function(x, plotting_parameters);
        if (isinf(x_plot) || isinf(y_plot) || isnan(x_plot) || isnan(y_plot)) break;
        chart->translate(x_plot, y_plot);
        path.lineTo(x_plot, y_plot);
    }
    painter->drawPath(path);
}

QRectF PlottingFunction::boundingRect() const {
    return chart->geometry();
}



FunctionPlot::FunctionPlot(Chart *_chart) : Chartplot(_chart) {
}

FunctionPlot::~FunctionPlot(){
}


void FunctionPlot::clear(){
    for (auto plotting_function : functions) delete plotting_function;
    functions.clear();
}


void FunctionPlot::add(std::function<double(double, vector<double>)> _plotting_function, vector<double> _plotting_parameters, QString category, QColor _color){
    functions.push_back(new PlottingFunction(chart, _plotting_function, _plotting_parameters, _color));

    chart->scene.addItem(functions.back());
    functions.back()->setParentItem(chart->base);
    if (category.length() > 0) chart->legend_categories.push_back(LegendCategory(category, _color, &chart->scene));
    update_chart();
}

void FunctionPlot::update_chart(){

}
