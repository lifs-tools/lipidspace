#include "lipidspace/canvas.h"


Canvas::Canvas(QWidget *parent) : QWidget(parent){
    mousePressed = false;
    antialiased = false;
    scaling = 1.;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    lipid_space = 0;
    basescale = 1;
    PRECESION_FACTOR = 10000.; 
}


Canvas::~Canvas(){
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


void Canvas::refreshCanvas(){
    if (!lipid_space) return;
    
    Table* global_lipidome = lipid_space->global_lipidome;
    if (!global_lipidome) return;
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    if (global_lipidome->m.cols >= 2){
        for (int r = 0; r < global_lipidome->m.rows; ++r){
            x_min = min(x_min, global_lipidome->m(r, 0) * PRECESION_FACTOR);
            x_max = max(x_max, global_lipidome->m(r, 0) * PRECESION_FACTOR);
            y_min = min(y_min, global_lipidome->m(r, 1) * PRECESION_FACTOR);
            y_max = max(y_max, global_lipidome->m(r, 1) * PRECESION_FACTOR);
        }
    }
    else {
        return;
    }
    x_min *= 1.1;
    x_max *= 1.1;
    y_min *= 1.1;
    y_max *= 1.1;
    
    
    if ((x_max - x_min) / (double)width() > (y_max - y_min) / (double)height()){
        basescale = (x_max - x_min) / (double)width();
    }
    else {
        basescale = (y_max - y_min) / (double)height();
    }
    double ox = (width() >> 1) - ((x_max + x_min) / 2 / basescale);
    double oy = (height() >> 1) - ((y_max + y_min) / 2 / basescale);
    offset.setX(ox);
    offset.setY(oy);
}

    
void Canvas::paintEvent(QPaintEvent *){
    if (!lipid_space) return;
    
    Table* global_lipidome = lipid_space->global_lipidome;
    if (!global_lipidome) return;
    
    QPoint *points = 0;
    int point_len = 0;
    if (global_lipidome->m.cols >= 2){
        points = new QPoint[global_lipidome->m.rows];
        point_len = global_lipidome->m.rows;
        for (int r = 0; r < global_lipidome->m.rows; ++r){
            points[r].setX(global_lipidome->m(r, 0) * PRECESION_FACTOR);
            points[r].setY(global_lipidome->m(r, 1) * PRECESION_FACTOR);
        }
    }
    if (!points) return;
    
    
    
    QPainter painter(this);
    painter.setPen(pen);
    pen.setWidth(10 * basescale);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(QColor(255, 0, 0, 128));
    painter.setBrush(brush);
    if (antialiased)
        painter.setRenderHint(QPainter::Antialiasing, true);

    
    painter.save();
    painter.translate(offset);
    painter.scale(scaling / basescale, scaling / basescale);
    painter.drawPoints(points, point_len);
    painter.restore();
    
    delete []points;

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
