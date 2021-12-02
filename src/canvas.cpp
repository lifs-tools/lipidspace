#include "lipidspace/canvas.h"


Dendrogram::Dendrogram(LipidSpace *_lipid_space, Canvas *_view) : view(_view) {
    lipid_space = _lipid_space;
    feature == "";
    load();
}
 
 
void Dendrogram::clear(){
    dendrogram_titles.clear();
    lines.clear();
    update();
}
 
 
void Dendrogram::load(){
    dendrogram_titles.clear();
    lines.clear();
    
    for (int i : lipid_space->dendrogram_sorting){
        dendrogram_titles.push_back(QString(lipid_space->lipidomes[i]->cleaned_name.c_str()));
    }
    
    double x_min_d = 1e9;
    double x_max_d = 0;
    double y_min_d = 1e9;
    double y_max_d = 0;
    
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
  

    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
    dendrogram_x_factor = v.width() / (x_max_d - x_min_d) * 1.2;
    dendrogram_y_factor = v.height() / (y_max_d - y_min_d) * 0.8;
    
    x_min_d *= dendrogram_x_factor;
    x_max_d *= dendrogram_x_factor;
    y_min_d *= dendrogram_y_factor;
    y_max_d *= dendrogram_y_factor;
    
    double x_margin = (x_max_d - x_min_d) * 0.2;
    double y_margin = (y_max_d - y_min_d) * 0.2;
    
    bound.setX(x_min_d - x_margin);
    bound.setY(-y_max_d - y_margin);
    bound.setWidth(x_max_d - x_min_d + 2 * x_margin);
    bound.setHeight(y_max_d - y_min_d + 2 * y_margin);
    
    for (QLineF &line : lines){
        line.setLine(line.x1() * dendrogram_x_factor, line.y1() * dendrogram_y_factor, line.x2() * dendrogram_x_factor, line.y2() * dendrogram_y_factor);
    }
}


Dendrogram::~Dendrogram(){
    
}


QRectF Dendrogram::boundingRect() const {
    return bound;
}


void Dendrogram::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QPen pen;
    pen.setColor(QColor(0, 0, 0, 255));
    pen.setWidth(0.3);
    painter->setPen(pen);
    painter->drawLines(lines);
    
    QFont f("Helvetica", 10);
    painter->setFont(f);
    double dx = 0;
    double dy = 0;
    for (QString dtitle : dendrogram_titles){
        painter->save();
        painter->translate(QPointF(dx, dy));
        painter->rotate(-35);
        painter->drawText(QRect(-210, -30, 200, 60), Qt::AlignVCenter | Qt::AlignRight, dtitle);
        dx += dendrogram_x_factor;
        painter->restore();
    }
    
    if (!lipid_space || !lipid_space->dendrogram_root || feature == "")
        return;
    
    pen.setWidth(0);
    double pie_radius = 30;
    double pie_x = (lipid_space->dendrogram_root->x_left + lipid_space->dendrogram_root->x_right) * 0.5 * dendrogram_x_factor;
    double pie_y = -lipid_space->dendrogram_root->y * dendrogram_y_factor - pie_radius * 1.5;
    
    double sum = 0;
    for (auto kv : lipid_space->dendrogram_root->feature_count[feature]){
        sum += kv.second;
    }
    
    int angle_start = 16 * 90;
    for (auto kv : lipid_space->dendrogram_root->feature_count[feature]){
        if (kv.second == 0) continue;
        int span = 16. * 360. * (double)kv.second / sum;
        QBrush brush(GlobalData::colorMapFeatures[feature + "_" + kv.first]);
        QPen piePen(brush.color());
        painter->setPen(piePen);
        painter->setBrush(brush);
        painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
        angle_start = (angle_start + span) % 5760; // 360 * 16
    }
    
    painter->setPen(QPen());
    painter->setBrush(QBrush());
    painter->drawEllipse(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2);
    
    recursive_paint(painter, lipid_space->dendrogram_root, 3);
}


void Dendrogram::recursive_paint(QPainter *painter, DendrogramNode *node, int max_recursions, int recursion){
    if (recursion == max_recursions) return;
    double pie_radius = 30;
        
    // processing left child
    if (node->left_child->indexes.size() > 1){
        double pie_x = node->x_left * dendrogram_x_factor;
        double pie_y = -node->y * dendrogram_y_factor;
        
        double sum = 0;
        for (auto kv : node->left_child->feature_count[feature]) sum += kv.second;
        
        int angle_start = 16 * 90;
        for (auto kv : node->left_child->feature_count[feature]){
            if (kv.second == 0) continue;
            int span = 16. * 360. * (double)kv.second / sum;
            QBrush brush(GlobalData::colorMapFeatures[feature + "_" + kv.first]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
            angle_start = (angle_start + span) % 5760; // 360 * 16
        }
        painter->setPen(QPen());
        painter->setBrush(QBrush());
        painter->drawEllipse(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2);
    
        recursive_paint(painter, node->left_child, max_recursions, recursion + 1);
    }
    
    // processing right child
    if (node->right_child->indexes.size() > 1){
        double pie_x = node->x_right * dendrogram_x_factor;
        double pie_y = -node->y * dendrogram_y_factor;
    
        double sum = 0;
        for (auto kv : node->right_child->feature_count[feature]) sum += kv.second;
        
        int angle_start = 16 * 90;
        for (auto kv : node->right_child->feature_count[feature]){
            if (kv.second == 0) continue;
            int span = 16. * 360. * (double)kv.second / sum;
            QBrush brush(GlobalData::colorMapFeatures[feature + "_" + kv.first]);
            QPen piePen(brush.color());
            painter->setPen(piePen);
            painter->setBrush(brush);
            painter->drawPie(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2, angle_start, span);
            angle_start = (angle_start + span) % 5760; // 360 * 16
        }
        painter->setPen(QPen());
        painter->setBrush(QBrush());
        painter->drawEllipse(pie_x - pie_radius, pie_y - pie_radius, pie_radius * 2, pie_radius * 2);
        
        recursive_paint(painter, node->right_child, max_recursions, recursion + 1);
    }
}








PointSet::PointSet(Table *_lipidome, Canvas *_view) : view(_view) {
    lipidome = _lipidome;
    loadPoints();
}


void PointSet::loadPoints(){
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
            
        double xval = lipidome->m(rr, GlobalData::PC1);
        double yval = lipidome->m(rr, GlobalData::PC2);
        double intens = lipidome->intensities[rr] > 1 ? log(lipidome->intensities[rr]) : 0.5;
        x_min = min(x_min, xval - intens);
        x_max = max(x_max, xval + intens);
        y_min = min(y_min, yval - intens);
        y_max = max(y_max, yval + intens);
        
        points.push_back(PCPoint());
        points.back().point = QPointF(xval, yval);
        points.back().intensity = lipidome->intensities[rr];
        points.back().color = GlobalData::colorMap[lipidome->classes[r]];
        points.back().label = lipidome->species[r].c_str();
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
    
    // print regular points
    for (int i = 0; i < (int)points.size(); ++i){
        if (contains_val(highlighted_points, points[i].label)) continue;
        
        double intens = GlobalData::showQuant ? (points[i].intensity > 1 ? log(points[i].intensity) : 0.5) : 1.;
        QRectF bubble(points[i].point.x() - intens * 0.5, points[i].point.y() - intens * 0.5,  intens, intens);
        if (!v.intersects(bubble)) continue;
        
        // setting up pen for painter
        QColor qcolor = points[i].color;
        qcolor.setAlpha(GlobalData::alpha);
        QPainterPath p;
        p.addEllipse(bubble);
        painter->fillPath(p, qcolor);
    }
    
    // print highlighted points on top
    for (int i = 0; i < (int)points.size(); ++i){
        if (uncontains_val(highlighted_points, points[i].label)) continue;
        
        double intens = GlobalData::showQuant ? (points[i].intensity > 1 ? log(points[i].intensity) : 0.5) : 1.;
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
    pen_arr.setWidth(0.1);
    
    
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
        painter->setPen(pen_arr);
        
        QPointF rotate_point = new_start.x() < pc_label.class_mean.x() ? new_start : pc_label.class_mean;
        double sign = 1. - 2. * (new_start.x() < pc_label.class_mean.x());
        
        painter->save();
        painter->translate(rotate_point);
        painter->rotate(sign * angle);
        QRectF rectangle(0, -hypothenuse * 0.1, hypothenuse, hypothenuse * 0.2);
        double const startAngle = 16. * 20.;
        double const endAngle   = 16. * 160.;
        double const spanAngle = endAngle - startAngle;
        painter->drawArc(rectangle, startAngle, spanAngle);
        painter->restore();
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
    
    map<string, vector<int>> indexes;
    for (int i = 0, j = 0; i < (int)lipidome->classes.size(); ++i){
        if (!lipidome->selection[i]) continue;
        string lipid_class = lipidome->classes.at(i);
        if (uncontains_val(indexes, lipid_class)) indexes.insert({lipid_class, vector<int>()});
        indexes.at(lipid_class).push_back(j++);
    }
    Array mean_x;
    Array mean_y;
    
    // plot the dots
    for (auto kv : indexes){
        double mx = 0, my = 0;
        for (auto i : kv.second){
            mx += lipidome->m(i, GlobalData::PC1);
            my += lipidome->m(i, GlobalData::PC2);
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
        mean_x.push_back(lipidome->m(i, 0));
        mean_y.push_back(lipidome->m(i, 1));
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
















Canvas::Canvas(QWidget *parent) : QGraphicsView(parent), title(this) {
    pointSet = 0;
    dendrogram = 0;
    variances = 0;
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
    initialized = false;
    num = -2;
    QFont f("Helvetica", 7);
    title.move(2, 0);
    title.setText("Dendrogram");
    title.setFont(f);
    title.show();
}



void Canvas::setDendrogramData(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
    dendrogram = new Dendrogram(lipid_space, this);
    graphics_scene.addItem(dendrogram);
}


void Canvas::resetDendrogram(){
    resetMatrix();
    dendrogram->load();
    QResizeEvent resize(QSize(width(), height()), QSize(width(), height()));
    emit resizeEvent(&resize);
}


Canvas::Canvas(LipidSpace *_lipid_space, int _num, QListWidget* _listed_species, QWidget *) : num(_num), title(this) {
    lipid_space = _lipid_space;
    listed_species = _listed_species;
    
    pointSet = 0;
    dendrogram = 0;
    variances = 0;
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
    initialized = false;
    
    
    
    leftMousePressed = false;
    showQuant = true;
    // set the graphics item within this graphics view
    if (num == -2){ // dendrogram
        dendrogram = new Dendrogram(lipid_space, this);
        graphics_scene.addItem(dendrogram);
        title.setText("Dendrogram");
    }
    else if (num == -1){ // global lipidome
        pointSet = new PointSet(lipid_space->global_lipidome, this);
        graphics_scene.addItem(pointSet);
        title.setText("Global lipidome");
        
        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->global_lipidome->m, vars);
        QString var_label = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
        variances = new QLabel(var_label, this);
    }
    else { // regular lipidome
        pointSet = new PointSet(lipid_space->selected_lipidomes[num], this);
        graphics_scene.addItem(pointSet);
        title.setText(QString(lipid_space->selected_lipidomes[num]->cleaned_name.c_str()));
        
        Array vars;
        LipidSpace::compute_PCA_variances(lipid_space->lipidomes[num]->m, vars);
        QString var_label = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
        variances = new QLabel(var_label, this);
    }
    QFont f("Helvetica", 7);
    title.move(2, 0);
    title.setFont(f);
    title.show();
    
    if (variances){
        variances->setFont(f);
        variances->show();
    }
}



Canvas::~Canvas(){
    if (pointSet) delete pointSet;
    if (dendrogram) delete dendrogram;
    if (variances) delete variances;
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


void Canvas::mousePressEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;
    
    if (event->button() == Qt::LeftButton){
        leftMousePressed = true;
        QGraphicsView::mousePressEvent(event);
    }
    else if (lipid_space->selected_lipidomes.size() > 1 && event->button() == Qt::RightButton){
        mouse(event, this);
    }
    else {
        QGraphicsView::mousePressEvent(event);
    }
}



void Canvas::exportPdf(QString outputFolder){
    QPrinter printer(QPrinter::HighResolution);
    //printer.setPageSize(QPrinter::A4);
    printer.setOutputFormat(QPrinter::PdfFormat);
    QString file_name = QDir(outputFolder).filePath((title.text()) + ".pdf");
    
    printer.setOutputFileName(file_name);

    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    graphics_scene.setSceneRect(v);
    QPainter p(&printer);
    graphics_scene.render(&p);
    p.end();
}




void Canvas::mouseDoubleClickEvent(QMouseEvent *){
    if (!dendrogram && !pointSet) return;
    
    doubleClicked(num);
    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    transforming(v);
    if (pointSet) pointSet->updateView(v);
}
    
    
    
void Canvas::mouseReleaseEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;
    
    leftMousePressed = false;
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}



void Canvas::setInitialized(){
    initialized = true;
}




void Canvas::mouseMoveEvent(QMouseEvent *event){
    if (!dendrogram && !pointSet) return;
    
    if (leftMousePressed){
        viewport()->setCursor(Qt::DragMoveCursor);
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        QRectF v = mapToScene(viewportRect).boundingRect();
        transforming(v);
        if (pointSet) pointSet->updateView(v);
        
        oldCenter.setX(v.x() + v.width() * 0.5);
        oldCenter.setY(v.y() + v.height() * 0.5);
        if (variances) variances->move(2, height() - variances->height());
    }
    
    // check if mouse over lipid bubble
    if (pointSet){
        QPoint origin_mouse = mapFromGlobal(QCursor::pos());
        QPointF relative_mouse = mapToScene(origin_mouse);
        
        
        QStringList lipid_names;
        for (int i = 0; i < (int)pointSet->points.size(); ++i){
            double intens = GlobalData::showQuant ? (pointSet->points[i].intensity > 1 ? log(pointSet->points[i].intensity) : 0.5) : 1.;
            double margin = sq(0.5 * intens);
            if (sq(relative_mouse.x() - pointSet->points[i].point.x()) + sq(relative_mouse.y() - pointSet->points[i].point.y()) <= margin){
                lipid_names.push_back(QString(pointSet->points[i].label));
            }
        }
        
        if (!lipid_names.size()) showMessage("");
        else showMessage(lipid_names.join(", "));
    }
    QGraphicsView::mouseMoveEvent(event);
}


void Canvas::setUpdate(){
    repaint();
}


void Canvas::setFeature(string _feature){
    if (dendrogram) dendrogram->feature =  _feature;
    dendrogram->update();
}

    
void Canvas::resizeEvent(QResizeEvent *event) {
    if (!dendrogram && !pointSet) return;
        
    if (!initialized || dendrogram){
        QRectF bounds = dendrogram ? dendrogram->bound : pointSet->bound;
        fitInView(bounds, Qt::KeepAspectRatio);
        
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        QRectF v = mapToScene(viewportRect).boundingRect();
        transforming(v);
    }
    else {
        if (pointSet) pointSet->resize();
        
    }
    if (variances) variances->move(2, height() - variances->height());
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
    //if (num == _num)
    //    return;
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    
    double scaling = v.width() / f.width();
    scale(scaling, scaling);
    
    centerOn(f.x() + f.width() * 0.5, f.y() + f.height() * 0.5);
    oldCenter.setX(f.x() + f.width() * 0.5);
    oldCenter.setY(f.y() + f.height() * 0.5);
    if (pointSet) pointSet->updateView(v);
    if (variances) variances->move(2, height() - variances->height());
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
        QString var_label = QStringLiteral("Variances - PC%1: %2%, PC%3: %4%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3).arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3);
        variances->setText(var_label);
    }
}


void Canvas::resetCanvas(){
    graphics_scene.clear();
    resetMatrix();
}



void Canvas::showHideQuant(bool _showQuant){
    showQuant = _showQuant;
}

