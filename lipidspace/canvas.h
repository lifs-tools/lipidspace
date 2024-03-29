#ifndef CANVAS_H
#define CANVAS_H

#include <QtGlobal>
#include <QBrush>
#include <QPen>
#include <QFileInfo>
#include <QPixmap>
#include <QWidget>
#include <QLabel>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMarginsF>
#endif
#include <QMouseEvent>
#include <QGraphicsView>
#include <QMainWindow>
#include <QGraphicsItem>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QPrinter>
#include <QCursor>
#include <QDesktopServices>
#include <QPushButton>
#include <iostream>
#include <QGraphicsSceneMouseEvent>
#include <vector>
#include <map>
#include <math.h>
#include "lipidspace/lipidspace.h"
#include "lipidspace/globaldata.h"

using namespace std;

#define MARGIN 0.01
#define POINT_BASE_FACTOR 5
#define POINT_BASE_SIZE 1.0
#define PIE_BASE_RADIUS 30.0
#define LABEL_COLOR 200, 200, 200, 255
#define sign_log(x) (x >= 0 ? log(x + 1) - 1 : -(log(-x + 1) - 1))
#define DENDROGRAM_LINE_SIZE 2.

class Canvas;
class Dendrogram;

enum LabelDirection {NoLabel, LabelLeft, LabelRight};


class DendrogramLine : public QGraphicsLineItem {
public:
    bool permanent;
    DendrogramLine* next_line;
    DendrogramLine* second_line;
    Dendrogram *dendrogram;
    DendrogramNode *d_node;
    int node;

    DendrogramLine(QLineF l, QPen p, Dendrogram* d);
    void update_width(double);
    void update_height_factor(double, QPointF *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    QPainterPath shape() const override;
    void highlight(bool);
    void make_permanent(bool);
};



class Citation : public QGraphicsTextItem {
public:
    Citation(const QString &text, QGraphicsItem *parent = nullptr);
    void mousePressEvent(QGraphicsSceneMouseEvent *) override;
};



class LSWidget : public QWidget {
    Q_OBJECT

public:
    LSWidget(QWidget *parent = nullptr);
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

signals:
    void openFiles(const QList<QUrl> &);
};



class LSListWidget : public QListWidget {
    Q_OBJECT

public:
    LSListWidget(QWidget *parent = nullptr);
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

signals:
    void openFiles(const QList<QUrl> &);
};



class LSTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    LSTreeWidget(QWidget *parent = nullptr);
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

signals:
    void openFiles(const QList<QUrl> &);
};


class HomeView : public QGraphicsView {
    Q_OBJECT

public:

    QPushButton *firstTutorialPushButton;
    QPushButton *secondTutorialPushButton;
    QPushButton *thirdTutorialPushButton;
    QPushButton *fourthTutorialPushButton;
    QGraphicsSvgItem *banner;
    QGraphicsSvgItem *LIFS;
    QGraphicsSvgItem *LIFS_monitor;
    Citation *citation;

    HomeView(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

signals:
    void openFiles(const QList<QUrl> &);
};


class DendrogramTitle {
public:
    QString title;
    bool highlighted;
    bool permanent;

    DendrogramTitle(QString t);
};


class Dendrogram : public QGraphicsItem {
public:
    QRectF bound;
    Canvas *view;

    vector<DendrogramTitle> dendrogram_titles;
    QVector<QLineF> lines;
    double dendrogram_x_factor;
    double dendrogram_y_factor;
    double dendrogram_height;
    double dendrogram_y_offset;
    string study_variable;
    double x_min_d;
    double x_max_d;
    double y_min_d;
    double y_max_d;
    double dwidth;
    double dheight;
    double ratio;
    double max_title_width;
    DendrogramLine *top_line;
    set<int> *highlighted_for_selection;

    Dendrogram(Canvas *_view);
    ~Dendrogram();
    void load();
    void clear();
    void add_dendrogram_lines(DendrogramNode *node, DendrogramLine* line = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    void recursive_paint(QPainter *painter, DendrogramNode *_node, int max_recursions, int recursion = 0);
    QRectF boundingRect() const override;
    void draw_pie(QPainter *painter, DendrogramNode *node, double threshold_leq, double pie_x, double pie_y, LabelDirection direction = NoLabel);
    void update_bounds(bool complete = true);
};


struct PCPoint {
    QPointF point;
    double normalized_intensity;
    double intensity;
    QColor* color;
    QString label;
    QGraphicsEllipseItem *item;
    int ref_lipid_species;
};


struct PCLabel {
    QString label;
    QPointF label_point;
    QPointF class_mean;
};


class PointSet : public QGraphicsItem {
public:
    QRectF bound;
    Canvas *view;
    QRectF old_view;
    QString title;
    QString variances;
    vector<PCPoint> points;
    set<QString> highlighted_points;
    vector<PCLabel> labels;


    PointSet(Canvas *_view);
    ~PointSet();
    void set_labels();
    void set_point_size();
    void automated_annotation(Array &xx, Array &yy);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
    void updateView(QRectF);
    void loadPoints();
    void resize();
};


double compute_scale(Lipidome *l, int i);


class Decoration : public QGraphicsItem {
public:
    Canvas *view;
    QString title;
    QString footer;

    Decoration(Canvas* v, QString t, QString f = "");
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QRectF boundingRect() const override;
};



class Canvas : public QGraphicsView {
    Q_OBJECT

public:
    QGraphicsScene graphics_scene;
    LipidSpace *lipid_space;
    QMainWindow *mainWindow = 0;
    int canvas_id;
    Lipidome* lipidome = 0;
    string lipidome_group_name;
    CanvasType canvas_type;
    bool hovered_for_swap;
    bool marked_for_selected_view;
    bool showQuant;
    bool dragging = false;
    QPoint m_lastMousePos;
    QPoint oldCenter;
    QRectF oldView;
    PointSet *pointSet = 0;
    Dendrogram *dendrogram;
    Decoration *decoration = 0;

    Canvas(QWidget *parent = nullptr);
    Canvas(LipidSpace *_lipid_space, int _canvas_id, int _num, CanvasType _canvas_type, QWidget *parent = nullptr, string group_name = "");
    ~Canvas();
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *event);
    void setDendrogramData(LipidSpace *_lipid_space);
    void resetDendrogram();
    void update_alpha();
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);


public slots:
    void resetCanvas();
    void showHideQuant(bool _showQuant);
    void setTransforming(QRectF f);
    void setUpdate();
    void exportAsPdf();
    void exportAsSvg();
    void hoverOver();
    void setSwap(int source);
    void reloadPoints();
    void setStudyVariable(string);
    void clear();
    void highlightPoints(QListWidget *);
    void moveToPoint(QListWidgetItem*);
    void contextMenu(QPoint pos);
    void setLabelSize(int);
    void setDendrogramHeight();
    void updateDendrogram();


signals:
    void showMessage(QString message);
    void transforming(QRectF f);
    void mouse(QMouseEvent* event, Canvas *_canvas);
    void swappingLipidomes(int source, int target);
    void context(Canvas *canvas, QPoint pos);
    void rightClick(QPoint pos, set<int> *selected_d_lipidomes = 0);
    void lipidsForSelection(vector<string> &l);
    void tileSelected();
    void openFiles(const QList<QUrl> &);
};

#endif /* CANVAS_H */
