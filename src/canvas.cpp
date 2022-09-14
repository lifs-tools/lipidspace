#include "lipidspace/canvas.h"



Citation::Citation(const QString &text, QGraphicsItem *parent) : QGraphicsTextItem(text, parent){

}


void Citation::mousePressEvent(QGraphicsSceneMouseEvent *) {
    QString link = "https://lifs-tools.org";
    QDesktopServices::openUrl(QUrl(link));
}



HomeView::HomeView(QWidget *parent) : QGraphicsView(parent){
    firstTutorialPushButton = new QPushButton(this);
    firstTutorialPushButton->setObjectName(QString::fromUtf8("firstTutorialPushButton"));
    firstTutorialPushButton->setGeometry(QRect(60, 360, 221, 31));
    firstTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "1. Tutorial - Load tables", nullptr));
    secondTutorialPushButton = new QPushButton(this);
    secondTutorialPushButton->setObjectName(QString::fromUtf8("secondTutorialPushButton"));
    secondTutorialPushButton->setGeometry(QRect(320, 360, 221, 31));
    secondTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "2. Tutorial - UI introduction", nullptr));
    thirdTutorialPushButton = new QPushButton(this);
    thirdTutorialPushButton->setObjectName(QString::fromUtf8("thirdTutorialPushButton"));
    thirdTutorialPushButton->setGeometry(QRect(60, 400, 221, 31));
    thirdTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "3. Tutorial - Feature Selection", nullptr));
    fourthTutorialPushButton = new QPushButton(this);
    fourthTutorialPushButton->setObjectName(QString::fromUtf8("fourthTutorialPushButton"));
    fourthTutorialPushButton->setGeometry(QRect(320, 400, 221, 31));
    fourthTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "4. Tutorial - Quality control", nullptr));
    banner = 0;
}



void HomeView::resizeEvent(QResizeEvent *) {
    if (!banner) {
        banner = new QGraphicsSvgItem(QCoreApplication::applicationDirPath() + "/data/images/LipidSpace-banner.svg");
        scene()->addItem(banner);
        LIFS = new QGraphicsSvgItem(QCoreApplication::applicationDirPath() + "/data/images/LIFS.svg");
        scene()->addItem(LIFS);
        citation = new Citation("Citation: Kopczynski, Dominik et al. The Journal 47(11):08-15, 2022.");
        citation->setDefaultTextColor(Qt::white);
        scene()->addItem(citation);
        LIFS_monitor = new QGraphicsSvgItem(QCoreApplication::applicationDirPath() + "/data/images/LIFS-monitor.svg");
        scene()->addItem(LIFS_monitor);
    }

    double factor = min((double)width() / 1207., (double)height() / 483.);
    QFont font = firstTutorialPushButton->font();
    font.setPointSizeF(10 * factor);

    firstTutorialPushButton->setGeometry(QRect(60. * factor, 360. * factor, 221. * factor, 31. * factor));
    secondTutorialPushButton->setGeometry(QRect(320. * factor, 360. * factor, 221. * factor, 31. * factor));
    thirdTutorialPushButton->setGeometry(QRect(60. * factor, 400. * factor, 221. * factor, 31. * factor));
    fourthTutorialPushButton->setGeometry(QRect(320. * factor, 400. * factor, 221. * factor, 31. * factor));

    firstTutorialPushButton->setFont(font);
    secondTutorialPushButton->setFont(font);
    thirdTutorialPushButton->setFont(font);
    fourthTutorialPushButton->setFont(font);

    double banner_factor = 92. / (double)banner->boundingRect().height() * factor;
    banner->setPos(60. * factor, 85. * factor);
    banner->setScale(banner_factor);

    double LIFS_factor = 18. / (double)LIFS->boundingRect().height() * factor;
    LIFS->setPos(60. * factor, 50. * factor);
    LIFS->setScale(LIFS_factor);

    QFont f = citation->font();
    f.setPointSizeF(11.);
    citation->setFont(f);
    citation->setPos(60. * factor, 440. * factor);
    citation->setCursor(Qt::PointingHandCursor);
    citation->setScale(factor);


    double monitor_factor = 0.75;
    LIFS_monitor->setPos(width() - (564 * monitor_factor + 120) * factor, 0);
    LIFS_monitor->setScale(monitor_factor * factor);
}




DendrogramTitle::DendrogramTitle(QString t) : title(t), highlighted(false), permanent(false) {

}



DendrogramLine::DendrogramLine(QLineF line, QPen p, Dendrogram *d) : QGraphicsLineItem(line){
    setPen(p);
    next_line = 0;
    second_line = 0;
    permanent = false;
    dendrogram = d;
    setZValue(0);
    node = -1;
    d_node = 0;
}

void DendrogramLine::update_width(double w){
    QPen dpen = pen();
    dpen.setWidthF(w);
    setPen(dpen);
    if (next_line) next_line->update_width(w);
    if (second_line) second_line->update_width(w);
}


QPainterPath DendrogramLine::shape() const {
    QPainterPath path;
    double margin = 15. / dendrogram->view->transform().m11();
    if (fabs(line().x2() - line().x1()) > fabs(line().y2() - line().y1())){
        path.addRect(line().x1(), line().y1() - margin, line().x2() - line().x1(), 2 * margin);
    }
    else {
        path.addRect(line().x1() - margin, line().y1(), 2 * margin, line().y2() - line().y1());
    }
    return path;
}


void DendrogramLine::hoverEnterEvent(QGraphicsSceneHoverEvent *){
    highlight(true);
    dendrogram->update();
}

void DendrogramLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    highlight(false);
    dendrogram->update();
}

void DendrogramLine::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if (event->buttons() == Qt::LeftButton){
        if (permanent && dendrogram->top_line){
            dendrogram->top_line->make_permanent(false);
            highlight(true);
        }
        else {
            if (dendrogram->top_line) dendrogram->top_line->make_permanent(false);
            make_permanent(true);
        }
        dendrogram->update();
    }
    else if (event->buttons() == Qt::RightButton){
        if (d_node) dendrogram->highlighted_for_selection = &(d_node->indexes);
        event->setAccepted(true);
    }
}


void DendrogramLine::make_permanent(bool p){
    permanent = p;
    QPen dpen = pen();
    dpen.setColor(permanent ? Qt::red : Qt::black);
    setPen(dpen);
    if (next_line) next_line->make_permanent(p);
    if (second_line) second_line->make_permanent(p);
    if (node >= 0) dendrogram->dendrogram_titles[node].permanent = p;
}

void DendrogramLine::highlight(bool h){
    QPen dpen = pen();
    dpen.setColor((h || permanent) ? Qt::red : Qt::black);
    setPen(dpen);
    if (next_line) next_line->highlight(h);
    if (second_line) second_line->highlight(h);
    if (node >= 0) dendrogram->dendrogram_titles[node].highlighted = h;
}


void DendrogramLine::update_height_factor(double update_factor, QPointF *max_vals){
    QLineF d_line = line();
    setLine(d_line.x1(), d_line.y1() * update_factor, d_line.x2(), d_line.y2() * update_factor);
    max_vals->rx() = min(max_vals->x(), d_line.y1() * update_factor);
    max_vals->rx() = min(max_vals->x(), d_line.y2() * update_factor);
    max_vals->ry() = max(max_vals->y(), d_line.y1() * update_factor);
    max_vals->ry() = max(max_vals->y(), d_line.y2() * update_factor);
    if (next_line) next_line->update_height_factor(update_factor, max_vals);
    if (second_line) second_line->update_height_factor(update_factor, max_vals);
}


























///////////////////////////////////////////////////////////////////////////////////////
// Dendrogram methods
///////////////////////////////////////////////////////////////////////////////////////


Dendrogram::Dendrogram(LipidSpace *_lipid_space, Canvas *_view) : view(_view) {
    lipid_space = _lipid_space;
    study_variable = "";
    setZValue(100);
    top_line = 0;
    highlighted_for_selection = 0;
    ratio = 1;
    max_title_width = 0;
}


void Dendrogram::clear(){
    top_line = 0;
    view->graphics_scene.removeItem(this);
    view->graphics_scene.clear();
    view->graphics_scene.addItem(this);
    dendrogram_titles.clear();
    lines.clear();
    update();
}


void Dendrogram::add_dendrogram_lines(DendrogramNode* node, DendrogramLine* parent_line){
    if (!node) return;

    if (!node->left_child && !node->right_child){
        if (parent_line) parent_line->node = node->order;
        return;
    }

    double minx = max(1e-15, x_max_d - x_min_d);
    double miny = max(1e-15, y_max_d - y_min_d);
    double x1 = (node->x_left - x_min_d) * dwidth / minx;
    double y1 = (-node->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
    double x2 = (node->x_right - x_min_d) * dwidth / minx;
    double y2 = (-node->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
    double m = (x1 + x2) * 0.5;

    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidthF(DENDROGRAM_LINE_SIZE / view->transform().m11());

    DendrogramLine* v_line_l = new DendrogramLine(QLineF(x1, y1, m, y2), pen, this);
    view->graphics_scene.addItem(v_line_l);
    v_line_l->setAcceptHoverEvents(true);

    DendrogramLine* v_line_r = new DendrogramLine(QLineF(m, y1, x2, y2), pen, this);
    view->graphics_scene.addItem(v_line_r);
    v_line_r->setAcceptHoverEvents(true);

    if (node->left_child){
        double yl = (-node->left_child->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
        DendrogramLine* l_line = new DendrogramLine(QLineF(x1, y1, x1, yl), pen, this);
        l_line->d_node = node->left_child;
        view->graphics_scene.addItem(l_line);
        l_line->setAcceptHoverEvents(true);
        v_line_l->next_line = l_line;
        add_dendrogram_lines(node->left_child, l_line);
        if (l_line->d_node) v_line_l->d_node = l_line->d_node;
    }

    if (node->right_child){
        double yr = (-node->right_child->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
        DendrogramLine* r_line = new DendrogramLine(QLineF(x2, y2, x2, yr), pen, this);
        r_line->d_node = node->right_child;
        view->graphics_scene.addItem(r_line);
        r_line->setAcceptHoverEvents(true);
        v_line_r->next_line = r_line;
        add_dendrogram_lines(node->right_child, r_line);
        if (r_line->d_node) v_line_r->d_node = r_line->d_node;
    }

    if (parent_line){
        parent_line->next_line = v_line_l;
        parent_line->second_line = v_line_r;
    }
    else{
        top_line = new DendrogramLine(QLineF(0, 0, 0, 0), pen, this);
        top_line->next_line = v_line_l;
        top_line->second_line = v_line_r;
    }
}


void Dendrogram::load(){
    top_line = 0;
    view->graphics_scene.removeItem(this);
    view->graphics_scene.clear();
    view->graphics_scene.addItem(this);
    dendrogram_titles.clear();
    lines.clear();

    if (!lipid_space->dendrogram_root) return;

    max_title_width = 0;
    QFontMetrics fm(QFont("Helvetica", GlobalData::gui_num_var["label_size"]));
    for (int i : lipid_space->dendrogram_sorting){
        dendrogram_titles.push_back(DendrogramTitle(QString(lipid_space->selected_lipidomes[i]->cleaned_name.c_str())));
        max_title_width = max(max_title_width, (double)fm.horizontalAdvance(dendrogram_titles.back().title));
    }

    x_min_d = INFINITY;
    x_max_d = 0;
    y_min_d = INFINITY;
    y_max_d = 0;

    for (int i = 0; i < (int)lipid_space->dendrogram_points.size(); i += 4){
        lines.push_back(QLineF(lipid_space->dendrogram_points[i], -lipid_space->dendrogram_points[i + 1], lipid_space->dendrogram_points[i + 2], -lipid_space->dendrogram_points[i + 3]));

        x_min_d = min(x_min_d, lipid_space->dendrogram_points[i]);
        x_max_d = max(x_max_d, lipid_space->dendrogram_points[i]);
        x_min_d = min(x_min_d, lipid_space->dendrogram_points[i + 2]);
        x_max_d = max(x_max_d, lipid_space->dendrogram_points[i + 2]);

        y_min_d = min(y_min_d, lipid_space->dendrogram_points[i + 1]);
        y_max_d = max(y_max_d, lipid_space->dendrogram_points[i + 1]);
        y_min_d = min(y_min_d, lipid_space->dendrogram_points[i + 3]);
        y_max_d = max(y_max_d, lipid_space->dendrogram_points[i + 3]);
    }

    dendrogram_x_factor = 100;
    dendrogram_y_factor = GlobalData::gui_num_var["dendrogram_height"];
    ratio = 1;

    dwidth = (lipid_space->selected_lipidomes.size() - 1) * dendrogram_x_factor;
    dheight = dwidth * ratio;
    dendrogram_height = dheight * dendrogram_y_factor / 100. / (y_max_d - y_min_d);
    dendrogram_y_offset = y_max_d;


    double pie_radius = PIE_BASE_RADIUS * GlobalData::gui_num_var["pie_size"] / 100.;
    double x_l = max(max_title_width * 0.85, pie_radius);
    double y_b = max(max_title_width * 0.7, pie_radius);
    bound.setX(-x_l);
    bound.setY(-pie_radius);
    bound.setWidth(dwidth + x_l + pie_radius);
    bound.setHeight(dheight + y_b + pie_radius);

    add_dendrogram_lines(lipid_space->dendrogram_root);

    double w = max(bound.width(), (double)view->viewport()->width());
    double h = max(bound.height(), (double)view->viewport()->height());
    view->graphics_scene.setSceneRect(-10 * w, -10 * h, 20 * w, 20 * h);
}


Dendrogram::~Dendrogram(){

}


QRectF Dendrogram::boundingRect() const {
    return bound;
}



void Dendrogram::draw_pie(QPainter *painter, DendrogramNode *node, double threshold, double pie_x, double pie_y, LabelDirection direction){
    double resize_factor = GlobalData::gui_num_var["pie_size"] / 100.;
    double pie_radius = PIE_BASE_RADIUS * resize_factor;
    int angle_start = 16 * 90;
    QFont pie_font("Helvetica");
    pie_font.setPointSizeF(10. * resize_factor);
    if (lipid_space->study_variable_values[study_variable].study_variable_type == NominalStudyVariable){
        double sum = 0;
        for (auto kv : node->study_variable_count_nominal[study_variable]){
            sum += kv.second;
        }

        for (auto kv : node->study_variable_count_nominal[study_variable]){
            if (kv.second == 0) continue;
            int span = 16. * 360. * (double)kv.second / sum;
            QBrush brush(GlobalData::colorMapStudyVariables[study_variable + "_" + kv.first]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
            angle_start = (angle_start + span) % 5760; // 360 * 16
        }
    }
    else {
        string study_variable_le = study_variable + "_le";
        string study_variable_gr = study_variable + "_gr";

        int num = 0;
        if (uncontains_val(node->study_variable_numerical, study_variable) || node->study_variable_numerical[study_variable].size() == 0) return;
        for (double val : node->study_variable_numerical[study_variable]){
            num += val <= threshold;
        }

        int span = 16. * 360. * (double)num / (double)node->study_variable_numerical[study_variable].size();
        QBrush brush(GlobalData::colorMapStudyVariables[study_variable_le]);
        QPen piePen(brush.color());
        painter->setPen(piePen);
        painter->setBrush(brush);
        painter->setFont(pie_font);
        if (span > 0) painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);

        angle_start = (angle_start + span) % 5760;
        span = 5760 - span;
        brush.setColor(GlobalData::colorMapStudyVariables[study_variable_gr]);
        piePen.setColor(brush.color());
        painter->setPen(piePen);
        painter->setBrush(brush);
        if (span > 0) painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);


        if (direction == LabelLeft){
            double x = pie_x - pie_radius * 1.2;
            double y = pie_y;

            // drawing the legend squares
            QBrush brush(GlobalData::colorMapStudyVariables[study_variable_le]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x - 15 * resize_factor, y - 20 * resize_factor, 15 * resize_factor, 15 * resize_factor);

            brush.setColor(GlobalData::colorMapStudyVariables[study_variable_gr]);
            piePen.setBrush(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x - 15 * resize_factor, y + 5 * resize_factor, 15 * resize_factor, 15 * resize_factor);

            QPen textPen(Qt::black);
            painter->setPen(textPen);
            painter->drawText(QRectF(x - 320 * resize_factor, y - 42 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignRight, QString::number(threshold, 'g') + QString(" ") + QChar(0x2265));
            painter->drawText(QRectF(x - 320 * resize_factor, y - 18 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignRight, QString::number(threshold, 'g') + QString(" <") );



        }
        else if (direction == LabelRight){
            double x = pie_x + pie_radius * 1.2;
            double y = pie_y;

            // drawing the legend squares
            QBrush brush(GlobalData::colorMapStudyVariables[study_variable_le]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x, y - 20 * resize_factor, 15 * resize_factor, 15 * resize_factor);

            brush.setColor(GlobalData::colorMapStudyVariables[study_variable_gr]);
            piePen.setBrush(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x, y + 5 * resize_factor, 15 * resize_factor, 15 * resize_factor);



            QPen textPen(Qt::black);
            painter->setPen(textPen);
            painter->drawText(QRectF(x + 20 * resize_factor, y - 42 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignLeft, QChar(0x2264) + QString(" ") + QString::number(threshold, 'g'));
            painter->drawText(QRectF(x + 20 * resize_factor, y - 18 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignLeft, QString("> ") + QString::number(threshold, 'g'));

        }
    }

    QPen black_pen = QPen();
    black_pen.setWidthF(1. * resize_factor);
    painter->setPen(black_pen);
    painter->setBrush(QBrush());
    painter->drawEllipse(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2);

}





void Dendrogram::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (!lipid_space->dendrogram_root) return;

    QFont f("Helvetica");
    f.setPointSizeF(GlobalData::gui_num_var["label_size"]);
    painter->setFont(f);
    double dx = 0;
    double dy = 10 + (((lipid_space->selected_lipidomes.size() - 1) * dendrogram_x_factor) * ratio) * dendrogram_y_factor / 100;
    for (auto dtitle : dendrogram_titles){
        painter->save();
        painter->translate(QPointF(dx, dy));
        painter->rotate(-35);
        QPen pen((dtitle.highlighted || dtitle.permanent) ? Qt::red : Qt::black);
        painter->setPen(pen);
        painter->drawText(QRectF(-max_title_width, -GlobalData::gui_num_var["label_size"], max_title_width, GlobalData::gui_num_var["label_size"] * 2), Qt::AlignVCenter | Qt::AlignRight, dtitle.title);
        dx += dendrogram_x_factor;
        painter->restore();
    }


    if (lipid_space && lipid_space->dendrogram_root){
        if (study_variable != ""){
            recursive_paint(painter, lipid_space->dendrogram_root, GlobalData::gui_num_var["pie_tree_depth"]);

                // Draw global pie chart
            if (contains_val(lipid_space->study_variable_values, study_variable) && lipid_space->study_variable_values[study_variable].study_variable_type == NominalStudyVariable){
                int angle_start = 16 * 90;
                double sum = 0;
                DendrogramNode* node = lipid_space->dendrogram_root;
                for (auto kv : node->study_variable_count_nominal[study_variable]){
                    sum += kv.second;
                }

                double pie_radius = 15;
                QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
                double pie_x = view->width() - 25;
                double pie_y = 20;
                QTransform qtrans = view->transform();
                painter->save();
                painter->translate(QPointF(v.x(), v.y()));
                painter->scale(1. / qtrans.m11(), 1. / qtrans.m22());
                for (auto kv : node->study_variable_count_nominal[study_variable]){
                    if (kv.second == 0) continue;
                    int span = 16. * 360. * (double)kv.second / sum;
                    QBrush brush(GlobalData::colorMapStudyVariables[study_variable + "_" + kv.first]);
                    QPen piePen(brush.color());
                    painter->setPen(piePen);
                    painter->setBrush(brush);
                    painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
                    angle_start = (angle_start + span) % 5760; // 360 * 16
                }
                painter->restore();

            }


            // print the study variable legend
            QFont legend_font("Helvetica", 8);
            int num_study_variable = 0;
            for (auto study_variable_value_kv : lipid_space->study_variable_values.at(study_variable).nominal_values){
                QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
                QTransform qtrans = view->transform();
                painter->save();
                painter->translate(QPointF(v.x(), v.y()));
                painter->scale(1. / qtrans.m11(), 1. / qtrans.m22());
                if (contains_val(GlobalData::colorMapStudyVariables, study_variable + "_" + study_variable_value_kv.first)){
                    painter->fillRect(QRectF(view->viewport()->geometry().width() - 20, 40 + 20 * num_study_variable, 15, 15), GlobalData::colorMapStudyVariables[study_variable + "_" + study_variable_value_kv.first]);
                }
                painter->setFont(legend_font);
                string label_text = study_variable_value_kv.first + " (" + std::to_string(lipid_space->dendrogram_root->study_variable_count_nominal[study_variable][study_variable_value_kv.first]) + ")";
                painter->drawText(QRectF(view->viewport()->geometry().width() - 525, 40 + 20 * num_study_variable, 500, 30), Qt::AlignTop | Qt::AlignRight, label_text.c_str());

                painter->restore();
                ++num_study_variable;
            }
        }


        // Draw the title
        QFont title_font("Helvetica", 7);
        painter->setFont(title_font);
        QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
        QTransform qtrans = view->transform();
        painter->save();
        painter->translate(QPointF(v.x(), v.y()));
        painter->scale(1. / qtrans.m11(), 1. / qtrans.m22());
        painter->drawText(QRectF(2, 0, 200, 60), Qt::AlignTop | Qt::AlignLeft, "Dendrogram");
        painter->restore();
    }
}


void Dendrogram::recursive_paint(QPainter *painter, DendrogramNode *node, int max_recursions, int recursion){
    if (recursion == max_recursions || node == 0) return;

    // processing left child
    if (node->left_child != 0 && node->left_child->indexes.size() > 0){
        double pie_x = node->x_left * dendrogram_x_factor;
        double pie_y = (-node->y + dendrogram_y_offset) * dendrogram_height * dendrogram_y_factor / 100.;

        draw_pie(painter, node->left_child, node->study_variable_numerical_thresholds[study_variable], pie_x, pie_y, LabelLeft);
        recursive_paint(painter, node->left_child, max_recursions, recursion + 1);
    }

    // processing right child
    if (node->right_child != 0 && node->right_child->indexes.size() > 0){
        double pie_x = node->x_right * dendrogram_x_factor;
        double pie_y = (-node->y + dendrogram_y_offset) * dendrogram_height * dendrogram_y_factor / 100.;

        draw_pie(painter, node->right_child, node->study_variable_numerical_thresholds[study_variable], pie_x, pie_y, LabelRight);
        recursive_paint(painter, node->right_child, max_recursions, recursion + 1);
    }
}


void Dendrogram::update_bounds(bool complete){
    double tmp_factor = dendrogram_y_factor;
    dendrogram_y_factor = GlobalData::gui_num_var["dendrogram_height"];
    if (isinf(dendrogram_y_factor) || isnan(dendrogram_y_factor) || isinf(tmp_factor) || isnan(tmp_factor) || isinf(dendrogram_y_factor / tmp_factor) || isnan(dendrogram_y_factor / tmp_factor)) return;

    max_title_width = 0;
    QFontMetrics fm(QFont("Helvetica", GlobalData::gui_num_var["label_size"]));
    for (auto dendrogram_title : dendrogram_titles){
        max_title_width = max(max_title_width, (double)fm.horizontalAdvance(dendrogram_title.title));
    }
    double pie_radius = PIE_BASE_RADIUS * GlobalData::gui_num_var["pie_size"] / 100.;

    if (complete){
        double tmp_ratio = ratio;
        ratio = (double)view->height() / (double)view->width();

        dheight = dwidth * ratio;
        dendrogram_height = dheight / (y_max_d - y_min_d);
        dendrogram_y_offset = y_max_d;

        double x_l = max(max_title_width * 0.82, pie_radius);
        double y_b = max(max_title_width * 0.62, pie_radius);

        bound.setX(-x_l);
        bound.setY(-pie_radius);
        bound.setWidth(dwidth + x_l + pie_radius);
        bound.setHeight(dheight * dendrogram_y_factor / 100. + y_b + pie_radius);


        if (top_line){
            QPointF max_vals(INFINITY, -INFINITY);
            if (dendrogram_y_factor != tmp_factor){
                top_line->update_height_factor(dendrogram_y_factor / tmp_factor, &max_vals);
            }
            else {
                top_line->update_height_factor(ratio / tmp_ratio, &max_vals);
            }
        }


        double w = max(bound.width(), (double)view->viewport()->width());
        double h = max(bound.height(), (double)view->viewport()->height());
        view->graphics_scene.setSceneRect(-10 * w, -10 * h, 20 * w, 20 * h);
    }
    else {
        double pie_radius = PIE_BASE_RADIUS * GlobalData::gui_num_var["pie_size"] / 100.;
        double x_l = max(max_title_width * 0.82, pie_radius);
        double y_b = max(max_title_width * 0.62, pie_radius);
        bound.setX(-x_l);
        bound.setY(-pie_radius);
        bound.setWidth(dwidth + x_l + pie_radius);
        bound.setHeight(dheight * dendrogram_y_factor / 100. + y_b + pie_radius);
    }
    view->setBackgroundBrush(QBrush());
    update();
}
























///////////////////////////////////////////////////////////////////////////////////////
// Pointset methods
///////////////////////////////////////////////////////////////////////////////////////

double compute_scale(Lipidome *lipidome, int i){
    double f = sqrt(sq(lipidome->m(i, GlobalData::PC1)) + sq(lipidome->m(i, GlobalData::PC2)));
    //return 1. / log(f + 1.);
    return 1. / sqrt(f);
}

PointSet::PointSet(Lipidome *_lipidome, Canvas *_view) : view(_view) {
    lipidome = _lipidome;
    title = "";
    variances = "";
    setZValue(100);
}


void PointSet::loadPoints(){
    view->graphics_scene.removeItem(this);
    view->graphics_scene.clear();
    view->graphics_scene.addItem(this);

    points.clear();
    labels.clear();

    double x_min = INFINITY;
    double x_max = -INFINITY;
    double y_min = INFINITY;
    double y_max = -INFINITY;

    // we need at least three lipids to span a lipid space
    if (view->lipid_space->global_lipidome->lipids.size() <= 2) return;
    map<string, string> &translations = view->lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];

    for (uint rr = 0; rr < lipidome->selected_lipid_indexes.size(); ++rr){
        int r = lipidome->selected_lipid_indexes[rr];
        double f = compute_scale(lipidome, rr);

        double xval = lipidome->m(rr, GlobalData::PC1) * f * POINT_BASE_FACTOR;
        double yval = lipidome->m(rr, GlobalData::PC2) * f * POINT_BASE_FACTOR;
        double intens = GlobalData::showQuant ? lipidome->visualization_intensities[rr] : POINT_BASE_SIZE;
        double intens_boundery = intens * 0.5;

        x_min = __min(x_min, xval - intens_boundery);
        x_max = __max(x_max, xval + intens_boundery);
        y_min = __min(y_min, yval - intens_boundery);
        y_max = __max(y_max, yval + intens_boundery);

        points.push_back(PCPoint());
        PCPoint &pc_point = points.back();
        pc_point.point = QPointF(xval, yval);
        pc_point.normalized_intensity = lipidome->normalized_intensities[r];
        pc_point.intensity = intens;
        pc_point.color = GlobalData::colorMap[lipidome->classes[r]];
        pc_point.label = translations[lipidome->species[r]].c_str();
        pc_point.ref_lipid_species = r;

        QRectF bubble(xval - intens * 0.5, yval - intens * 0.5,  intens, intens);

        // setting up pen for painter
        QColor qcolor = GlobalData::colorMap[lipidome->classes[r]];
        qcolor.setAlpha(GlobalData::alpha);

        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(bubble);
        ellipse->setZValue(0);
        ellipse->setBrush(QBrush(qcolor));
        QPen pen(qcolor);
        pen.setWidthF(0);
        ellipse->setPen(pen);
        view->graphics_scene.addItem(ellipse);
        points.back().item = ellipse;
    }

    set_labels();

    for (auto pc_label : labels){
        x_min = min(x_min, pc_label.label_point.x());
        x_max = max(x_max, pc_label.label_point.x());
        y_min = min(y_min, pc_label.label_point.y());
        y_max = max(y_max, pc_label.label_point.y());
    }

    double x_margin = (x_max - x_min) * 0.05;
    double y_margin = (y_max - y_min) * 0.05;

    bound.setX(x_min - x_margin);
    bound.setY(y_min - y_margin);
    bound.setWidth(x_max - x_min + 2 * x_margin);
    bound.setHeight(y_max - y_min + 2 * y_margin);
}

PointSet::~PointSet(){
}


QRectF PointSet::boundingRect() const {
    return bound;
}


// is a straight line from the center of a bounding box to a target intersecting
// with the bounding box, if yes where??
bool find_start(QRectF &bound, QPointF target, QPointF &inter){
    QLineF path(bound.center().x(), bound.center().y(), target.x(), target.y());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QLineF B1(bound.x(), bound.y(), bound.x() + bound.width(), bound.y());
    if (path.intersects(B1, &inter) == QLineF::BoundedIntersection) return true;

    QLineF B2(bound.x(), bound.y(), bound.x(), bound.y() + bound.height());
    if (path.intersects(B2, &inter) == QLineF::BoundedIntersection) return true;

    QLineF B3(bound.x(), bound.y() + bound.height(), bound.x() + bound.width(), bound.y() + bound.height());
    if (path.intersects(B3, &inter) == QLineF::BoundedIntersection) return true;

    QLineF B4(bound.x() + bound.width(), bound.y(), bound.x() + bound.width(), bound.y() + bound.height());
    if (path.intersects(B4, &inter) == QLineF::BoundedIntersection) return true;

#else

    QLineF B1(bound.x(), bound.y(), bound.x() + bound.width(), bound.y());
    if (path.intersect(B1, &inter) == QLineF::BoundedIntersection) return true;

    QLineF B2(bound.x(), bound.y(), bound.x(), bound.y() + bound.height());
    if (path.intersect(B2, &inter) == QLineF::BoundedIntersection) return true;

    QLineF B3(bound.x(), bound.y() + bound.height(), bound.x() + bound.width(), bound.y() + bound.height());
    if (path.intersect(B3, &inter) == QLineF::BoundedIntersection) return true;

    QLineF B4(bound.x() + bound.width(), bound.y(), bound.x() + bound.width(), bound.y() + bound.height());
    if (path.intersect(B4, &inter) == QLineF::BoundedIntersection) return true;
#endif
    return false;
}


void PointSet::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();

    // print highlighted points on top
    for (int i = 0; i < (int)points.size(); ++i){
        if (uncontains_val(highlighted_points, points[i].label)) continue;

        double intens = GlobalData::showQuant ? points[i].intensity : 1.;
        QRectF bubble(points[i].point.x() - intens * 0.5, points[i].point.y() - intens * 0.5,  intens, intens);
        if (!v.intersects(bubble)) continue;

        // setting up pen for painter
        QColor qcolor = points[i].color;
        qcolor.setAlpha(255);
        QPainterPath p;
        p.addEllipse(bubble);
        painter->fillPath(p, qcolor);
        QPen pen_black(Qt::black);
        pen_black.setWidthF(0.1);
        painter->setPen(pen_black);
        painter->drawEllipse(bubble);
    }



    // drawing the labels
    QPen pen_arr;
    pen_arr.setColor(QColor(LABEL_COLOR));
    pen_arr.setStyle(Qt::DashLine);
    pen_arr.setWidthF(10);


    QFont f("Helvetica");
    f.setPointSizeF(1.2);
    painter->setFont(f);
    for (auto pc_label : labels){
        painter->setPen(pen_arr);
        QRectF labelPosition(pc_label.label_point.x() - 50, pc_label.label_point.y() - 20, 100, 40);
        QRectF boundingRect;
        painter->drawText(labelPosition, Qt::AlignCenter, pc_label.label, &boundingRect);


        // shrink a little bit the bounding box
        boundingRect = boundingRect.marginsRemoved(QMarginsF(0, boundingRect.height() * 0.3, 0, boundingRect.height() * 0.2));


        // Search for label arrow starting point
        QPointF new_start;
        bool found = find_start(boundingRect, pc_label.class_mean, new_start);
        if (!found) continue;

        double hypothenuse = sqrt(sq(new_start.x() - pc_label.class_mean.x()) + sq(new_start.y() - pc_label.class_mean.y()));
        double angle = asin((new_start.y() - pc_label.class_mean.y()) / hypothenuse) / M_PI * 180.;


        // draw the arrow
        QPointF rotate_point = new_start.x() < pc_label.class_mean.x() ? new_start : pc_label.class_mean;
        double sign = 1. - 2. * (new_start.x() < pc_label.class_mean.x());

        painter->save();
        painter->translate(rotate_point);
        painter->rotate(sign * angle);
        painter->setPen(pen_arr);
        painter->scale(0.01, 0.01);
        QRectF rectangle(0, -hypothenuse * 0.1 * 100, hypothenuse * 100, hypothenuse * 0.2 * 100);
        double const startAngle = 16. * 20.;
        double const endAngle   = 16. * 160.;
        double const spanAngle = endAngle - startAngle;
        painter->drawArc(rectangle, startAngle, spanAngle);
        painter->restore();
    }



    // Draw the title and the variances
    QFont title_font("Helvetica", 7);
    painter->setFont(title_font);
    QTransform qtrans = view->transform();
    painter->save();
    painter->translate(QPointF(v.x(), v.y()));
    painter->scale(1. / qtrans.m11(), 1. / qtrans.m22());
    painter->setPen(Qt::black);
    painter->drawText(QRectF(2, 0, 200, 60), Qt::AlignTop | Qt::AlignLeft, title);
    painter->drawText(QRectF(2, view->viewport()->geometry().height() - 60, 200, 60), Qt::AlignBottom | Qt::AlignLeft, variances);
    painter->restore();


    // Draw marking rect
    if (view->marked_for_selected_view && !GlobalData::selected_view){
        painter->save();
        painter->translate(QPointF(v.x(), v.y()));
        painter->scale(1. / qtrans.m11(), 1. / qtrans.m22());
        painter->setPen(Qt::black);
        painter->drawRect(0, 0, view->width() - 1, view->height() -1);
        painter->restore();
    }
}


void PointSet::set_point_size(){
    for (auto pc_point : points){
        int rr = pc_point.ref_lipid_species;
        double f = compute_scale(lipidome, rr);

        double xval = lipidome->m(rr, GlobalData::PC1) * f * POINT_BASE_FACTOR;
        double yval = lipidome->m(rr, GlobalData::PC2) * f * POINT_BASE_FACTOR;
        double intens = GlobalData::showQuant ? pc_point.intensity : POINT_BASE_SIZE;

        QRectF bubble(xval - intens * 0.5, yval - intens * 0.5,  intens, intens);
        points[rr].item->setRect(bubble);
    }
}


double pairwise_sum(Matrix &m){
    assert(m.cols == 2);
    Matrix tm(m, true);

    double dist_sum = 0;
    for (int tm2c = 0; tm2c < tm.cols; tm2c++){
        double* tm2col = tm.data() + (tm2c * tm.rows);
        for (int tm1c = 0; tm1c < tm.cols; ++tm1c){
            double* tm1col = tm.data() + (tm1c * tm.rows);
            double dist = sq(tm1col[0] - tm2col[0]);
            dist += sq(tm1col[1] - tm2col[1]);
            dist_sum += sqrt(dist);
        }
    }
    return dist_sum;
}



void PointSet::set_labels(){
    if (lipidome->m.rows == 0 || lipidome->m.cols == 0) return;
    set<LipidAdduct*> selected_lipids;
    for (auto lipid : view->lipid_space->global_lipidome->lipids) selected_lipids.insert(lipid);

    map<string, vector<int>> indexes;
    for (int i = 0, j = 0; i < (int)lipidome->lipids.size(); ++i){
        if (uncontains_val(selected_lipids, lipidome->lipids[i])) continue;
        string lipid_class = lipidome->lipids.at(i)->get_extended_class();
        if (uncontains_val(indexes, lipid_class)) indexes.insert({lipid_class, vector<int>()});
        indexes.at(lipid_class).push_back(j++);
    }
    Array mean_x;
    Array mean_y;

    // plot the dots
    for (auto kv : indexes){
        double mx = 0, my = 0;
        for (auto i : kv.second){
            double f = compute_scale(lipidome, i);
            //mx += sign_log(lipidome->m(i, GlobalData::PC1)) * POINT_BASE_FACTOR;
            //my += sign_log(lipidome->m(i, GlobalData::PC2)) * POINT_BASE_FACTOR;
            mx += lipidome->m(i, GlobalData::PC1) * f * POINT_BASE_FACTOR;
            my += lipidome->m(i, GlobalData::PC2) * f * POINT_BASE_FACTOR;
        }
        mean_x.push_back(mx / (double)kv.second.size());
        mean_y.push_back(my / (double)kv.second.size());
        labels.push_back(PCLabel());
        labels.back().class_mean = QPointF(mx / (double)kv.second.size(), my / (double)kv.second.size());

        stringstream label;
        label << kv.first << " (" << kv.second.size() << ")";
        labels.back().label = QString(label.str().c_str());
    }

    for (int i = 0; i < lipidome->m.rows; ++i){
        double f = compute_scale(lipidome, i);
        //mean_x.push_back(sign_log(lipidome->m(i, 0)) * POINT_BASE_FACTOR);
        //mean_y.push_back(sign_log(lipidome->m(i, 1)) * POINT_BASE_FACTOR);
        mean_x.push_back(lipidome->m(i, 0) * f * POINT_BASE_FACTOR);
        mean_y.push_back(lipidome->m(i, 1) * f * POINT_BASE_FACTOR);
    }


    // plot the annotations
    automated_annotation(mean_x, mean_y);

}



void PointSet::automated_annotation(Array &xx, Array &yy){
    int l = labels.size();
    Array label_xx(xx, l);
    Array label_yy(yy, l);
    for (int i = 0; i < l; ++i){
        label_xx(i) += randnum() * 0.5 - 0.25;
        label_yy(i) += randnum() * 0.5 - 0.25;
    }
    Array orig_label_xx(label_xx);
    Array orig_label_yy(label_yy);

    double sigma_x = xx.stdev();
    double sigma_y = yy.stdev();

    Array all_xx(label_xx);
    all_xx.add(xx);
    Array all_yy(label_yy);
    all_yy.add(yy);
    Matrix r;
    r.add_column(all_xx);
    r.add_column(all_yy);


    double ps = pairwise_sum(r) / sq(all_xx.size());
    double nf_x = 0.5 * sigma_x / ps; // normalization factor
    double nf_y = 0.5 * sigma_y / ps; // normalization factor


    // do 30 iterations to find an equilibrium of pulling and pushing forces
    // for the label positions
    for (int rep = 0; rep < 30; ++rep){
        Array new_xx(l, 0);
        Array new_yy(l, 0);
        for (int ii = 0; ii < l; ++ii){
            double l_xx = label_xx[ii];
            double l_yy = label_yy[ii];

            Array distances;
            distances.compute_distances(all_xx, l_xx, all_yy, l_yy);

            // apply pushing force
            double force_x = 0, force_y = 0;
            for (int i = 0; i < (int)all_xx.size(); ++i){
                force_x += ((i < l) ? 50.0 : 1.) * nf_x * (all_xx(i) - l_xx) / (distances(i) + 1e-16);
                force_y += ((i < l) ? 30.0 : 1.) * nf_y * (all_yy(i) - l_yy) / (distances(i) + 1e-16);
            }

            l_xx -= force_x;
            l_yy -= force_y;

            // apply pulling force
            new_xx(ii) = orig_label_xx(ii) + (l_xx - orig_label_xx(ii)) * 0.6;
            new_yy(ii) = orig_label_yy(ii) + (l_yy - orig_label_yy(ii)) * 0.6;
        }
        for (int i = 0; i < l; ++i){
            label_xx(i) = new_xx(i);
            all_xx(i) = new_xx(i);
            label_yy(i) = new_yy(i);
            all_yy(i) = new_yy(i);
        }
    }

    for (int r = 0; r < (int)labels.size(); ++r){
        labels[r].label_point = QPointF(label_xx[r], label_yy[r]);
    }
}


void PointSet::updateView(QRectF current_view){
    old_view = current_view;
}


void PointSet::resize(){
    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
    double x_scaling = v.width() / old_view.width();
    double y_scaling = v.height() / old_view.height();

    double scaling = min(x_scaling, y_scaling);

    view->scale(scaling, scaling);
    view->centerOn(old_view.x() + old_view.width() * 0.5, old_view.y() + old_view.height() * 0.5);

    QRect viewportRect(0, 0, view->viewport()->width(), view->viewport()->height());
    old_view = view->mapToScene(viewportRect).boundingRect();
}






















///////////////////////////////////////////////////////////////////////////////////////
// Canvas methods
///////////////////////////////////////////////////////////////////////////////////////


Canvas::Canvas(QWidget *parent) : QGraphicsView(parent) {
    pointSet = 0;
    dendrogram = 0;
    canvas_id = -1;
    canvas_type = UndefinedCanvasType;
    hovered_for_swap = false;
    marked_for_selected_view = false;

    setDragMode(QGraphicsView::ScrollHandDrag);
    setFrameStyle(QFrame::NoFrame);
    setRenderHints(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setCursor(Qt::ArrowCursor);
    setMouseTracking(true);

    graphics_scene.setSceneRect(-5000000, -3000000, 10000000, 6000000);
    setScene(&graphics_scene);
    num = -3;
}





void Canvas::update_alpha(){
    if (pointSet){
        for (auto point : pointSet->points){
            QBrush b = point.item->brush();
            QColor c = b.color();
            c.setAlpha(GlobalData::alpha);
            b.setColor(c);
            point.item->setBrush(b);
        }
    }
}


void Canvas::setDendrogramData(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
    graphics_scene.clear();
    dendrogram = new Dendrogram(lipid_space, this);
    graphics_scene.addItem(dendrogram);
    graphics_scene.setSceneRect(-5000000, -3000000, 10000000, 6000000);
}


void Canvas::resetDendrogram(){
    dendrogram->load();
    QResizeEvent resize(QSize(width(), height()), QSize(width(), height()));
    emit resizeEvent(&resize);
}


void Canvas::setLabelSize(int font_size){
    GlobalData::gui_num_var["label_size"] = font_size;
    if (dendrogram) dendrogram->update_bounds(false);
}


void Canvas::setDendrogramHeight(){
    if (dendrogram) dendrogram->update_bounds();
}


Canvas::Canvas(LipidSpace *_lipid_space, int _canvas_id, int _num, QListWidget* _listed_species, CanvasType _canvas_type, QWidget *) : num(_num) {
    lipid_space = _lipid_space;
    listed_species = _listed_species;
    canvas_type = _canvas_type;
    canvas_id = _canvas_id;

    pointSet = 0;
    dendrogram = 0;
    hovered_for_swap = false;
    marked_for_selected_view = false;

    setDragMode(QGraphicsView::ScrollHandDrag);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setCursor(Qt::ArrowCursor);
    setMouseTracking(true);

    graphics_scene.setSceneRect(-5000000, -3000000, 10000000, 6000000);
    setScene(&graphics_scene);



    showQuant = true;
    // set the graphics item within this graphics view
    if (canvas_type == DendrogramCanvas){ // dendrogram
        dendrogram = new Dendrogram(lipid_space, this);
        graphics_scene.addItem(dendrogram);
        dendrogram->load();
    }
    else if (canvas_type == GlobalSpaceCanvas){ // global lipidome
        pointSet = new PointSet(lipid_space->global_lipidome, this);
        graphics_scene.addItem(pointSet);
        pointSet->title = "Global lipidome";
        pointSet->loadPoints();

        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->global_lipidome->m, vars);
        pointSet->variances = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
    }
    else if (canvas_type == StudySpaceCanvas){ // global lipidome
        pointSet = new PointSet(lipid_space->study_lipidomes[num], this);
        graphics_scene.addItem(pointSet);
        pointSet->title = QString(lipid_space->study_lipidomes[num]->cleaned_name.c_str());
        pointSet->loadPoints();

        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->study_lipidomes[num]->m, vars);
        pointSet->variances = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
    }
    else { // regular lipidome
        pointSet = new PointSet(lipid_space->selected_lipidomes[num], this);
        graphics_scene.addItem(pointSet);
        pointSet->title = QString(lipid_space->selected_lipidomes[num]->cleaned_name.c_str());
        pointSet->loadPoints();

        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->selected_lipidomes[num]->m, vars);
        pointSet->variances = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
    }
}



Canvas::~Canvas(){
}


void Canvas::clear(){
    if (dendrogram) dendrogram->clear();
}



void Canvas::highlightPoints(){
    if (pointSet){
        pointSet->highlighted_points.clear();

        for (auto item : listed_species->selectedItems()){
            pointSet->highlighted_points.insert(item->text());
        }

        pointSet->update();
    }
}


void Canvas::hoverOver(){
    if (dendrogram || canvas_type != SampleSpaceCanvas) return;

    QRectF widgetRect = QRectF(0, 0, width(), height());
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    if (widgetRect.contains(mousePos)) {
        hovered_for_swap = true;
    }
    else {
        hovered_for_swap = false;
    }
}


void Canvas::setSwap(int source){
    if (dendrogram || !hovered_for_swap) return;
    hovered_for_swap = false;
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    swappingLipidomes(source, canvas_id);
    transforming(v);

}



void Canvas::contextMenu(QPoint pos){
    context(this, pos);
}


void Canvas::mousePressEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;


    if (event->button() == Qt::LeftButton){
        if (GlobalData::ctrl_pressed){
            mouse(event, this);
        }
        else {
            QGraphicsView::mousePressEvent(event);
        }
    }
    else if (event->button() == Qt::RightButton){
        QGraphicsView::mousePressEvent(event);
        if (dendrogram){
            emit rightClick(event->pos(), dendrogram->highlighted_for_selection);
            dendrogram->highlighted_for_selection = 0;
        }
    }
    else {
        QGraphicsView::mousePressEvent(event);
    }
}



void Canvas::exportAsPdf(){
    if (!pointSet && !dendrogram) return;
    QString file_name = QFileDialog::getSaveFileName(this, "Export as pdf", GlobalData::last_folder, "*.pdf (*.pdf)");
    if (!file_name.length()) return;

    QFileInfo fi(file_name);
    GlobalData::last_folder = fi.absoluteDir().absolutePath();

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setFontEmbeddingEnabled(true);
    QPageSize pageSize(QSizeF(viewport()->width(), viewport()->height()) , QPageSize::Point);
    printer.setPageSize(pageSize);

    printer.setOutputFileName(file_name);

    // set margins to 0
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMarginsF margins;
	margins.setTop(0);
	margins.setLeft(0);
	margins.setRight(0);
	margins.setBottom(0);
    printer.setPageMargins(margins);
#else
    QPagedPaintDevice::Margins margins;
    margins.top = 0;
    margins.left = 0;
    margins.right = 0;
    margins.bottom = 0;
    printer.setMargins(margins);
#endif


    QPainter p(&printer);
    render(&p);
    p.end();

    QMessageBox::information(this, "Export completed", "The export is completed into the file '" + file_name + "'.");
}




void Canvas::mouseDoubleClickEvent(QMouseEvent *){
    if ((!dendrogram && !pointSet) || GlobalData::selected_view) return;
    marked_for_selected_view = !marked_for_selected_view;
}



void Canvas::mouseReleaseEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;

    if (!GlobalData::ctrl_pressed){
        setBackgroundBrush(QBrush());
        viewport()->setCursor(Qt::DragMoveCursor);
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        QRectF v = mapToScene(viewportRect).boundingRect();
        transforming(v);
        if (pointSet) pointSet->updateView(v);

        oldCenter.setX(v.x() + v.width() * 0.5);
        oldCenter.setY(v.y() + v.height() * 0.5);
    }
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}






void Canvas::mouseMoveEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;

    if (event->buttons() & Qt::LeftButton){
        if (!GlobalData::ctrl_pressed){
            setBackgroundBrush(QBrush());
            viewport()->setCursor(Qt::DragMoveCursor);
            QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
            QRectF v = mapToScene(viewportRect).boundingRect();
            transforming(v);
            if (pointSet) pointSet->updateView(v);

            oldCenter.setX(v.x() + v.width() * 0.5);
            oldCenter.setY(v.y() + v.height() * 0.5);
            QGraphicsView::mouseMoveEvent(event);
        }
        else {
            event->ignore();
        }
    }
    else {

        // check if mouse over lipid bubble
        if (pointSet){
            QPoint origin_mouse = mapFromGlobal(QCursor::pos());
            QPointF relative_mouse = mapToScene(origin_mouse);


            QStringList lipid_names;
            vector<string> lipids_for_selection;
            for (int i = 0; i < (int)pointSet->points.size(); ++i){
                double intens = GlobalData::showQuant ? pointSet->points[i].intensity : POINT_BASE_SIZE;
                double margin = sq(0.5 * intens);
                if (sq(relative_mouse.x() - pointSet->points[i].point.x()) + sq(relative_mouse.y() - pointSet->points[i].point.y()) <= margin){
                    lipid_names.push_back(QString("%1 (%2)").arg(pointSet->points[i].label).arg(pointSet->points[i].normalized_intensity));
                    lipids_for_selection.push_back(pointSet->points[i].label.toStdString());
                }
            }

            emit lipidsForSelection(lipids_for_selection);
            if (!lipid_names.size()) showMessage("");
            else showMessage(lipid_names.join(", "));
        }
        else if (dendrogram){
            QPoint origin_mouse = mapFromGlobal(QCursor::pos());
            QPointF relative_mouse = mapToScene(origin_mouse);
            QList<QGraphicsItem *> over = graphics_scene.items(QRectF(relative_mouse.x() - 5, relative_mouse.y() - 5, 10, 10), Qt::IntersectsItemShape);
        }
        QGraphicsView::mouseMoveEvent(event);
    }
}


void Canvas::setUpdate(){
    if (pointSet) pointSet->set_point_size();
    repaint();
}


void Canvas::updateDendrogram(){
    if (dendrogram){
        dendrogram->update_bounds(false);
    }
}


void Canvas::setStudyVariable(string _study_variable){
    if (dendrogram){
        dendrogram->study_variable = _study_variable;
        dendrogram->update_bounds(false);
    }
}




void Canvas::resizeEvent(QResizeEvent *event) {
    if (!dendrogram && !pointSet) return;

    if (dendrogram) dendrogram->update_bounds();

    QRectF bound = pointSet ? pointSet->boundingRect() : dendrogram->boundingRect();
    fitInView(bound, Qt::KeepAspectRatio);
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    if (dendrogram && dendrogram->top_line) dendrogram->top_line->update_width(DENDROGRAM_LINE_SIZE / (double)transform().m11());

    transforming(v);
    QGraphicsView::resizeEvent(event);
}



void Canvas::wheelEvent(QWheelEvent *event){
    if (!dendrogram && !pointSet) return;

    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();


    QPointF center(v.x() + v.width() * 0.5, v.y() + v.height() * 0.5);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF mouse_pos = mapToScene(QPoint(event->position().x(), event->position().y()));
    double scale_factor = (event->angleDelta().y() > 0) ? 1.1 : 1. / 1.1;
#else
    QPointF mouse_pos = mapToScene(event->pos());
    double scale_factor = (event->delta() > 0) ? 1.1 : 1. / 1.1;
#endif
    scale(scale_factor, scale_factor);

    centerOn(mouse_pos.x() + (center.x() - mouse_pos.x()) / scale_factor, mouse_pos.y() + (center.y() - mouse_pos.y()) / scale_factor);

    QRect viewportRect2(0, 0, viewport()->width(), viewport()->height());
    v = mapToScene(viewportRect2).boundingRect();

    transforming(v);
    if (pointSet) pointSet->updateView(v);
    else if (dendrogram && dendrogram->top_line) dendrogram->top_line->update_width(DENDROGRAM_LINE_SIZE / (double)transform().m11());
    oldCenter.setX(v.x() + v.width() * 0.5);
    oldCenter.setY(v.y() + v.height() * 0.5);
}




void Canvas::moveToPoint(QListWidgetItem* item){
    if (pointSet){
        QString species = item->text();
        for (auto point : pointSet->points){
            if (species == point.label){
                centerOn(point.point);

                QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
                QRectF v = mapToScene(viewportRect).boundingRect();
                transforming(v);
                break;
            }
        }
    }
}


void Canvas::setTransforming(QRectF f){

    setBackgroundBrush(QBrush());
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();

    double scaling = v.width() / f.width();
    scale(scaling, scaling);

    centerOn(f.x() + f.width() * 0.5, f.y() + f.height() * 0.5);
    oldCenter.setX(f.x() + f.width() * 0.5);
    oldCenter.setY(f.y() + f.height() * 0.5);
    if (pointSet) pointSet->updateView(v);
}




void Canvas::reloadPoints(){
    if (pointSet){
        pointSet->loadPoints();

        Array vars;
        if (canvas_type == GlobalSpaceCanvas){
            LipidSpace::compute_PCA_variances(lipid_space->global_lipidome->m, vars);
        }
        else if (canvas_type == StudySpaceCanvas){
            LipidSpace::compute_PCA_variances(lipid_space->study_lipidomes[num]->m, vars);
        }
        else if (canvas_type == SampleSpaceCanvas){
            LipidSpace::compute_PCA_variances(lipid_space->selected_lipidomes[num]->m, vars);
        }
        pointSet->variances = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
    }
    else if (dendrogram){
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        mapToScene(viewportRect).boundingRect();
    }
}


void Canvas::resetCanvas(){
    graphics_scene.clear();
}



void Canvas::showHideQuant(bool _showQuant){
    showQuant = _showQuant;
}

