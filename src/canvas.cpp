#include "lipidspace/canvas.h"


PointSet::PointSet(int _len, Table *_table, bool _is_dendrogram){
    len = _len;
    table = _table;
    points = new QPointF[len];
    is_dendrogram = _is_dendrogram;
    
    if (!is_dendrogram){
        QFileInfo qFileInfo(table->file_name.c_str());
        title = qFileInfo.baseName();
    }
    else {
        title = "Dendrogram";
    }
}

PointSet::~PointSet(){
    delete []points;
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
    if (table->m.rows == 0 || table->m.cols == 0) return;
    
    map<string, vector<int>> indexes;
    for (int i = 0; i < (int)table->classes.size(); ++i){
        string lipid_class = table->classes.at(i);
        if (uncontains_val(indexes, lipid_class)) indexes.insert({lipid_class, vector<int>()});
        indexes.at(lipid_class).push_back(i);
    }
    Array mean_x;
    Array mean_y;
    
    // plot the dots
    for (auto kv : indexes){
        double mx = 0, my = 0;
        for (auto i : kv.second){
            mx += table->m(i, 0);
            my += table->m(i, 1);
        }
        mean_x.push_back(mx / (double)kv.second.size());
        mean_y.push_back(my / (double)kv.second.size());
        class_means.push_back(QPointF(mx / (double)kv.second.size() * PRECESION_FACTOR, my / (double)kv.second.size() * PRECESION_FACTOR));
        
        stringstream label;
        label << kv.first << " (" << kv.second.size() << ")";
        labels.push_back(QString(label.str().c_str()));
    }
    
    for (int i = 0; i < table->m.rows; ++i){
        mean_x.push_back(table->m(i, 0));
        mean_y.push_back(table->m(i, 1));
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








Canvas::Canvas(QWidget *parent) : QWidget(parent), logo("LipidSpace.png"), label_color(LABEL_COLOR) {
    view_enabled = false;
    mousePressed = Qt::NoButton;
    scaling = 1.;
    scaling_dendrogram = 1.;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    lipid_space = 0;
    basescale = 1;
    numTiles = 0;
    tileColumns = 0;
    offset.setX(0);
    offset.setY(0);
    offset_dendrogram.setX(0);
    offset_dendrogram.setY(0);
    basescale = -1;
    oldSize.setX(width());
    oldSize.setX(height());
    color_counter = 0;
    alpha = ALPHA;
    mainWindow = 0;
    setMouseTracking(true);
    tileLayout = 0;
    showQuant = true;
    showDendrogram = true;
    showGlobalLipidome = true;
    movePointSet.setRect(0, 0, 0, 0);
    movePointSetStart.setX(0.);
    movePointSetStart.setY(0.);
    moveSet = -1;
    dendrogram_factor = 1;
}


Canvas::~Canvas(){
    for (auto pointSet : pointSets) delete pointSet;
}



const vector<QColor> Canvas::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"), QColor("#17becf")};



void Canvas::mousePressEvent(QMouseEvent *event){
    if (mousePressed == Qt::NoButton){
        mousePressed = event->button();
        deltaMouse = event->pos();
        oldOffset = QPoint(offset.x(), offset.y());
        oldOffset_dendrogram = QPoint(offset_dendrogram.x(), offset_dendrogram.y());
        
        if (mousePressed == Qt::RightButton){
            QPoint mouse = event->pos();
            int bound_num = -1;
            for (int i = 0; i < (int)pointSets.size(); ++i){
                PointSet *pointSet = pointSets[i];
                QRectF &bound = pointSet->bound;
                if (bound.x() <= mouse.x() && mouse.x() <= bound.x() + bound.width() && bound.y() <= mouse.y() && mouse.y() <= bound.y() + bound.height()){
                    bound_num = i;
                    break;
                }
            }
            if (bound_num >= (showGlobalLipidome + showDendrogram)){
                movePointSet.setRect(pointSets[bound_num]->bound.x(), pointSets[bound_num]->bound.y(), pointSets[bound_num]->bound.width(), pointSets[bound_num]->bound.height());
                movePointSetStart.setX(pointSets[bound_num]->bound.x());
                movePointSetStart.setY(pointSets[bound_num]->bound.y());
                moveSet = bound_num;
            }
        }
    }
    
    
}






void Canvas::mouseReleaseEvent(QMouseEvent *event){
    
    if (mousePressed == Qt::RightButton){
        int bound_num = -1;
        QPoint mouse = event->pos();
        for (int i = 0; i < (int)pointSets.size(); ++i){
            PointSet *pointSet = pointSets[i];
            QRectF &bound = pointSet->bound;
            if (bound.x() <= mouse.x() && mouse.x() <= bound.x() + bound.width() && bound.y() <= mouse.y() && mouse.y() <= bound.y() + bound.height()){
                bound_num = i;
                break;
            }
                
        }
        if (bound_num >= (showGlobalLipidome + showDendrogram) && moveSet > -1) {
            if (bound_num != moveSet){
                swap(pointSets[moveSet]->bound, pointSets[bound_num]->bound);
                swap(pointSets[moveSet], pointSets[bound_num]);
                swap(lipid_space->lipidomes[moveSet], lipid_space->lipidomes[bound_num]);
            }
        }
    }
    
    mousePressed = Qt::NoButton;
    movePointSet.setRect(0, 0, 0, 0);
    moveSet = -1;
    update();
}






void Canvas::mouseMoveEvent(QMouseEvent *event){
    QPoint mouse = event->pos();
    
    // check if mouse is hovering over a lipid bubble
    int bound_num = -1;
    for (int i = 0; i < (int)pointSets.size(); ++i){
        PointSet *pointSet = pointSets[i];
        QRectF &bound = pointSet->bound;
        if (bound.x() <= mouse.x() && mouse.x() <= bound.x() + bound.width() && bound.y() <= mouse.y() && mouse.y() <= bound.y() + bound.height()){
            bound_num = i;
            break;
        }
            
    }
    if (bound_num < 0) return;
    
    if (mousePressed == Qt::LeftButton){
        if (bound_num == 0 && showDendrogram && lipid_space->lipidomes.size() > 1){
            offset_dendrogram.setX(oldOffset_dendrogram.x() + (mouse.x() - deltaMouse.x()));
            offset_dendrogram.setY(oldOffset_dendrogram.y() + (mouse.y() - deltaMouse.y()));
        }
        else {
            offset.setX(oldOffset.x() + (mouse.x() - deltaMouse.x()));
            offset.setY(oldOffset.y() + (mouse.y() - deltaMouse.y()));
        }
        update();
        return;
    }
    
    else if (mousePressed == Qt::RightButton && pointSets.size() > 2){
        movePointSet.setRect(movePointSetStart.x() + (mouse.x() - deltaMouse.x()),movePointSetStart.y() + (mouse.y() - deltaMouse.y()), movePointSet.width(), movePointSet.height());
        update();
        return;
    }

    
    
    PointSet *pointSet = pointSets[bound_num];
    QPointF *points = pointSet->points;
    
    QStringList lipid_names;
    for (int i = 0; i < pointSet->len; ++i){
        double xx = (mouse.x() - offset.x() - pointSet->bound.x()) / (basescale * scaling);
        double yy = (mouse.y() - offset.y() - pointSet->bound.y()) / (basescale * scaling);
        double intens = showQuant ? (pointSet->table->intensities[i] > 1 ? log(pointSet->table->intensities[i]) : 0.5) : 1.;
        double margin = sq(POINT_BASE_SIZE * 0.5 * intens);
        if (sq(xx - points[i].x()) + sq(points[i].y() - yy) <= margin){
            lipid_names.push_back(QString(pointSet->table->species[i].c_str()));
        }
    }
    
    if (!lipid_names.size()) showMessage("");
    else showMessage(lipid_names.join(", "));
}




    
void Canvas::wheelEvent(QWheelEvent *event){
    
    
    QPoint mouse = event->pos();
    int bound_num = -1;
    for (int i = 0; i < (int)pointSets.size(); ++i){
        PointSet *pointSet = pointSets[i];
        QRectF &bound = pointSet->bound;
        if (bound.x() <= mouse.x() && mouse.x() <= bound.x() + bound.width() && bound.y() <= mouse.y() && mouse.y() <= bound.y() + bound.height()){
            bound_num = i;
            break;
        }
            
    }
    if (bound_num < 0) return;
    
    if (bound_num == 0 && showDendrogram && lipid_space->lipidomes.size() > 1){
        double old_scaling = scaling_dendrogram;
        if (event->angleDelta().y() < 0){
            if (scaling_dendrogram > 0.2) scaling_dendrogram /= 1.1;
        }
        else {
            scaling_dendrogram *= 1.1;
        }
        QRectF &bound = pointSets[bound_num]->bound;
        double mx = mouse.x() - bound.x();
        double my = mouse.y() - bound.y();
        offset_dendrogram.setX(mx + (offset_dendrogram.x() - mx) / old_scaling * scaling_dendrogram);
        offset_dendrogram.setY(my + (offset_dendrogram.y() - my) / old_scaling * scaling_dendrogram);
    }
    else {
        double old_scaling = scaling;
        if (event->angleDelta().y() < 0){
            if (scaling > 0.2) scaling /= 1.1;
        }
        else {
            scaling *= 1.1;
        }
        QRectF &bound = pointSets[bound_num]->bound;
        double mx = mouse.x() - bound.x();
        double my = mouse.y() - bound.y();
        offset.setX(mx + (offset.x() - mx) / old_scaling * scaling);
        offset.setY(my + (offset.y() - my) / old_scaling * scaling);
    }
    
    update();
}



void Canvas::setInputClasses(LipidSpace *_lipid_space, QMainWindow *_mainWindow){
    lipid_space = _lipid_space;
    mainWindow = _mainWindow;
}




void Canvas::resizeEvent(QResizeEvent* ){
    if (!lipid_space || !pointSets.size()) return;
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_space = (double)width() * MARGIN;
    double h_space = (double)height() * MARGIN;
    double w_rect = (double)width() * ((1. - MARGIN * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - MARGIN * ((double)tileRows + 1.)) / (double)tileRows);
    
    
    double dx = w_space;
    double dy = h_space;
    for (int i = 0, cc = 0; i < numTiles; ++i){
        pointSets[i]->bound.setRect(dx, dy, w_rect, h_rect);
        dx += w_space + w_rect;
        if (++cc >= tileColumns){
            dx = w_space;
            dy += h_space + h_rect;
            cc = 0;
        }
    }
    
    
    double ww = pointSets[0]->bound.width();
    double hh = pointSets[0]->bound.height();
    
    double mx = pointSets[0]->bound.x() + (ww * 0.5);
    double my = pointSets[0]->bound.y() + (hh * 0.5);
    
    
    if (oldSize.x() > 0 && oldSize.y() > 0){
        offset.setX(offset.x() + (mx - oldSize.x()));
        offset.setY(offset.y() + (my - oldSize.y()));
    }
    if (oldSize.x() > 0 && oldSize.y() > 0){
        offset_dendrogram.setX(offset_dendrogram.x() + (mx - oldSize.x()));
        offset_dendrogram.setY(offset_dendrogram.y() + (my - oldSize.y()));
    }
    
    oldSize.setX(mx);
    oldSize.setY(my);
    
    
    if ((double)w_rect / (minMax.y() - minMax.x()) < (double)h_rect / (minMax.height() - minMax.width())){
        basescale = (double)w_rect / (minMax.y() - minMax.x());
    }
    else {
        basescale = (double)h_rect / (minMax.height() - minMax.width());
    }
    

    update();
}




void Canvas::resetCanvas(){
    view_enabled = false;
    colorMap.clear();
    color_counter = 0;
    numTiles = 0;
    tileColumns = 0;
    for (auto pointSet : pointSets) delete pointSet;
    pointSets.clear();
    oldSize.setX(0);
    oldSize.setY(0);
    update();
    scaling = 1;
    basescale = 1;
}




void Canvas::setLayout(int _tileLayout){
    tileLayout = _tileLayout;
}


void Canvas::showHideQuant(bool _showQuant){
    showQuant = _showQuant;
}


void Canvas::showHideDendrogram(bool _showDendrogram){
    showDendrogram = _showDendrogram;
}


void Canvas::showHideGlobalLipidome(bool _showGlobalLipidome){
    showGlobalLipidome = _showGlobalLipidome;
}



void Canvas::refreshCanvas(){
    if (!lipid_space) return;
    
    
    
    colorMap.clear();
    color_counter = 0;
    Table* global_lipidome = lipid_space->global_lipidome;
    numTiles = (lipid_space->lipidomes.size() > 1 && showDendrogram) + (lipid_space->lipidomes.size() > 1 && showGlobalLipidome) + lipid_space->lipidomes.size();
    tileColumns = tileLayout == 0 ? ceil(sqrt((double)numTiles)) : tileLayout;
    
    
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_rect = (double)width() * ((1. - MARGIN * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - MARGIN * ((double)tileRows + 1.)) / (double)tileRows);
    
    
    pointSets.clear();
    
    
    if (showDendrogram && lipid_space->lipidomes.size() > 1){
        double x_min_d = 1e9;
        double x_max_d = 0;
        double y_min_d = 1e9;
        double y_max_d = 0;
        
        pointSets.push_back(new PointSet(0, 0, true));
        for (int i = 0; i < (int)lipid_space->dendrogram_points.size(); i += 4){
            pointSets.back()->lines.push_back(QLineF(lipid_space->dendrogram_points[i], -lipid_space->dendrogram_points[i + 1], lipid_space->dendrogram_points[i + 2], -lipid_space->dendrogram_points[i + 3]));
            
            
            x_min_d = min(x_min_d, lipid_space->dendrogram_points[i]);
            x_max_d = max(x_max_d, lipid_space->dendrogram_points[i]);
            x_min_d = min(x_min_d, lipid_space->dendrogram_points[i + 2]);
            x_max_d = max(x_max_d, lipid_space->dendrogram_points[i + 2]);
            y_min_d = min(y_min_d, lipid_space->dendrogram_points[i + 1]);
            y_max_d = max(y_max_d, lipid_space->dendrogram_points[i + 1]);
            y_min_d = min(y_min_d, lipid_space->dendrogram_points[i + 3]);
            y_max_d = max(y_max_d, lipid_space->dendrogram_points[i + 3]);
            
        }
        
        dendrogram_factor = w_rect / (x_max_d - x_min_d) * 0.9;
        double factor_y_d = h_rect / (y_max_d - y_min_d) * 0.7;
        
        for (QLineF &line : pointSets.back()->lines){
            line.setLine(line.x1() * dendrogram_factor, line.y1() * factor_y_d, line.x2() * dendrogram_factor, line.y2() * factor_y_d);
        }
        
        
        offset_dendrogram.setX((double)width() * MARGIN + w_rect * 0.5 - (x_max_d + x_min_d) * 0.5 * dendrogram_factor);
        offset_dendrogram.setY((double)height() * MARGIN + h_rect * 0.85);
    }
    
    
    
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    
    // add only global lipidome when we have more than one lipidome
    if (lipid_space->lipidomes.size() > 1 && showGlobalLipidome){
        if (global_lipidome->m.cols >= 2){
            pointSets.push_back(new PointSet(global_lipidome->m.rows, global_lipidome));
            
            for (int r = 0; r < global_lipidome->m.rows; ++r){
                double xval = global_lipidome->m(r, 0) * PRECESION_FACTOR;
                double yval = global_lipidome->m(r, 1) * PRECESION_FACTOR;
                x_min = min(x_min, xval);
                x_max = max(x_max, xval);
                y_min = min(y_min, yval);
                y_max = max(y_max, yval);
                pointSets.back()->points[r].setX(xval);
                pointSets.back()->points[r].setY(yval);
            }
            pointSets.back()->set_labels();
        }
        else {
            return;
        }
        x_min *= 1.1;
        x_max *= 1.1;
        y_min *= 1.1;
        y_max *= 1.1;
    }
    
    for (Table* lipidome : lipid_space->lipidomes){
        pointSets.push_back(new PointSet(lipidome->m.rows, lipidome));
        for (int r = 0; r < lipidome->m.rows; ++r){
            double xval = lipidome->m(r, 0) * PRECESION_FACTOR;
            double yval = lipidome->m(r, 1) * PRECESION_FACTOR;
            x_min = min(x_min, xval);
            x_max = max(x_max, xval);
            y_min = min(y_min, yval);
            y_max = max(y_max, yval);
            pointSets.back()->points[r].setX(xval);
            pointSets.back()->points[r].setY(yval);
        }
        pointSets.back()->set_labels();
    }
    x_min *= 1.1;
    x_max *= 1.1;
    y_min *= 1.1;
    y_max *= 1.1;
    minMax.setRect(x_min, x_max, y_min, y_max);
    
    if ((double)w_rect / (x_max - x_min) < (double)h_rect / (y_max - y_min)){
        basescale = (double)w_rect / (x_max - x_min);
    }
    else {
        basescale = (double)h_rect / (y_max - y_min);
    }
    
    
    
    
    if (showDendrogram && lipid_space->lipidomes.size() > 1){
        int off = (lipid_space->lipidomes.size() > 1 && showDendrogram) + (lipid_space->lipidomes.size() > 1 && showGlobalLipidome);
        for (int i : lipid_space->dendrogram_sorting){
            pointSets[0]->dendrogram_titles.push_back(pointSets.at(off + i)->title);
        }
    }
    
    
    
    double ox = w_rect * 0.5 - ((x_max + x_min) / 2 * basescale);
    double oy = h_rect * 0.5 - ((y_max + y_min) / 2 * basescale);
    offset.setX(ox);
    offset.setY(oy);
    
    
    resizeEvent(0);
}



void Canvas::enableView(bool view){
    view_enabled = view;
    update();
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



    
void Canvas::paintEvent(QPaintEvent *event){
    const QRect & rect = event->rect();
    QPainter painter(this);
    painter.eraseRect(rect);
    //painter.setRenderHint(QPainter::Antialiasing);
    
    if (!view_enabled){
        painter.fillRect(QRect(0, 0, width(), height()), Qt::white);
        double val = min(width(), height()) * 0.7;
        painter.drawImage(QRectF((width() - val) * 0.5, (height() - val) * 0.5, val, val), logo);
        return;
    }
    
    if (!lipid_space->analysis_finished) return;
    
    // set background
    painter.fillRect(QRect(0, 0, width(), height()), QColor(245, 245, 245, 255));
    
    

    if (showDendrogram && lipid_space->lipidomes.size() > 1){
        QPen pen;
        pen.setColor(QColor(0, 0, 0, 255));
        pen.setWidth(1);
        painter.setPen(pen);
        
        QPainterPath path;
        path.addRoundedRect(pointSets[0]->bound, 10, 10);
        painter.fillPath(path, Qt::white);
        
        painter.save();
        painter.setClipRect(pointSets[0]->bound); // setting clipping area
        
        // transform area to according section
        painter.translate(offset_dendrogram);
        painter.scale(scaling_dendrogram, scaling_dendrogram);
        
        painter.drawLines(pointSets[0]->lines);
        double di = 0;
        QFont f("Helvetica", 2);
        painter.setFont(f);
        for (QString title : pointSets[0]->dendrogram_titles){
            painter.rotate(-90);
            painter.drawText(QRect(-202, (di - 20), 200, 40), Qt::AlignVCenter | Qt::AlignRight, title);
            painter.rotate(90);
            di += dendrogram_factor;
        }
        painter.restore();
    }
            
    for (auto pointSet : pointSets){
        if (pointSet->is_dendrogram) continue;
        
        QPainterPath path;
        path.addRoundedRect(pointSet->bound, 10, 10);
        painter.fillPath(path, Qt::white);
        
        // drawing the points
        for (int i = 0; i < pointSet->len; ++i){
            // checking lipid class and selecting color
            string lipid_class = pointSet->table->classes[i];
            if (uncontains_val(colorMap, lipid_class)){
                colorMap.insert({lipid_class, COLORS[color_counter++ % COLORS.size()]});
            }
            
            painter.save();
            painter.setClipRect(pointSet->bound); // setting clipping area
            painter.translate(offset); // transform area to according section
            painter.translate(QPointF(pointSet->bound.x(), pointSet->bound.y()));
            
            double intens = showQuant ? (pointSet->table->intensities[i] > 1 ? log(pointSet->table->intensities[i]) : 0.5) : 1.;
            painter.scale(scaling * basescale, scaling * basescale);
            
            // setting up pen for painter
            QColor qcolor = colorMap[lipid_class];
            qcolor.setAlpha(alpha);
            
            QPainterPath path;
            path.addEllipse(pointSet->points[i], intens, intens);
            painter.fillPath(path, qcolor);
            painter.restore();
        }
        
        // drawing the labels
        // setting up the font type
        QFont f("Helvetica", 1.);
        painter.setFont(f);
        
        QPen pen_arr;
        pen_arr.setColor(label_color);
        pen_arr.setWidth(qreal(0.1));
        pen_arr.setStyle(Qt::DashLine);
        
        for (int i = 0; i < (int)pointSet->labels.size(); ++i){
            painter.save();                             
            painter.setClipRect(pointSet->bound);       // setting clipping area
            painter.translate(offset);                  // transform area to according section
            painter.translate(QPointF(pointSet->bound.x(), pointSet->bound.y()));
            painter.scale(scaling * basescale, scaling * basescale);
            
            
            // draw the actual label
            painter.setPen(pen_arr);
            QRectF labelPosition(pointSet->label_points[i].x() - 5 * basescale, pointSet->label_points[i].y() - 2 * basescale, 10 * basescale, 4 * basescale);
            QRectF boundingRect;
            painter.drawText(labelPosition, Qt::AlignCenter, pointSet->labels[i], &boundingRect);
            
            
            // shrink a little bit the bounding box
            boundingRect = boundingRect.marginsRemoved(QMarginsF(0, boundingRect.height() * 0.3, 0, boundingRect.height() * 0.2));
            painter.restore();
            
            
            // Search for label arrow starting point
            QPointF new_start;
            bool found = find_start(boundingRect, pointSet->class_means[i], new_start);
            if (!found) continue;
            
            double hypothenuse = sqrt(sq(new_start.x() - pointSet->class_means[i].x()) + sq(new_start.y() - pointSet->class_means[i].y()));
            double angle = asin((new_start.y() - pointSet->class_means[i].y()) / hypothenuse) / M_PI * 180.;
            painter.save();
            painter.setClipRect(pointSet->bound);       // setting clipping area
            painter.translate(offset);                  // transform area to according section
            painter.translate(QPointF(pointSet->bound.x(), pointSet->bound.y()));
            painter.scale(scaling * basescale, scaling * basescale);
            
            // draw the arrow
            painter.setPen(pen_arr);
            
            QPointF rotate_point = new_start.x() < pointSet->class_means[i].x() ? new_start : pointSet->class_means[i];
            double sign = 1. - 2. * (new_start.x() < pointSet->class_means[i].x());
             
            painter.translate(rotate_point);
            painter.rotate(sign * angle);
            QRectF rectangle(0, -hypothenuse * 0.1, hypothenuse, hypothenuse * 0.2);
            double const startAngle = 16. * 20.;
            double const endAngle   = 16. * 160.;
            double const spanAngle = endAngle - startAngle;
            painter.drawArc(rectangle, startAngle, spanAngle);
            painter.restore();
            
            
            
            
            //TODO
            //QPen pen_head;
            //pen_head.setColor(label_color);
            //painter.setPen(pen_head);
            //QPolygonF polygon;
            //polygon << QPointF(0.5 * basescale, -0.5 * basescale) << QPointF(0.8 * basescale);
            //QPainterPath head_path;
            //head_path.addPolygon(myPolygon);
            //painter.fillPath(head_path, label_color);
            
        }
        
        
    }
    
    
    
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    
    QFont f("Helvetica", 8);
    painter.setFont(f);
    
    /*
    for (auto pointSet : pointSets){
        QRectF textBound(pointSet->bound);
        textBound.setX(textBound.x() + 5);
        painter.drawText(textBound, Qt::AlignTop | Qt::AlignLeft, pointSet->title);
        painter.drawRect(pointSet->bound);
    }
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
    */
    
    // show shaddow window when moving tile
    if (movePointSet.width() > 0){
        QPen movePen;
        QColor qc = QColor(0, 110, 255, 50);
        movePen.setColor(qc);
        painter.setPen(movePen);
        painter.fillRect(movePointSet, qc);
    }
}
