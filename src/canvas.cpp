#include "lipidspace/canvas.h"


Canvas::Canvas(QWidget *parent) : QWidget(parent){
    mousePressed = false;
    antialiased = false;
    scaling = 1.;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}




void Canvas::mousePressEvent(QMouseEvent *event){
    if (!mousePressed){
        mousePressed = true;
        deltaMouse = event->pos();
        oldOffset = QPoint(offset.x(), offset.y());
    }
    
}


void Canvas::mouseReleaseEvent(QMouseEvent *event){
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
        scaling = max(0.1, scaling / 1.1);
    }
    else {
        scaling *= 1.1;
    }
    QPoint mouse = event->pos();
    offset.setX(mouse.x() + (offset.x() - mouse.x()) / old_scaling * scaling);
    offset.setY(mouse.y() + (offset.y() - mouse.y()) / old_scaling * scaling);
    
    update();
}


    
void Canvas::paintEvent(QPaintEvent *){
    static const QPoint points[4] = {
        QPoint(100, 80),
        QPoint(20, 10),
        QPoint(80, 30),
        QPoint(90, 70)
    };
    
    
    QPainter painter(this);
    painter.setPen(pen);
    pen.setWidth(10);
    pen.setCapStyle(Qt::RoundCap);
    painter.setBrush(brush);
    if (antialiased)
        painter.setRenderHint(QPainter::Antialiasing, true);

    
    painter.save();
    painter.translate(offset);
    painter.scale(scaling, scaling);
    painter.drawPoints(points, 4);
    painter.restore();
    

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
