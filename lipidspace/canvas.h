#ifndef CANVAS_H
#define CANVAS_H

#include <QBrush>
#include <QPen>
#include <QFileInfo>
#include <QPixmap>
#include <QWidget>
#include <QLabel>
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
#include "lipidspace/globaldata.h"

using namespace std;

#define MARGIN 0.01
#define POINT_BASE_SIZE 1.8
#define LABEL_COLOR 200, 200, 200, 255

class Canvas;



class Dendrogram : public QGraphicsItem {
public:
    QRectF bound;
    Canvas *view;
    
    LipidSpace* lipid_space;
    vector<QString> dendrogram_titles;
    QVector<QLineF> lines;
    double dendrogram_x_factor;
    double dendrogram_y_factor;
    string feature;
    
    Dendrogram(LipidSpace* _lipid_space, Canvas *_view);
    ~Dendrogram();
    void load();
    void clear();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    void recursive_paint(QPainter *painter, DendrogramNode *_node, int max_recursions, int recursion = 0);
    QRectF boundingRect() const override;
};
    

struct PCPoint {
    QPointF point;
    double intensity;
    QColor color;
    QString label;
};


struct PCLabel {
    QString label;
    QPointF label_point;
    QPointF class_mean;
};


class PointSet : public QGraphicsItem {
public:
    QRectF bound;
    Table* lipidome;
    Canvas *view;
    QRectF old_view;
    
    vector<PCPoint> points;
    set<QString> highlighted_points;
    vector<PCLabel> labels;
    
    
    PointSet(Table* _lipidome, Canvas *_view);
    ~PointSet();
    void set_labels();
    void automated_annotation(Array &xx, Array &yy);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
    void updateView(QRectF);
    void loadPoints();
    void resize();
};



class Canvas : public QGraphicsView
{
    Q_OBJECT

public:
    QGraphicsScene graphics_scene;
    LipidSpace *lipid_space;
    QMainWindow *mainWindow;
    int num;
    bool hovered_for_swap;
    
    Canvas(QWidget *parent = nullptr);
    Canvas(LipidSpace *_lipid_space, int _num, QListWidget* _listed_species, QWidget *parent = nullptr);
    ~Canvas();
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *event);
    void setDendrogramData(LipidSpace *_lipid_space);
    void resetDendrogram();
    
    
public slots:
    void resetCanvas();
    void showHideQuant(bool _showQuant);
    void setTransforming(QRectF f);
    void setUpdate();
    void exportPdf(QString outputFolder);
    void setInitialized();
    void hoverOver();
    void setSwap(int source);
    void reloadPoints();
    void setFeature(string);
    void clear();
    void highlightPoints();
    void moveToPoint(QListWidgetItem*);
    
    
signals:
    void showMessage(QString message);
    void transforming(QRectF f);
    void doubleClicked(int);
    void mouse(QMouseEvent* event, Canvas *_canvas);
    void swappingLipidomes(int source, int target);
    
    
private:
    
    bool showQuant;
    QPoint m_lastMousePos;
    bool leftMousePressed;
    QPoint oldCenter;
    PointSet *pointSet;
    Dendrogram *dendrogram;
    bool initialized;
    QLabel title;
    QLabel *variances;
    QListWidget *listed_species;
};

#endif /* CANVAS_H */