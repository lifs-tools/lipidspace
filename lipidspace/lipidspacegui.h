#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>
#include <QClipboard>
#include "ui_lipidspacegui.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/canvas.h"
#include "lipidspace/progressbar.h"
#include "lipidspace/managelipidomes.h"
#include "lipidspace/setalpha.h"
#include "lipidspace/setPCnum.h"
#include "lipidspace/selectpc.h"
#include "lipidspace/selectlipidomes.h"
#include "lipidspace/importtable.h"
#include "lipidspace/about.h"
#include "lipidspace/globaldata.h"
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/filterdialog.h"
#include "lipidspace/Tutorial.h"
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/statistics.h"
#include <thread>

#define ALPHABETICALLY_ASC "Alphabetically (Asc)"
#define ALPHABETICALLY_DESC "Alphabetically (Desc)"

QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class Canvas;
class Tutorial;
class LipidSpaceGUI;


class HomeItem : public QGraphicsItem {

public:
    QGraphicsView *view;

    HomeItem(QGraphicsView *v);
    ~HomeItem();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;


};



class TableWrapper : public QThread {
    Q_OBJECT

public:
    LipidSpaceGUI* lipid_space_gui;

    TableWrapper(LipidSpaceGUI* _lipid_space_gui);

    void run() override;
};




class DragLayer : public QWidget {
    Q_OBJECT

public:
    QPoint start_position;
    QPoint delta;
    int source_tile;
    LipidSpaceGUI *lipid_space_gui;

    explicit DragLayer(LipidSpaceGUI *_lipid_space_gui, QWidget *parent = 0);

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


class LipidSpaceGUI : public QMainWindow {
    Q_OBJECT

public:
    enum TileLayout {AUTOMATIC = 0, ONE_COLULMN = 1, TWO_COLUMNS = 2, THREE_COLUMNS = 3, FOUR_COLUMNS = 4, FIVE_COLUMNS = 5, SIX_COLUMNS = 6};

    Ui::LipidSpaceGUI *ui;
    LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    LipidSpace* lipid_space;
    QPixmap *arrow_bl;
    Tutorial *tutorial;
    ImportTable import_table;
    Statistics statisticsBoxPlot;
    Statistics statisticsHistogram;
    Statistics statisticsROCCurve;
    Statistics statisticsBarPlot;
    QLabel *select_tiles_information;
    set<int> *selected_d_lipidomes;
    bool showStudyLipidomes;
    bool showGlobalLipidome;
    TileLayout tileLayout;
    bool updating;
    bool selected_tiles_mode;
    Progressbar *progressbar;
    string keystrokes;
    Progress *progress;
    DragLayer *dragLayer;
    vector<Canvas*> canvases;
    bool knubbel;
    bool table_transposed;
    map<string, vector<pair<string, double>>> sortings[4];
    vector<QComboBox*> sorting_boxes;
    string hovered_box_plot_lipid;
    string lipid_for_deselect;
    TableWrapper table_wrapper;

    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void copy_to_clipboard();
    void closeEvent(QCloseEvent *event) override;


signals:
    void transforming(QRectF f);
    void updateCanvas();
    void exporting(string);
    void studyVariableChanged(string);
    void updateHighlightedPoints(vector<QString> *);
    void analysisCompleted();



public slots:
    void quitProgram();
    void openLists();
    void openTable();
    void openMzTabM();
    //void fill_table();
    void export_list();
    void loadTable(ImportData *import_data);
    void loadTable(ImportData *import_data, bool start_analysis);
    void resetAnalysis();
    void showMessage(QString message);
    void updateGUI();
    void showHideStudyLipidomes();
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
    void setSecondarySorting();
    void updateSecondarySorting(int);
    void runAnalysis();
    void completeFeatureAnalysis();
    void setTransforming(QRectF f);
    void setExport();
    void openSetAlpha();
    void openSelectLipidomes();
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
    void setStudyVariable(int pos);
    void itemChanged(QListWidgetItem *item);
    void studyVariableItemChanged(QTreeWidgetItem *item, int col);
    void studyVariableItemDoubleClicked(QTreeWidgetItem *item, int col);
    void updateSelectionView();
    void updateView(int);
    void updateTable();
    void check_all_entities();
    void uncheck_all_entities();
    void check_selected_entities();
    void uncheck_selected_entities();
    void toggle_all_entities();
    void toggle_selected_entities();
    void select_all_study_variables();
    void deselect_all_study_variables();
    void reset_all_study_variables();
    void setPieTree(int);
    void setKnubbel();
    void setDendrogramHeight(int);
    void setPieSize(int);
    void setNormalization(int);
    void selectDendrogramLipidomes();
    void setSelectedTilesMode();
    void lipidEntered(string _lipid_name);
    void lipidExited();
    void deselectHoveredLipid();

};
#endif // LIPIDSPACEGUI_H
