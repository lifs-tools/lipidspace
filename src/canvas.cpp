#include "lipidspace/canvas.h"

/*

CanvasItem::CanvasItem(QGraphicsView *_view) : view(_view) {
    old_view = view->mapToScene(view->viewport()->geometry()).boundingRect();
    title = "";
}


CanvasItem::~CanvasItem() {
}


QRectF CanvasItem::boundingRect() const {
    return bound;
}



void CanvasItem::updateView(){
    old_view = view->mapToScene(view->viewport()->geometry()).boundingRect();
}




void CanvasItem::resize(){
    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
    
    double x_scale = v.width() / old_view.width();
    double y_scale = v.height() / old_view.height();
    double scale_factor = min(x_scale, y_scale);
    
    double x_center = old_view.x() + 0.5 * old_view.width();
    double y_center = old_view.y() + 0.5 * old_view.height();
    cout << v.x() << " " << v.y() << " " << v.width() << " " << v.height() << " | " << x_center << " " << y_center << endl;
    
    view->scale(scale_factor, scale_factor);
    view->centerOn(x_center, y_center);
    old_view = v;
}

*/



Dendrogram::Dendrogram(LipidSpace *_lipid_space, Canvas *_view) : view(_view) {
    lipid_space = _lipid_space;
    
    for (int i : lipid_space->dendrogram_sorting){
        QFileInfo qFileInfo(lipid_space->lipidomes[i]->file_name.c_str());
        dendrogram_titles.push_back(qFileInfo.baseName());
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
    
    
    double x_mean = (x_max_d + x_min_d) * 0.5;
    double y_mean = -(y_max_d + y_min_d) * 0.5;
    
    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
    dendrogram_factor = v.width() / (x_max_d - x_min_d) * 1.2;
    double factor_y_d = v.height() / (y_max_d - y_min_d) * 0.8;
    pp = factor_y_d;
    
    bound.setX((x_min_d - x_mean) * dendrogram_factor * 0.5);
    bound.setY((y_min_d - y_mean) * factor_y_d * 0.5);
    bound.setWidth((x_max_d - x_min_d) * dendrogram_factor * 1.5);
    bound.setHeight((y_max_d - y_min_d) * factor_y_d * 1.5);
    
    for (QLineF &line : lines){
        line.setLine((line.x1() - x_mean) * dendrogram_factor, (line.y1() - y_mean) * factor_y_d, (line.x2() - x_mean) * dendrogram_factor, (line.y2() - y_mean) * factor_y_d);
    }
    title = "Dendrogram";
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
    double dx = bound.x() * 2.;
    double dy = bound.y() * 2.;
    for (QString dtitle : dendrogram_titles){
        painter->save();
        painter->translate(QPointF(dx, dy));
        painter->rotate(-35);
        painter->drawText(QRect(-210, -30, 200, 60), Qt::AlignVCenter | Qt::AlignRight, dtitle);
        dx += dendrogram_factor;
        painter->restore();
    }
}









PointSet::PointSet(Table *_lipidome, Canvas *_view) : view(_view) {
    lipidome = _lipidome;
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    QFileInfo qFileInfo(lipidome->file_name.c_str());
    title = qFileInfo.baseName();
    
    for (int r = 0; r < lipidome->m.rows; ++r){
        double xval = lipidome->m(r, 0);
        double yval = lipidome->m(r, 1);
        double intens = lipidome->intensities[r] > 1 ? log(lipidome->intensities[r]) : 0.5;
        x_min = min(x_min, xval - intens);
        x_max = max(x_max, xval + intens);
        y_min = min(y_min, yval - intens);
        y_max = max(y_max, yval + intens);
        
        string lipid_class = lipidome->classes[r];
        if (uncontains_val(LipidSpaceGUI::colorMap, lipid_class)){
            LipidSpaceGUI::colorMap.insert({lipid_class, LipidSpaceGUI::COLORS[LipidSpaceGUI::color_counter++ % LipidSpaceGUI::COLORS.size()]});
        }
        points.push_back(QPointF(xval, yval));
    }
    
    set_labels();
    
    for (auto label_point : label_points){
        x_min = min(x_min, label_point.x());
        x_max = max(x_max, label_point.x());
        y_min = min(y_min, label_point.y());
        y_max = max(y_max, label_point.y());
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
    
    QLineF B1(bound.x(), bound.y(), bound.x() + bound.width(), bound.y());
    if (path.intersect(B1, &inter) == QLineF::BoundedIntersection) return true;
    
    QLineF B2(bound.x(), bound.y(), bound.x(), bound.y() + bound.height());
    if (path.intersect(B2, &inter) == QLineF::BoundedIntersection) return true;
    
    QLineF B3(bound.x(), bound.y() + bound.height(), bound.x() + bound.width(), bound.y() + bound.height());
    if (path.intersect(B3, &inter) == QLineF::BoundedIntersection) return true;
    
    QLineF B4(bound.x() + bound.width(), bound.y(), bound.x() + bound.width(), bound.y() + bound.height());
    if (path.intersect(B4, &inter) == QLineF::BoundedIntersection) return true;
    
    return false;
}


void PointSet::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    
    QRectF v = view->mapToScene(view->viewport()->geometry()).boundingRect();
    
    for (int i = 0; i < (int)points.size(); ++i){
        
        double intens = LipidSpaceGUI::showQuant ? (lipidome->intensities[i] > 1 ? log(lipidome->intensities[i]) : 0.5) : 1.;
        QRectF bubble(points[i].x() - intens * 0.5, points[i].y() - intens * 0.5,  intens, intens);
        if (!v.intersects(bubble)) continue;
        
        string lipid_class = lipidome->classes[i];
        
        // setting up pen for painter
        QColor &qcolor = LipidSpaceGUI::colorMap[lipid_class];
        qcolor.setAlpha(128);
        QPainterPath p;
        p.addEllipse(bubble);
        painter->fillPath(p, qcolor);
    }
    
    // drawing the labels
    QPen pen_arr;
    pen_arr.setColor(QColor(LABEL_COLOR));
    pen_arr.setStyle(Qt::DashLine);
    pen_arr.setWidth(0.1);
    
    
    QFont f("Helvetica", 1);
    painter->setFont(f);
    for (int i = 0; i < (int)label_points.size(); ++i){
        painter->setPen(pen_arr);
        QRectF labelPosition(label_points[i].x() - 50, label_points[i].y() - 20, 100, 40);
        QRectF boundingRect;
        painter->drawText(labelPosition, Qt::AlignCenter, labels[i], &boundingRect);
        
        
        // shrink a little bit the bounding box
        boundingRect = boundingRect.marginsRemoved(QMarginsF(0, boundingRect.height() * 0.3, 0, boundingRect.height() * 0.2));
        
        
        // Search for label arrow starting point
        QPointF new_start;
        bool found = find_start(boundingRect, class_means[i], new_start);
        if (!found) continue;
        
        double hypothenuse = sqrt(sq(new_start.x() - class_means[i].x()) + sq(new_start.y() - class_means[i].y()));
        double angle = asin((new_start.y() - class_means[i].y()) / hypothenuse) / M_PI * 180.;
        
        
        // draw the arrow
        painter->setPen(pen_arr);
        
        QPointF rotate_point = new_start.x() < class_means[i].x() ? new_start : class_means[i];
        double sign = 1. - 2. * (new_start.x() < class_means[i].x());
        
        
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
    for (int i = 0; i < (int)lipidome->classes.size(); ++i){
        string lipid_class = lipidome->classes.at(i);
        if (uncontains_val(indexes, lipid_class)) indexes.insert({lipid_class, vector<int>()});
        indexes.at(lipid_class).push_back(i);
    }
    Array mean_x;
    Array mean_y;
    
    // plot the dots
    for (auto kv : indexes){
        double mx = 0, my = 0;
        for (auto i : kv.second){
            mx += lipidome->m(i, 0);
            my += lipidome->m(i, 1);
        }
        mean_x.push_back(mx / (double)kv.second.size());
        mean_y.push_back(my / (double)kv.second.size());
        class_means.push_back(QPointF(mx / (double)kv.second.size() * PRECESION_FACTOR, my / (double)kv.second.size() * PRECESION_FACTOR));
        
        stringstream label;
        label << kv.first << " (" << kv.second.size() << ")";
        labels.push_back(QString(label.str().c_str()));
    }
    
    for (int i = 0; i < lipidome->m.rows; ++i){
        mean_x.push_back(lipidome->m(i, 0));
        mean_y.push_back(lipidome->m(i, 1));
    }

    
    // plot the annotations
    Matrix label_m;
    automated_annotation(mean_x, mean_y, label_m);
    for (int r = 0; r < label_m.rows; ++r){
        label_points.push_back(QPointF(label_m(r, 0) * PRECESION_FACTOR, label_m(r, 1) * PRECESION_FACTOR));
    }
}



void PointSet::automated_annotation(Array &xx, Array &yy, Matrix &label_points){
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
    
    
    label_points.reset(0, 0);
    label_points.add_column(label_xx);
    label_points.add_column(label_yy);
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
    view->graphics_scene.setSceneRect(-50000, -50000, 100000, 100000);
}









Canvas::Canvas(LipidSpace *_lipid_space, QMainWindow *_mainWindow, int _num, QWidget *) : num(_num) {
    lipid_space = _lipid_space;
    mainWindow = _mainWindow;
    
    pointSet = 0;
    dendrogram = 0;
    
    setDragMode(QGraphicsView::ScrollHandDrag);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    viewport()->setCursor(Qt::ArrowCursor);
    //setViewport(new QOpenGLWidget());
    
    
    setScene(&graphics_scene);
    graphics_scene.setSceneRect(-500, -300, 1000, 600);
    resetMatrix();
    initialized = false;
    
    leftMousePressed = false;
    showQuant = true;
    // set the graphics item within this graphics view
    if (num == -2){ // dendrogram
        dendrogram = new Dendrogram(lipid_space, this);
        graphics_scene.addItem(dendrogram);
    }
    else if (num == -1){ // global lipidome
        pointSet = new PointSet(lipid_space->global_lipidome, this);
        graphics_scene.addItem(pointSet);
    }
    else { // regular lipidome
        pointSet = new PointSet(lipid_space->lipidomes[num], this);
        graphics_scene.addItem(pointSet);
    }
}



Canvas::~Canvas(){
    if (pointSet) delete pointSet;
    if (dendrogram) delete dendrogram;
}





void Canvas::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        leftMousePressed = true;
    }
    QGraphicsView::mousePressEvent(event);
}

void Canvas::exportPdf(QString outputFolder){
    QPrinter printer(QPrinter::HighResolution);
    //printer.setPageSize(QPrinter::A4);
    printer.setOutputFormat(QPrinter::PdfFormat);
    QString file_name = QDir(outputFolder).filePath((dendrogram ? dendrogram->title : pointSet->title) + ".pdf");
    
    printer.setOutputFileName(file_name);

    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    graphics_scene.setSceneRect(v);
    QPainter p(&printer);
    graphics_scene.render(&p);
    p.end();
    graphics_scene.setSceneRect(-50000, -50000, 100000, 100000);
}




void Canvas::mouseDoubleClickEvent(QMouseEvent *){
    doubleClicked(num);
    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    transforming(v, num);
    if (pointSet) pointSet->updateView(v);
}
    
    
    
void Canvas::mouseReleaseEvent(QMouseEvent *event){
    leftMousePressed = false;
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}



void Canvas::setInitialized(){
    initialized = true;
}



void Canvas::mouseMoveEvent(QMouseEvent *event){
    if (leftMousePressed){
        viewport()->setCursor(Qt::DragMoveCursor);
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        QRectF v = mapToScene(viewportRect).boundingRect();
        transforming(v, num);
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
            double intens = LipidSpaceGUI::showQuant ? (pointSet->lipidome->intensities[i] > 1 ? log(pointSet->lipidome->intensities[i]) : 0.5) : 1.;
            double margin = sq(0.5 * intens);
            if (sq(relative_mouse.x() - pointSet->points[i].x()) + sq(relative_mouse.y() - pointSet->points[i].y()) <= margin){
                lipid_names.push_back(QString(pointSet->lipidome->species[i].c_str()));
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

    
void Canvas::resizeEvent(QResizeEvent *event) {
    if (!initialized){
        QRectF bounds = dendrogram ? dendrogram->boundingRect() : pointSet->boundingRect();
        fitInView(bounds, Qt::KeepAspectRatio);
        QGraphicsView::resizeEvent(event);
        
        QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
        QRectF v = mapToScene(viewportRect).boundingRect();
        transforming(v, num);
        
    }
    else {
        if (pointSet) pointSet->resize();
    }
}


void Canvas::wheelEvent(QWheelEvent *event){
    if(event->delta() > 0) scale(1.1, 1.1);
    else scale(1. / 1.1, 1. / 1.1);
    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    
    transforming(v, num);
    if (pointSet) pointSet->updateView(v);
    oldCenter.setX(v.x() + v.width() * 0.5);
    oldCenter.setY(v.y() + v.height() * 0.5);
}


void Canvas::setTransforming(QRectF f, int _num){
    if (num == _num) return;
    
    QRect viewportRect(0, 0, viewport()->width(), viewport()->height());
    QRectF v = mapToScene(viewportRect).boundingRect();
    
    double scaling = v.width() / f.width();
    scale(scaling, scaling);
    
    centerOn(f.x() + f.width() * 0.5, f.y() + f.height() * 0.5);
    oldCenter.setX(f.x() + f.width() * 0.5);
    oldCenter.setY(f.y() + f.height() * 0.5);
    if (pointSet) pointSet->updateView(v);
    graphics_scene.setSceneRect(-50000, -50000, 100000, 100000);
}


void Canvas::resetCanvas(){
    graphics_scene.clear();
    resetMatrix();
}



void Canvas::showHideQuant(bool _showQuant){
    showQuant = _showQuant;
}

