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





class Dendrogram : public QGraphicsItem {
public:
    LipidSpace* lipid_space;
    QRectF bound;
    vector<QString> dendrogram_titles;
    QVector<QLineF> lines;
    QString title;
    QGraphicsView *view;
    double dendrogram_factor;
    double pp;
    
    Dendrogram(LipidSpace* _lipid_space, QGraphicsView *_view);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
};
    


class PointSet : public QGraphicsItem {
public:
    vector<QPointF> points;
    Table* lipidome;
    QRectF bound;
    QString title;
    vector<QString> labels;
    vector<QPointF> label_points;
    vector<QPointF> class_means;
    QGraphicsView *view;
    
    
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
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *event);
    
    
public slots:
    void resetCanvas();
    void showHideQuant(bool _showQuant);
    void setScale(QWheelEvent *event, QRectF f, int _num);
    void setMove(QRectF f, int _num);
    void setUpdate();
    
signals:
    void showMessage(QString message);
    void scaling(QWheelEvent *event, QRectF f, int _num);
    void moving(QRectF f, int _num);
    void doubleClicked(int);
    
    
private:
    QMainWindow *mainWindow;
    LipidSpace *lipid_space;
    int num;
    
    bool showQuant;
    PointSet* pointSet;
    QGraphicsScene scene;
    QPoint m_lastMousePos;
    bool leftMousePressed;
    QPoint oldCenter;
    
};

/*
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
    void resizeEvent(QResizeEvent* event);

public slots:
    
    void setInputClasses(LipidSpace *_lipid_space, QMainWindow *_mainWindow);
    void refreshCanvas();
    void resetCanvas();
    
    void setLayout(int tileLayout);
    void showHideQuant(bool _showQuant);
    void showHideDendrogram(bool _showDendrogram);
    void showHideGlobalLipidome(bool _showGlobalLipidome);
    
    void enableView(bool);

protected:
    void paintEvent(QPaintEvent *event) override;
    
signals:
    void showMessage(QString message);

private:
    bool view_enabled;
    QPixmap pixmap;
    double scaling;
    double scaling_dendrogram;
    double basescale;
    QPointF offset;
    QPointF offset_dendrogram;
    Qt::MouseButton mousePressed;
    QPointF deltaMouse;
    QPointF oldOffset;
    QPointF oldOffset_dendrogram;
    LipidSpace *lipid_space;
    int alpha;
    int numTiles;
    int tileColumns;
    vector<PointSet*> pointSets;
    QRectF minMax;
    QRectF movePointSet;
    QPointF movePointSetStart;
    int moveSet;
    QPointF oldSize;
    map<string, QColor> colorMap;
    int color_counter;
    QMainWindow *mainWindow;
    bool showQuant;
    bool showDendrogram;
    bool showGlobalLipidome;
    int tileLayout;
    double dendrogram_factor;
    QImage logo;
    QColor label_color;
    
};
*/

#endif /* CANVAS_H */
