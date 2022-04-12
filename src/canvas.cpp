#include "lipidspace/canvas.h"


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
        if (dendrogram->top_line) dendrogram->top_line->make_permanent(false);
        make_permanent(true);
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
// Canvas methods
///////////////////////////////////////////////////////////////////////////////////////


Dendrogram::Dendrogram(LipidSpace *_lipid_space, Canvas *_view) : view(_view) {
    lipid_space = _lipid_space;
    feature = "";
    setZValue(100);
    top_line = 0;
    highlighted_for_selection = 0;
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
    double minx = max(1e-3, x_max_d - x_min_d);
    double miny = max(1e-3, y_max_d - y_min_d);
    double x1 = (node->x_left - x_min_d) * dwidth / minx;
    double y1 = (-node->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
    double x2 = (node->x_right - x_min_d) * dwidth / minx;
    double y2 = (-node->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
    
    
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidthF(DENDROGRAM_LINE_SIZE / view->transform().m11());
    
    DendrogramLine* v_line = new DendrogramLine(QLineF(x1, y1, x2, y2), pen, this);
    view->graphics_scene.addItem(v_line);
    v_line->setAcceptHoverEvents(true);
    
    bool is_node = true;
    if (node->left_child){
        double yl = (-node->left_child->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
        DendrogramLine* l_line = new DendrogramLine(QLineF(x1, y1, x1, yl), pen, this);
        l_line->d_node = node->left_child;
        view->graphics_scene.addItem(l_line);
        l_line->setAcceptHoverEvents(true);
        v_line->next_line = l_line;
        add_dendrogram_lines(node->left_child, l_line);
        is_node = false;
    }
    
    if (node->right_child){
        double yr = (-node->right_child->y + y_max_d) * dheight / miny / 100. * dendrogram_y_factor;
        DendrogramLine* r_line = new DendrogramLine(QLineF(x2, y2, x2, yr), pen, this);
        r_line->d_node = node->right_child;
        view->graphics_scene.addItem(r_line);
        r_line->setAcceptHoverEvents(true);
        v_line->second_line = r_line;
        add_dendrogram_lines(node->right_child, r_line);
        is_node = false;
    }
    if (is_node){
        v_line->node = node->order;
    }
    
    if (parent_line) parent_line->next_line = v_line;
    else top_line = v_line;
}
 
 
void Dendrogram::load(){
    top_line = 0;
    view->graphics_scene.removeItem(this);
    view->graphics_scene.clear();
    view->graphics_scene.addItem(this);
    dendrogram_titles.clear();
    lines.clear();
    
    if (!lipid_space->dendrogram_root) return;
    
    for (int i : lipid_space->dendrogram_sorting){
        dendrogram_titles.push_back(DendrogramTitle(QString(lipid_space->selected_lipidomes[i]->cleaned_name.c_str())));
    }
    
    x_min_d = 1e9;
    x_max_d = 0;
    y_min_d = 1e9;
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
    dendrogram_y_factor = GlobalData::dendrogram_height;
    
    dwidth = (lipid_space->selected_lipidomes.size() - 1) * dendrogram_x_factor;
    dheight = dwidth / 16 * 9;
    dendrogram_height = dheight / (y_max_d - y_min_d);
    dendrogram_y_offset = y_max_d;
    
    double w = max(dendrogram_x_factor, 100.);
    double h = max(dendrogram_y_factor, 100.);
    
    bound.setX(- 2 * dendrogram_x_factor);
    bound.setY(- dendrogram_y_factor);
    bound.setWidth(dwidth + 3 * w);
    bound.setHeight(dheight + 3 * h);
    
    
    add_dendrogram_lines(lipid_space->dendrogram_root);
    for (QLineF &line : lines){
        line.setLine((line.x1() - x_min_d) * dwidth / (x_max_d - x_min_d),
                     (line.y1() + y_max_d) * dheight / (y_max_d - y_min_d),
                     (line.x2() - x_min_d) * dwidth / (x_max_d - x_min_d),
                     (line.y2() + y_max_d) * dheight / (y_max_d - y_min_d));
    }
        
    w = max(bound.width(), (double)view->viewport()->width());
    h = max(bound.height(), (double)view->viewport()->height());
    view->graphics_scene.setSceneRect(-10 * w, -10 * h, 20 * w, 20 * h);
}


Dendrogram::~Dendrogram(){
    
}


QRectF Dendrogram::boundingRect() const {
    return bound;
}



void Dendrogram::draw_pie(QPainter *painter, DendrogramNode *node, double threshold, double pie_x, double pie_y, LabelDirection direction){
    double resize_factor = (double)GlobalData::pie_size / 100.;
    double pie_radius = 30. * resize_factor;
    int angle_start = 16 * 90;
    QFont pie_font("Helvetica");
    pie_font.setPointSizeF(10. * resize_factor);
    if (lipid_space->feature_values[feature].feature_type == NominalFeature){
        double sum = 0;
        for (auto kv : node->feature_count_nominal[feature]){
            sum += kv.second;
        }
        
        for (auto kv : node->feature_count_nominal[feature]){
            if (kv.second == 0) continue;
            int span = 16. * 360. * (double)kv.second / sum;
            QBrush brush(GlobalData::colorMapFeatures[feature + "_" + kv.first]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
            angle_start = (angle_start + span) % 5760; // 360 * 16
        }
    }
    else {
        string feature_le = feature + "_le";
        string feature_gr = feature + "_gr";
        
        int num = 0;
        if (uncontains_val(node->feature_numerical, feature) || node->feature_numerical[feature].size() == 0) return;
        for (double val : node->feature_numerical[feature]){
            num += val <= threshold;
        }
        
        int span = 16. * 360. * (double)num / (double)node->feature_numerical[feature].size();
        QBrush brush(GlobalData::colorMapFeatures[feature_le]);
        QPen piePen(brush.color());
        painter->setPen(piePen);
        painter->setBrush(brush);
        painter->setFont(pie_font);
        if (span > 0) painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
        
        angle_start = (angle_start + span) % 5760;
        span = 5760 - span;
        brush.setColor(GlobalData::colorMapFeatures[feature_gr]);
        piePen.setColor(brush.color());
        painter->setPen(piePen);
        painter->setBrush(brush);
        if (span > 0) painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
        
        
        if (direction == LabelLeft){
            double x = pie_x - pie_radius * 1.2;
            double y = pie_y;
            
            // drawing the legend squares
            QBrush brush(GlobalData::colorMapFeatures[feature_le]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x - 15 * resize_factor, y - 20 * resize_factor, 15 * resize_factor, 15 * resize_factor);
            
            brush.setColor(GlobalData::colorMapFeatures[feature_gr]);
            piePen.setBrush(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x - 15 * resize_factor, y + 5 * resize_factor, 15 * resize_factor, 15 * resize_factor);
            
            QPen textPen(Qt::black);
            painter->setPen(textPen);
            painter->drawText(QRect(x - 320 * resize_factor, y - 42 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignRight, QString::number(threshold, 'g') + QString(" ") + QChar(0x2265));
            painter->drawText(QRect(x - 320 * resize_factor, y - 18 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignRight, QString::number(threshold, 'g') + QString(" <") );
            
            
            
        }
        else if (direction == LabelRight){
            double x = pie_x + pie_radius * 1.2;
            double y = pie_y;
            
            // drawing the legend squares
            QBrush brush(GlobalData::colorMapFeatures[feature_le]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x, y - 20 * resize_factor, 15 * resize_factor, 15 * resize_factor);
            
            brush.setColor(GlobalData::colorMapFeatures[feature_gr]);
            piePen.setBrush(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawRect(x, y + 5 * resize_factor, 15 * resize_factor, 15 * resize_factor);
            
            
            
            QPen textPen(Qt::black);
            painter->setPen(textPen);
            painter->drawText(QRect(x + 20 * resize_factor, y - 42 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignLeft, QChar(0x2264) + QString(" ") + QString::number(threshold, 'g'));
            painter->drawText(QRect(x + 20 * resize_factor, y - 18 * resize_factor, 300 * resize_factor, 60 * resize_factor), Qt::AlignVCenter | Qt::AlignLeft, QString("> ") + QString::number(threshold, 'g'));
            
        }
    }
    painter->setPen(QPen());
    painter->setBrush(QBrush());
    painter->drawEllipse(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2);
}





void Dendrogram::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (!lipid_space->dendrogram_root) return;
    
    QFont f("Helvetica", 18);
    painter->setFont(f);
    double dx = 0;
    double dy = 10 + (((lipid_space->selected_lipidomes.size() - 1) * dendrogram_x_factor) / 16. * 9.) * dendrogram_y_factor / 100;
    for (auto dtitle : dendrogram_titles){
        painter->save();
        painter->translate(QPointF(dx, dy));
        painter->rotate(-35);
        QPen pen((dtitle.highlighted || dtitle.permanent) ? Qt::red : Qt::black);
        painter->setPen(pen);
        painter->drawText(QRect(-200, -30, 200, 60), Qt::AlignVCenter | Qt::AlignRight, dtitle.title);
        dx += dendrogram_x_factor;
        painter->restore();
    }
    
    
    if (lipid_space && lipid_space->dendrogram_root){
        if (feature != ""){
            recursive_paint(painter, lipid_space->dendrogram_root, GlobalData::pie_tree_depth);
            
            // print the feature legend
            QFont legend_font("Helvetica", 8);
            if (contains_val(lipid_space->feature_values, feature)){
                int num_feature = 0; 
                for (auto feature_value_kv : lipid_space->feature_values.at(feature).nominal_values){
                    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
                    QTransform qtrans = view->transform();
                    painter->save();
                    painter->translate(QPointF(v.x(), v.y()));
                    painter->scale(1. / qtrans.m11(), 1. / qtrans.m22());
                    if (contains_val(GlobalData::colorMapFeatures, feature + "_" + feature_value_kv.first)){
                        painter->fillRect(QRectF(view->viewport()->geometry().width() - 20, 5 + 20 * num_feature, 15, 15), GlobalData::colorMapFeatures[feature + "_" + feature_value_kv.first]);
                    }
                    painter->setFont(legend_font);
                    painter->drawText(QRect(view->viewport()->geometry().width() - 225, 5 + 20 * num_feature, 200, 30), Qt::AlignTop | Qt::AlignRight, feature_value_kv.first.c_str());
                    
                    painter->restore();
                    ++num_feature;
                }
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
        painter->drawText(QRect(2, 0, 200, 60), Qt::AlignTop | Qt::AlignLeft, "Dendrogram");
        painter->restore();
    }
}


void Dendrogram::recursive_paint(QPainter *painter, DendrogramNode *node, int max_recursions, int recursion){
    if (recursion == max_recursions || node == 0) return;
        
    // processing left child
    if (node->left_child != 0 && node->left_child->indexes.size() > 0){
        double pie_x = node->x_left * dendrogram_x_factor;
        double pie_y = (-node->y + dendrogram_y_offset) * dendrogram_height * dendrogram_y_factor / 100.;
        
        draw_pie(painter, node->left_child, node->feature_numerical_thresholds[feature], pie_x, pie_y, LabelLeft);
        recursive_paint(painter, node->left_child, max_recursions, recursion + 1);
    }
    
    // processing right child
    if (node->right_child != 0 && node->right_child->indexes.size() > 0){
        double pie_x = node->x_right * dendrogram_x_factor;
        double pie_y = (-node->y + dendrogram_y_offset) * dendrogram_height * dendrogram_y_factor / 100.;
        
        draw_pie(painter, node->right_child, node->feature_numerical_thresholds[feature], pie_x, pie_y, LabelRight);
        recursive_paint(painter, node->right_child, max_recursions, recursion + 1);
    }
}











///////////////////////////////////////////////////////////////////////////////////////
// Canvas methods
///////////////////////////////////////////////////////////////////////////////////////



PointSet::PointSet(Table *_lipidome, Canvas *_view) : view(_view) {
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
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    
    for (int r = 0, rr = 0; r < (int)lipidome->species.size(); ++r){
        if (!lipidome->selection[r]){
            continue;
        }
            
        
        double xval = sign_log(lipidome->m(rr, GlobalData::PC1)) * POINT_BASE_FACTOR;
        double yval = sign_log(lipidome->m(rr, GlobalData::PC2)) * POINT_BASE_FACTOR;
        double intens = lipidome->intensities[rr] > 1 ? log(lipidome->intensities[rr]) : 0.5;
        double intens_boundery = intens * 0.5;
        
        x_min = min(x_min, xval - intens_boundery);
        x_max = max(x_max, xval + intens_boundery);
        y_min = min(y_min, yval - intens_boundery);
        y_max = max(y_max, yval + intens_boundery);
        
        points.push_back(PCPoint());
        points.back().point = QPointF(xval, yval);
        points.back().intensity = intens;
        points.back().color = GlobalData::colorMap[lipidome->classes[r]];
        points.back().label = lipidome->species[r].c_str();
        
        QRectF bubble(xval - intens * 0.5, yval - intens * 0.5,  intens, intens);
        
        // setting up pen for painter
        QColor qcolor = GlobalData::colorMap[lipidome->classes[r]];
        qcolor.setAlpha(GlobalData::alpha);
        
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(bubble);
        ellipse->setZValue(0);
        ellipse->setBrush(QBrush(qcolor));
        QPen pen(qcolor);
        pen.setWidth(0);
        ellipse->setPen(pen);
        view->graphics_scene.addItem(ellipse);
        points.back().item = ellipse;
        rr++;
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
    return view->graphics_scene.sceneRect();
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
        pen_black.setWidth(0.5);
        painter->setPen(pen_black);
        painter->drawEllipse(bubble);
    }
    
    
    
    // drawing the labels
    QPen pen_arr;
    pen_arr.setColor(QColor(LABEL_COLOR));
    pen_arr.setStyle(Qt::DashLine);
    pen_arr.setWidth(5);
    
    
    QFont f("Helvetica", 1);
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
    painter->drawText(QRect(2, 0, 200, 60), Qt::AlignTop | Qt::AlignLeft, title);
    painter->drawText(QRect(2, view->viewport()->geometry().height() - 60, 200, 60), Qt::AlignBottom | Qt::AlignLeft, variances);
    painter->restore();
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
    
    map<string, vector<int>> indexes;
    for (int i = 0, j = 0; i < (int)lipidome->classes.size(); ++i){
        if (!lipidome->selection[i]) continue;
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
            mx += sign_log(lipidome->m(i, GlobalData::PC1)) * POINT_BASE_FACTOR;
            my += sign_log(lipidome->m(i, GlobalData::PC2)) * POINT_BASE_FACTOR;
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
        mean_x.push_back(sign_log(lipidome->m(i, 0)) * POINT_BASE_FACTOR);
        mean_y.push_back(sign_log(lipidome->m(i, 1)) * POINT_BASE_FACTOR);
    }

    
    // plot the annotations
    automated_annotation(mean_x, mean_y);
    
}



void PointSet::automated_annotation(Array &xx, Array &yy){
    int l = labels.size();
    Array label_xx(xx, l);
    Array label_yy(yy, l);
    double max_int = (double)(~0u) * 0.5;
    for (int i = 0; i < l; ++i){
        label_xx(i) += (((double)rand()) / max_int) * 0.5 - 0.25;
        label_yy(i) += (((double)rand()) / max_int) * 0.5 - 0.25;
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
    hovered_for_swap = false;
    
    setDragMode(QGraphicsView::ScrollHandDrag);
    setFrameStyle(QFrame::NoFrame);
    setRenderHints(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
    
    graphics_scene.setSceneRect(-5000, -3000, 10000, 6000);
    setScene(&graphics_scene);
    resetMatrix();
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
    graphics_scene.setSceneRect(-5000, -3000, 10000, 6000);
}


void Canvas::resetDendrogram(){
    resetMatrix();
    dendrogram->load();
    QResizeEvent resize(QSize(width(), height()), QSize(width(), height()));
    emit resizeEvent(&resize);
}


Canvas::Canvas(LipidSpace *_lipid_space, int _num, QListWidget* _listed_species, QWidget *) : num(_num) {
    lipid_space = _lipid_space;
    listed_species = _listed_species;
    
    
    pointSet = 0;
    dendrogram = 0;
    hovered_for_swap = false;
    
    setDragMode(QGraphicsView::ScrollHandDrag);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
    
    graphics_scene.setSceneRect(-5000, -3000, 10000, 6000);
    setScene(&graphics_scene);
    resetMatrix();
    
    
    
    leftMousePressed = false;
    showQuant = true;
    // set the graphics item within this graphics view
    if (num == -2){ // dendrogram
        dendrogram = new Dendrogram(lipid_space, this);
        graphics_scene.addItem(dendrogram);
        dendrogram->load();
    }
    else if (num == -1){ // global lipidome
        pointSet = new PointSet(lipid_space->global_lipidome, this);
        graphics_scene.addItem(pointSet);
        pointSet->loadPoints();
        pointSet->title = "Global lipidome";
        
        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->global_lipidome->m, vars);
        pointSet->variances = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
    }
    else { // regular lipidome
        pointSet = new PointSet(lipid_space->selected_lipidomes[num], this);
        graphics_scene.addItem(pointSet);
        pointSet->loadPoints();
        pointSet->title = QString(lipid_space->selected_lipidomes[num]->cleaned_name.c_str());
        
        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->lipidomes[num]->m, vars);
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
    if (dendrogram || num < 0) return;
    QRect widgetRect = QRect(0, 0, width(), height());
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    if (widgetRect.contains(mousePos)) {
        setFrameStyle(QFrame::Panel);
        hovered_for_swap = true;
    }
    else {
        setFrameStyle(QFrame::NoFrame);
        hovered_for_swap = false;
    }
}


void Canvas::setSwap(int source){
    if (dendrogram || num < 0 || !hovered_for_swap) return;
    setFrameStyle(QFrame::NoFrame);
    hovered_for_swap = false;
    swappingLipidomes(source, num);
}



void Canvas::contextMenu(QPoint pos){
    context(this, pos);
}


void Canvas::mousePressEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;
    
    if (event->button() == Qt::LeftButton){
        leftMousePressed = true;
        QGraphicsView::mousePressEvent(event);
    }
    else if (lipid_space->selected_lipidomes.size() > 1 && event->button() == Qt::MiddleButton){
        mouse(event, this);
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
    QString file_name = QFileDialog::getSaveFileName(this, "Export as pdf", ".", "*.pdf");
    if (!file_name.length()) {
        return;
    }
    
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setFontEmbeddingEnabled(true);
    QPageSize pageSize(QSizeF(viewport()->width(), viewport()->height()) , QPageSize::Point);
    printer.setPageSize(pageSize);
    
    printer.setOutputFileName(file_name);
    
    // set margins to 0
    QPagedPaintDevice::Margins margins;
    margins.top = 0;
    margins.left = 0;
    margins.right = 0;
    margins.bottom = 0;
    printer.setMargins (margins);

    
    QPainter p(&printer);
    render(&p);
    p.end();
}




void Canvas::mouseDoubleClickEvent(QMouseEvent *){
    if (!dendrogram && !pointSet) return;
    
    doubleClicked(num);
    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    transforming(v);
    if (pointSet) pointSet->updateView(v);
    if (dendrogram && num == -3 && dendrogram->top_line){
        dendrogram->top_line->make_permanent(false);
        setBackgroundBrush(QBrush());
    }
}
    
    
    
void Canvas::mouseReleaseEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;
    
    leftMousePressed = false;
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}






void Canvas::mouseMoveEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;
    
    if (leftMousePressed){
        setBackgroundBrush(QBrush());
        viewport()->setCursor(Qt::DragMoveCursor);
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        QRectF v = mapToScene(viewportRect).boundingRect();
        transforming(v);
        if (pointSet) pointSet->updateView(v);
        
        oldCenter.setX(v.x() + v.width() * 0.5);
        oldCenter.setY(v.y() + v.height() * 0.5);
    }
    
    // check if mouse over lipid bubble
    if (pointSet){
        QPoint origin_mouse = mapFromGlobal(QCursor::pos());
        QPointF relative_mouse = mapToScene(origin_mouse);
        
        
        QStringList lipid_names;
        for (int i = 0; i < (int)pointSet->points.size(); ++i){
            double intens = GlobalData::showQuant ? pointSet->points[i].intensity : 1.;
            double margin = sq(0.5 * intens);
            if (sq(relative_mouse.x() - pointSet->points[i].point.x()) + sq(relative_mouse.y() - pointSet->points[i].point.y()) <= margin){
                lipid_names.push_back(QString(pointSet->points[i].label));
            }
        }
        
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


void Canvas::setUpdate(){
    repaint();
}


void Canvas::setFeature(string _feature){
    
    if (dendrogram){
        dendrogram->feature = _feature;
        double tmp_factor = dendrogram->dendrogram_y_factor;
        dendrogram->dendrogram_y_factor = GlobalData::dendrogram_height;
        QPointF max_vals(1e9, -1e9);
        if (dendrogram->top_line) dendrogram->top_line->update_height_factor(dendrogram->dendrogram_y_factor / tmp_factor, &max_vals);
        double ybound = max(dendrogram->dendrogram_y_factor, 100.);
        dendrogram->bound.setY(max_vals.x() - ybound);
        dendrogram->bound.setHeight((max_vals.y() - max_vals.x()) + 3 * ybound);
        
        double w = max(dendrogram->bound.width(), (double)viewport()->width());
        double h = max(dendrogram->bound.height(), (double)viewport()->height());
        graphics_scene.setSceneRect(-10 * w, -10 * h, 20 * w, 20 * h);
        setBackgroundBrush(QBrush());
        dendrogram->update();
    }
}

    
void Canvas::resizeEvent(QResizeEvent *event) {
    if (!dendrogram && !pointSet) return;
      
    QRectF bounds = dendrogram ? dendrogram->bound : pointSet->bound;
    fitInView(bounds, Qt::KeepAspectRatio);
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    transforming(v);
    if (dendrogram && dendrogram->top_line) dendrogram->top_line->update_width(DENDROGRAM_LINE_SIZE / (double)transform().m11());
    
    QGraphicsView::resizeEvent(event);
}



void Canvas::wheelEvent(QWheelEvent *event){
    if (!dendrogram && !pointSet) return;
    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    
    
    QPointF center(v.x() + v.width() * 0.5, v.y() + v.height() * 0.5);
    QPointF mouse_pos = mapToScene(event->pos());
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    double scale_factor = (event->angleDelta().y() > 0) ? 1.1 : 1. / 1.1;
#else
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
                
                QRect viewportRect2(0, 0, viewport()->width(), viewport()->height());
                QRectF v = mapToScene(viewportRect2).boundingRect();
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
        if (num == -1){
            LipidSpace::compute_PCA_variances(lipid_space->global_lipidome->m, vars);
        }
        else if (num >= 0){
            LipidSpace::compute_PCA_variances(lipid_space->lipidomes[num]->m, vars);
        }
        pointSet->variances = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
    }
    else if (dendrogram){
        QRect viewportRect2(0, 0, viewport()->width(), viewport()->height());
        mapToScene(viewportRect2).boundingRect();
    }
}


void Canvas::resetCanvas(){
    graphics_scene.clear();
    resetMatrix();
}



void Canvas::showHideQuant(bool _showQuant){
    showQuant = _showQuant;
}

