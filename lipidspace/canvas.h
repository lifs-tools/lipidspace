#ifndef CANVAS_H
#define CANVAS_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <iostream>


class Canvas : public QWidget
{
    Q_OBJECT

public:
    enum Shape { Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc, Chord, Pie, Path, Text, Pixmap };

    explicit Canvas(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void wheelEvent(QWheelEvent *event);
    

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Shape shape;
    QPen pen;
    QBrush brush;
    bool antialiased;
    bool transformed;
    QPixmap pixmap;
};

#endif /* CANVAS_H */
