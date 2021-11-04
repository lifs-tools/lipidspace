#include "lipidspace/canvas.h"


PointSet::PointSet(int _len){
    len = _len;
    points = new QPointF[len];
}

PointSet::~PointSet(){
    delete []points;
}


Canvas::Canvas(QWidget *parent) : QWidget(parent){
    mousePressed = false;
    antialiased = false;
    scaling = 1.;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    lipid_space = 0;
    basescale = 1;
    PRECESION_FACTOR = 10000.;
    numTiles = 0;
    tileColumns = 0;
    offset.setX(0);
    offset.setY(0);
    basescale = -1;
    oldSize.setX(width());
    oldSize.setX(height());
}


Canvas::~Canvas(){
    for (auto pointSet : pointSets) delete pointSet;
}



void Canvas::mousePressEvent(QMouseEvent *event){
    if (!mousePressed){
        mousePressed = true;
        deltaMouse = event->pos();
        oldOffset = QPoint(offset.x(), offset.y());
    }
    
}


void Canvas::mouseReleaseEvent(QMouseEvent *){
    mousePressed = false;
}


void Canvas::mouseMoveEvent(QMouseEvent *event){
    if (mousePressed){
        offset.setX(oldOffset.x() + (event->pos().x() - deltaMouse.x()));
        offset.setY(oldOffset.y() + (event->pos().y() - deltaMouse.y()));
        update();
    }
}




    
void Canvas::wheelEvent(QWheelEvent *event){
    double old_scaling = scaling;
    if (event->angleDelta().y() < 0){
        if (scaling > 0.2) scaling /= 1.1;
    }
    else {
        scaling *= 1.1;
    }
    QPoint mouse = event->pos();
    offset.setX(mouse.x() + (offset.x() - mouse.x()) / old_scaling * scaling);
    offset.setY(mouse.y() + (offset.y() - mouse.y()) / old_scaling * scaling);
    
    update();
}



void Canvas::setLipidSpace(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
}



void Canvas::resizeEvent(QResizeEvent* ){
    if (!lipid_space || !pointSets.size()) return;
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_space = (double)width() * 0.02;
    double h_space = (double)height() * 0.02;
    double w_rect = (double)width() * ((1. - 0.02 * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - 0.02 * ((double)tileRows + 1.)) / (double)tileRows);
    
    
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


void Canvas::refreshCanvas(){
    if (!lipid_space) return;
    
    Table* global_lipidome = lipid_space->global_lipidome;
    numTiles = 1 + lipid_space->lipidomes.size();
    tileColumns = ceil(sqrt((double)numTiles));
    
    
    if (!global_lipidome) return;
    
    pointSets.clear();
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    if (global_lipidome->m.cols >= 2){
        pointSets.push_back(new PointSet(global_lipidome->m.rows));
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
    
    
    for (Table* lipidome : lipid_space->lipidomes){
        pointSets.push_back(new PointSet(lipidome->m.rows));
        for (int r = 0; r < lipidome->m.rows; ++r){
            pointSets.back()->points[r].setX(lipidome->m(r, 0) * PRECESION_FACTOR);
            pointSets.back()->points[r].setY(lipidome->m(r, 1) * PRECESION_FACTOR);
        }
    }
    minMax.setRect(x_min, x_max, y_min, y_max);
    
    
    if ((x_max - x_min) / (double)width() > (y_max - y_min) / (double)height()){
        basescale = (x_max - x_min) / (double)width();
    }
    else {
        basescale = (y_max - y_min) / (double)height();
    }
    
    
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_space = (double)width() * 0.02;
    double h_space = (double)height() * 0.02;
    double w_rect = (double)width() * ((1. - 0.02 * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - 0.02 * ((double)tileRows + 1.)) / (double)tileRows);
    
    double ox = w_rect * 0.5 - ((x_max + x_min) / 2 / basescale);
    double oy = h_rect * 0.5 - ((y_max + y_min) / 2 / basescale);
    offset.setX(ox);
    offset.setY(oy);
    
    
    
    resizeEvent(0);
}

    
void Canvas::paintEvent(QPaintEvent *){
    if (!pointSets.size()) return;
    
    
    QPen pen2;
    pen2.setWidth(10 * basescale);
    pen2.setCapStyle(Qt::RoundCap);
    pen2.setColor(QColor(0, 0, 0, 255));
    
    QPainter painter(this);
    painter.setPen(pen);
    pen.setWidth(10 * basescale);
    pen.setCapStyle(Qt::RoundCap);
    painter.setBrush(brush);
    if (antialiased)
        painter.setRenderHint(QPainter::Antialiasing, true);

    pen.setColor(QColor(255, 0, 0, 128));

    for (auto pointSet : pointSets){
        painter.save();
        painter.translate(offset);
        painter.translate(QPointF(pointSet->bound.x(), pointSet->bound.y()));
        painter.scale(scaling / basescale, scaling / basescale);
        painter.drawPoints(pointSet->points, pointSet->len);
        painter.restore();
    }
    
    
    
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    
    for (auto pointSet : pointSets){
        painter.drawRect(pointSet->bound);
    }    
    
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
