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
#define POINT_BASE_SIZE 1
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
    Citation(const QString &text, QGraphicsItem *parent = nullptr) : QGraphicsTextItem(text, parent){}
    
    void mousePressEvent(QGraphicsSceneMouseEvent *) override {
        QString link = "http://www.lifs-tools.org";
        QDesktopServices::openUrl(QUrl(link));
    }
};


class HomeView : public QGraphicsView {
public:
    
    QPushButton *firstTutorialPushButton;
    QPushButton *secondTutorialPushButton;
    QPushButton *thirdTutorialPushButton;
    QGraphicsSvgItem *banner;
    QGraphicsSvgItem *LIFS;
    Citation *citation;
    
    HomeView(QWidget *parent = nullptr) : QGraphicsView(parent){
        firstTutorialPushButton = new QPushButton(this);
        firstTutorialPushButton->setObjectName(QString::fromUtf8("firstTutorialPushButton"));
        firstTutorialPushButton->setGeometry(QRect(60, 360, 181, 31));
        firstTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "First Tutorial", nullptr));
        secondTutorialPushButton = new QPushButton(this);
        secondTutorialPushButton->setObjectName(QString::fromUtf8("secondTutorialPushButton"));
        secondTutorialPushButton->setGeometry(QRect(280, 360, 181, 31));
        secondTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "Second Tutorial", nullptr));
        thirdTutorialPushButton = new QPushButton(this);
        thirdTutorialPushButton->setObjectName(QString::fromUtf8("thirdTutorialPushButton"));
        thirdTutorialPushButton->setGeometry(QRect(60, 400, 181, 31));
        thirdTutorialPushButton->setText(QApplication::translate("LipidSpaceGUI", "Third Tutorial", nullptr));
        
        banner = 0;
        
        
    }
    
    void resizeEvent(QResizeEvent *) override {
        if (!banner) {
            banner = new QGraphicsSvgItem(QCoreApplication::applicationDirPath() + "/data/images/LipidSpace-banner.svg");
            scene()->addItem(banner);
            LIFS = new QGraphicsSvgItem(QCoreApplication::applicationDirPath() + "/data/images/LIFS.svg");
            scene()->addItem(LIFS);
            citation = new Citation("Citation: Kopczynski, Dominik et al. The Journal 47(11):08-15, 2022.");
            citation->setDefaultTextColor(Qt::white);
            scene()->addItem(citation);
        }
        
        double factor = min((double)width() / 1207., (double)height() / 483.);
        QFont font = firstTutorialPushButton->font();
        font.setPointSizeF(10 * factor);
        
        firstTutorialPushButton->setGeometry(QRect(60. * factor, 360. * factor, 181. * factor, 31. * factor));
        secondTutorialPushButton->setGeometry(QRect(280. * factor, 360. * factor, 181. * factor, 31. * factor));
        thirdTutorialPushButton->setGeometry(QRect(60. * factor, 400. * factor, 181. * factor, 31. * factor));
        
        firstTutorialPushButton->setFont(font);
        secondTutorialPushButton->setFont(font);
        thirdTutorialPushButton->setFont(font);
        
        double banner_factor = 92. / (double)banner->boundingRect().height() * factor;
        banner->setPos(60. * factor, 85. * factor);
        banner->setScale(banner_factor);
        
        double LIFS_factor = 18. / (double)LIFS->boundingRect().height() * factor;
        LIFS->setPos(60. * factor, 50. * factor);
        LIFS->setScale(LIFS_factor);
        
        QFont f = citation->font();
        f.setPointSizeF(11. * factor);
        citation->setFont(f);
        citation->setPos(60. * factor, 440. * factor);
        citation->setCursor(Qt::PointingHandCursor);
    }
};


class Dendrogram : public QGraphicsItem {
public:
    QRectF bound;
    Canvas *view;
    
    struct DendrogramTitle {
        QString title;
        bool highlighted;
        bool permanent;
        
        DendrogramTitle(QString t) : title(t), highlighted(false), permanent(false) {}
    };
    
    LipidSpace* lipid_space;
    vector<DendrogramTitle> dendrogram_titles;
    QVector<QLineF> lines;
    double dendrogram_x_factor;
    double dendrogram_y_factor;
    double dendrogram_height;
    double dendrogram_y_offset;
    string feature;
    double x_min_d;
    double x_max_d;
    double y_min_d;
    double y_max_d;
    double dwidth;
    double dheight;
    DendrogramLine *top_line;
    set<int> *highlighted_for_selection;
    
    Dendrogram(LipidSpace* _lipid_space, Canvas *_view);
    ~Dendrogram();
    void load();
    void clear();
    void add_dendrogram_lines(DendrogramNode *node, DendrogramLine* line = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    void recursive_paint(QPainter *painter, DendrogramNode *_node, int max_recursions, int recursion = 0);
    QRectF boundingRect() const override;
    void draw_pie(QPainter *painter, DendrogramNode *node, double threshold_leq, double pie_x, double pie_y, LabelDirection direction = NoLabel);
};
    

struct PCPoint {
    QPointF point;
    double intensity;
    QColor color;
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
    Lipidome* lipidome;
    Canvas *view;
    QRectF old_view;
    QString title;
    QString variances;
    vector<PCPoint> points;
    set<QString> highlighted_points;
    vector<PCLabel> labels;
    
    
    PointSet(Lipidome* _lipidome, Canvas *_view);
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
    void update_alpha();
    
    
public slots:
    void resetCanvas();
    void showHideQuant(bool _showQuant);
    void setTransforming(QRectF f);
    void setUpdate();
    void exportAsPdf();
    void hoverOver();
    void setSwap(int source);
    void reloadPoints();
    void setFeature(string);
    void clear();
    void highlightPoints();
    void moveToPoint(QListWidgetItem*);
    void contextMenu(QPoint pos);
    void setLabelSize(int);
    
    
signals:
    void showMessage(QString message);
    void transforming(QRectF f);
    void doubleClicked(int);
    void mouse(QMouseEvent* event, Canvas *_canvas);
    void swappingLipidomes(int source, int target);
    void context(Canvas *canvas, QPoint pos);
    void rightClick(QPoint pos, set<int> *selected_d_lipidomes = 0);
    
    
private:
    
    bool showQuant;
    QPoint m_lastMousePos;
    bool leftMousePressed;
    QPoint oldCenter;
    PointSet *pointSet;
    Dendrogram *dendrogram;
    QListWidget *listed_species;
};

#endif /* CANVAS_H */
