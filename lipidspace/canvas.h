#ifndef CANVAS_H
#define CANVAS_H

#include <QBrush>
#include <QPen>
#include <QFileInfo>
#include <QPixmap>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QMainWindow>
#include <QGraphicsItem>
#include <iostream>
#include <vector>
#include <map>
#include <QOpenGLWidget>
#include <math.h>
#include "lipidspace/lipidspace.h"
#include "lipidspace/lipidspacegui.h"

using namespace std;

#define MARGIN 0.01
#define POINT_BASE_SIZE 1.8
#define PRECESION_FACTOR 1.
#define LABEL_COLOR 200, 200, 200, 255
#define ALPHA 128


/*
class CanvasItem : public QGraphicsItem {
public:
    QString title;
    QRectF old_view;
    QRectF bound;
    
    CanvasItem(QGraphicsView *_view);
    ~CanvasItem();
    
    QGraphicsView *view;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override = 0;
    QRectF boundingRect() const override;
    void resize();
    void updateView();
};
*/


class Dendrogram : public QGraphicsItem {
public:
    QString title;
    QRectF bound;
    QGraphicsView *view;
    
    LipidSpace* lipid_space;
    vector<QString> dendrogram_titles;
    QVector<QLineF> lines;
    double dendrogram_factor;
    double pp;
    
    Dendrogram(LipidSpace* _lipid_space, QGraphicsView *_view);
    ~Dendrogram();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
};
    


class PointSet : public QGraphicsItem {
public:
    QString title;
    QRectF bound;
    QGraphicsView *view;
    
    vector<QPointF> points;
    Table* lipidome;
    vector<QString> labels;
    vector<QPointF> label_points;
    vector<QPointF> class_means;
    
    
    PointSet(Table* _lipidome, QGraphicsView *_view);
    ~PointSet();
    void set_labels();
    void automated_annotation(Array &xx, Array &yy, Matrix &label_points);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
};



class Canvas : public QGraphicsView
{
    Q_OBJECT

public:
    Canvas(LipidSpace *_lipid_space, QMainWindow *_mainWindow, int _num, QWidget *parent = nullptr);
    ~Canvas();
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *event);
    
    
public slots:
    void resetCanvas();
    void showHideQuant(bool _showQuant);
    void setTransforming(QRectF f, int _num);
    void setUpdate();
    
signals:
    void showMessage(QString message);
    void transforming(QRectF f, int _num);
    void doubleClicked(int);
    
    
private:
    QMainWindow *mainWindow;
    LipidSpace *lipid_space;
    int num;
    
    bool showQuant;
    QGraphicsScene scene;
    QPoint m_lastMousePos;
    bool leftMousePressed;
    QPoint oldCenter;
    PointSet *pointSet;
    Dendrogram *dendrogram;
    bool initialized;
};

#endif /* CANVAS_H */
