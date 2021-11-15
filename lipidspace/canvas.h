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
#include <QOpenGLWidget>
#include <QPrinter>
#include <QCursor>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include "lipidspace/lipidspace.h"
#include "lipidspace/lipidspacegui.h"

using namespace std;

#define MARGIN 0.01
#define POINT_BASE_SIZE 1.8
#define PRECESION_FACTOR 1.
#define LABEL_COLOR 200, 200, 200, 255

class Canvas;


class Dendrogram : public QGraphicsItem {
public:
    QString title;
    QRectF bound;
    Canvas *view;
    
    LipidSpace* lipid_space;
    vector<QString> dendrogram_titles;
    QVector<QLineF> lines;
    double dendrogram_factor;
    double pp;
    
    Dendrogram(LipidSpace* _lipid_space, Canvas *_view);
    ~Dendrogram();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
};
    


class PointSet : public QGraphicsItem {
public:
    QString title;
    QRectF bound;
    Canvas *view;
    
    QRectF old_view;
    vector<QPointF> points;
    Table* lipidome;
    vector<QString> labels;
    vector<QPointF> label_points;
    vector<QPointF> class_means;
    
    
    PointSet(Table* _lipidome, Canvas *_view);
    ~PointSet();
    void set_labels();
    void automated_annotation(Array &xx, Array &yy, Matrix &label_points);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
    void updateView(QRectF);
    void resize();
};



class Canvas : public QGraphicsView
{
    Q_OBJECT

public:
    QGraphicsScene graphics_scene;
    LipidSpace *lipid_space;
    QMainWindow *mainWindow;
    
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
    void exportPdf(QString outputFolder);
    void setInitialized();
    
signals:
    void showMessage(QString message);
    void transforming(QRectF f, int _num);
    void doubleClicked(int);
    
    
private:
    int num;
    
    bool showQuant;
    QPoint m_lastMousePos;
    bool leftMousePressed;
    QPoint oldCenter;
    PointSet *pointSet;
    Dendrogram *dendrogram;
    bool initialized;
};

#endif /* CANVAS_H */
