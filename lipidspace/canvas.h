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
#include <QMainWindow>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include "lipidspace/lipidspace.h"

using namespace std;

#define MARGIN 0.01
#define POINT_BASE_SIZE 2.0

class PointSet {
public:
    QPointF* points;
    Table* table;
    QRectF bound;
    int len;
    QString title;
    bool is_dendrogram;
    QVector<QLineF> lines;
    QVector<QString> dendrogram_titles;
    
    PointSet(int, Table* _table, bool _is_dendrogram = false);
    ~PointSet();
    
};


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

protected:
    void paintEvent(QPaintEvent *event) override;
    
signals:
    void showMessage(QString message);

private:
    bool antialiased;
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
    
    static const vector<QColor> COLORS;
    static const double PRECESION_FACTOR;
};

#endif /* CANVAS_H */
