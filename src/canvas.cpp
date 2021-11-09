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


Canvas::Canvas(QWidget *parent) : QWidget(parent){
    mousePressed = Qt::NoButton;
    antialiased = true;
    scaling = 1.;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    lipid_space = 0;
    basescale = 1;
    numTiles = 0;
    tileColumns = 0;
    offset.setX(0);
    offset.setY(0);
    basescale = -1;
    oldSize.setX(width());
    oldSize.setX(height());
    color_counter = 0;
    alpha = 128;
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
}


Canvas::~Canvas(){
    for (auto pointSet : pointSets) delete pointSet;
}






const double Canvas::PRECESION_FACTOR = 10000.;
const vector<QColor> Canvas::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"), QColor("#17becf")};







void Canvas::mousePressEvent(QMouseEvent *event){
    if (mousePressed == Qt::NoButton){
        mousePressed = event->button();
        deltaMouse = event->pos();
        oldOffset = QPoint(offset.x(), offset.y());
        
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
        if (bound_num == 0 && showDendrogram){
            
        }
        else {
            offset.setX(oldOffset.x() + (mouse.x() - deltaMouse.x()));
            offset.setY(oldOffset.y() + (mouse.y() - deltaMouse.y()));
            update();
        }
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
        double xx = points[i].x() / basescale * scaling + offset.x() + pointSet->bound.x();
        double yy = points[i].y() / basescale * scaling + offset.y() + pointSet->bound.y();
        double intens = showQuant ? (pointSet->table->intensities[i] > 1 ? log(pointSet->table->intensities[i]) : 0.5) : 1.;
        double margin = POINT_BASE_SIZE * 0.5 * intens * scaling;
        if (sq(mouse.x() - xx) + sq(mouse.y() - yy) <= sq(margin)){
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
    
    if (bound_num == 0 && showDendrogram){
        
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
    
    oldSize.setX(mx);
    oldSize.setY(my);
    
    
    update();
}




void Canvas::resetCanvas(){
    
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
    
    
    if (!global_lipidome) return;
    
    pointSets.clear();
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    
    if (showDendrogram){
        pointSets.push_back(new PointSet(0, 0, true));
    }
    
    
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
    }
    x_min *= 1.1;
    x_max *= 1.1;
    y_min *= 1.1;
    y_max *= 1.1;
    minMax.setRect(x_min, x_max, y_min, y_max);
    
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_rect = (double)width() * ((1. - MARGIN * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - MARGIN * ((double)tileRows + 1.)) / (double)tileRows);
    
    if ((x_max - x_min) / (double)w_rect > (y_max - y_min) / (double)h_rect){
        basescale = (x_max - x_min) / (double)w_rect;
    }
    else {
        basescale = (y_max - y_min) / (double)h_rect;
    }
    
    
    
    double ox = w_rect * 0.5 - ((x_max + x_min) / 2 / basescale);
    double oy = h_rect * 0.5 - ((y_max + y_min) / 2 / basescale);
    offset.setX(ox);
    offset.setY(oy);
    
    
    resizeEvent(0);
}



    
void Canvas::paintEvent(QPaintEvent *event){
    const QRect & rect = event->rect();
    QPainter painter(this);
    painter.eraseRect(rect);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (!pointSets.size()) return;

            
    for (auto pointSet : pointSets){
        if (pointSet->is_dendrogram) continue;
        for (int i = 0; i < pointSet->len; ++i){
            painter.save();
            
            // setting clipping area
            painter.setClipRect(pointSet->bound);
            
            
            // transform area to according section
            painter.translate(offset);
            painter.translate(QPointF(pointSet->bound.x(), pointSet->bound.y()));
            
            // checking lipid class and selecting color
            string lipid_class = pointSet->table->classes[i];
            if (uncontains_val(colorMap, lipid_class)){
                colorMap.insert({lipid_class, COLORS[color_counter++ % COLORS.size()]});
            }
            
            double intens = showQuant ? (pointSet->table->intensities[i] > 1 ? log(pointSet->table->intensities[i]) : 0.5) : 1.;
            
            painter.scale(scaling / basescale, scaling / basescale);
            
            
            // setting up pen for painter
            QPen pen;
            QColor qcolor = i > 0 ? colorMap[lipid_class] : QColor(0, 0, 0, 255);
            qcolor.setAlpha(alpha);
            pen.setColor(qcolor);
            pen.setWidth(POINT_BASE_SIZE * intens * basescale);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            
            painter.drawPoint(pointSet->points[i]);
            painter.restore();
        }
    }
    
    
    
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    
    QFont f("Helvetica", 8);
    painter.setFont(f);
    
    for (auto pointSet : pointSets){
        QRectF textBound(pointSet->bound);
        textBound.setX(textBound.x() + 5);
        painter.drawText(textBound, Qt::AlignTop | Qt::AlignLeft, pointSet->title);
        painter.drawRect(pointSet->bound);
    }
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
    
    
    if (movePointSet.width() > 0){
        QPen movePen;
        QColor qc = QColor(0, 110, 255, 50);
        movePen.setColor(qc);
        painter.setPen(movePen);
        painter.fillRect(movePointSet, qc);
    }
}
