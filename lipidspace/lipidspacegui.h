#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QClipboard>
#include "ui_lipidspacegui.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/SvgPathHandler.h"
#include "lipidspace/canvas.h"
#include "lipidspace/progressbar.h"
#include "lipidspace/managelipidomes.h"
#include "lipidspace/setalpha.h"
#include "lipidspace/setPCnum.h"
#include "lipidspace/selectpc.h"
#include "lipidspace/importtable.h"
#include "lipidspace/about.h"
#include "lipidspace/globaldata.h"
#include "lipidspace/CBTableWidget.h"
#include "lipidspace/filterdialog.h"
#include "lipidspace/Tutorial.h"
#include "cppgoslin/cppgoslin.h"
#include <thread>

#define ALPHABETICALLY_ASC "Alphabetically (Asc)"
#define ALPHABETICALLY_DESC "Alphabetically (Desc)"

QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class Canvas;
class Tutorial;



class HomeItem : public QGraphicsItem {
    
public:
    QGraphicsView *view;
    
    HomeItem(QGraphicsView *v) : view(v) {
    }
    
    ~HomeItem(){
    }
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override {
        QLinearGradient gradient;
        double factor = min((double)view->width() / 1207., (double)view->height() / 483.);

        painter->setRenderHint(QPainter::Antialiasing, true);
        
        
        gradient.setStart(0, 0);
        gradient.setFinalStop(0, 1);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0.0, Qt::white);
        gradient.setColorAt(0.04, Qt::white);
        gradient.setColorAt(0.08, QColor("#e4eff7")); // e4eff7 / f7f0e4
        gradient.setColorAt(0.6, QColor("#6babce")); // 6babce / ceab6b
        painter->setBrush(gradient);
        painter->setPen(Qt::NoPen);
        painter->drawRect(0, 0, view->width(), 400 * factor);
        
        
        
        double l_banner = 502.5 * factor;
        
        QBrush bottom(QColor("#6babce")); // 6babce / ceab6b
        painter->setPen(Qt::NoPen);
        painter->setBrush(bottom);
        painter->drawRect(0., 400. * factor, view->width(), view->height());
        
        
        QBrush banner(QColor("#71a9cc")); // 71a9cc / ce956b
        painter->setBrush(banner);
        painter->drawRect(QRectF(0., 90. * factor, 50. * factor, 70. * factor));
        painter->drawRect(QRectF(l_banner + 70. * factor, 90. * factor, view->width(), 70. * factor));
        
        
        // Write the text
        QFont f = painter->font();
        painter->setPen(QPen(Qt::white));
        f.setPointSizeF(11. * factor);
        painter->setFont(f);
        painter->drawText(QRectF(60. * factor, 190. * factor, 1000., 400.), Qt::AlignTop | Qt::AlignLeft, "Accessing the chemical space of individual lipidomes:\n  • Comprehensive study of multiple lipidomes\n  • Providing mechanism for quality control\n  • Feature analysis and lipid selection\n  • Integration of study variables");
        
        painter->drawText(QRectF(60. * factor, 310. * factor, 1000., 400.), Qt::AlignTop | Qt::AlignLeft, "LipidSpace offers several interactive tutorials for an easy introduction\ninto its functionality:");
        
        
        //painter->drawText(QRectF(60. * factor, 440. * factor, 1000., 400.), Qt::AlignTop | Qt::AlignLeft, "Citation: Kopczynski, Dominik et al. The Journal 47(11):08-15, 2022.");
        
    }
    
    QRectF boundingRect() const override {
        return QRectF(0, 0, view->width(), view->height());
    }
    
    
};




class DragLayer : public QWidget {
    Q_OBJECT
    
public:
    QPoint start_position;
    QPoint delta;
    int source_tile;
    
    explicit DragLayer(QWidget *parent = 0);
    
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    
public slots:
    void mousePressEvent(QMouseEvent* event, Canvas *canvas);
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
signals:
    void hover();
    void swapping(int);
    
};


class LipidSpaceGUI : public QMainWindow
{
    Q_OBJECT

public:
    Ui::LipidSpaceGUI *ui;
    LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    LipidSpace* lipid_space;
    QPixmap *arrow_bl;
    Tutorial *tutorial;
    ImportTable import_table;
    bool ctrl_pressed;
    
    enum TileLayout {AUTOMATIC = 0, ONE_COLULMN = 1, TWO_COLUMNS = 2, THREE_COLUMNS = 3, FOUR_COLUMNS = 4, FIVE_COLUMNS = 5, SIX_COLUMNS = 6};
    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void copy_to_clipboard();
    void closeEvent(QCloseEvent *event) override;
    
    
signals:
    void transforming(QRectF f);
    void updateCanvas();
    void exporting(string);
    void featureChanged(string);
    void updateHighlightedPoints(vector<QString> *);
    void analysisCompleted();
    
    
public slots:
    void quitProgram();
    void openLists();
    void openTable();
    void openMzTabM();
    void fill_table();
    void export_list();
    void loadTable(string file_name, vector<TableColumnType> *column_types, TableType table_type, string sheet);
    void resetAnalysis();
    void showMessage(QString message);
    void updateGUI();
    void showHideDendrogram();
    void showHideGlobalLipidome();
    void toggleLipidNameTranslation();
    void showHideQuant();
    void toggleQuant();
    void toggleBoundMetric();
    void setAutomaticLayout();
    void setCompleteLinkage();
    void setAverageLinkage();
    void setSingleLinkage();
    void set1ColumnLayout();
    void set2ColumnLayout();
    void set3ColumnLayout();
    void set4ColumnLayout();
    void set5ColumnLayout();
    void set6ColumnLayout();
    void setSnPositions();
    void startFeatureAnalysis();
    void openManageLipidomesWindow();
    void runAnalysis();
    void setTransforming(QRectF f);
    void setDoubleClick(int _num);
    void setExport();
    void openSetAlpha();
    void openSetPCnum();
    void openSelectPC();
    void openAbout();
    void openLog();
    void export_table();
    void swapLipidomes(int source, int target);
    void ShowContextMenu(const QPoint);
    void ShowContextMenuStatisticsBoxPlot(const QPoint);
    void ShowContextMenuStatisticsBarPlot(const QPoint);
    void ShowContextMenuStatisticsHistogram(const QPoint);
    void ShowContextMenuStatisticsROCCurve(const QPoint);
    void ShowTableContextMenu(const QPoint);
    void ShowContextMenuDendrogram(const QPoint, set<int> *selected_d_lipidomes = 0);
    void ShowContextMenuLipidome(Canvas *canvas, const QPoint);
    void transposeTable();
    void reassembleSelection();
    void setFeature(int pos);
    void setFeatureStat(int pos);
    void itemChanged(QListWidgetItem *item);
    void featureItemChanged(QTreeWidgetItem *item, int col);
    void featureItemDoubleClicked(QTreeWidgetItem *item, int col);
    void updateSelectionView();
    void updateView(int);
    void check_all_entities();
    void uncheck_all_entities();
    void toggle_all_entities();
    void select_all_features();
    void deselect_all_features();
    void reset_all_features();
    void setPieTree(int);
    void setKnubbel();
    void setDendrogramHeight(int);
    void setPieSize(int);
    void setNormalization(int);
    void selectDendrogramLipidomes();
    
    
private:
    set<int> *selected_d_lipidomes;
    bool showDendrogram;
    bool showGlobalLipidome;
    TileLayout tileLayout;
    bool updating;
    Progressbar *progressbar;
    string keystrokes;
    Progress *progress;
    int single_window;
    DragLayer *dragLayer;
    vector<Canvas*> canvases;
    bool knubbel;
    bool table_transposed;
    map<string, vector<pair<string, double>>> sortings[4];
    vector<QComboBox*> sorting_boxes;
    
};
#endif // LIPIDSPACEGUI_H
