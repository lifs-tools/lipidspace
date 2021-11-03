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
#include <math.h>
#include "lipidspace/lipidspace.h"

using namespace std;


class Canvas : public QWidget
{
    Q_OBJECT

public:

    explicit Canvas(QWidget *parent = nullptr);
    ~Canvas();
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setLipidSpace(LipidSpace *_lipid_space);
    void refreshCanvas();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPen pen;
    QBrush brush;
    bool antialiased;
    QPixmap pixmap;
    double scaling;
    double basescale;
    QPoint offset;
    bool mousePressed;
    QPoint deltaMouse;
    QPoint oldOffset;
    LipidSpace *lipid_space;
    double PRECESION_FACTOR;
};

#endif /* CANVAS_H */
