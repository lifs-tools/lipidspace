#include "canvas.h"


Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    shape = Ellipse;
    antialiased = false;
    transformed = false;
    
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}


QSize Canvas::minimumSizeHint() const
{
    return QSize(100, 100);
}


void Canvas::wheelEvent(QWheelEvent *event){
    
    std::cout << "wheel" << std::endl;
}




QSize Canvas::sizeHint() const
{
    return QSize(400, 200);
}


    
void Canvas::paintEvent(QPaintEvent *){
    static const QPoint points[4] = {
        QPoint(10, 80),
        QPoint(20, 10),
        QPoint(80, 30),
        QPoint(90, 70)
    };
    
    
    QPainter painter(this);
    painter.setPen(pen);
    pen.setWidth(30);
    pen.setCapStyle(Qt::RoundCap);
    painter.setBrush(brush);
    if (antialiased)
        painter.setRenderHint(QPainter::Antialiasing, true);

    painter.drawPoints(points, 4);
    
    painter.save();
    /*
    painter.translate(x, y);
    
    if (transformed) {
        painter.translate(50, 50);
        painter.rotate(60.0);
        painter.scale(0.6, 0.9);
        painter.translate(-50, -50);
    }
    */
    
    
    painter.restore();

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
