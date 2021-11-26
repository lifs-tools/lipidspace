#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include "ui_lipidspacegui.h"
#include "lipidspace/lipidspace.h"
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
#include "cppgoslin/cppgoslin.h"
#include <thread>


QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class Canvas;



class DragLayer : public QWidget {
    Q_OBJECT
    
public:
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
    LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    LipidSpace* lipid_space;
    
    enum TileLayout {AUTOMATIC = 0, ONE_COLULMN = 1, TWO_COLUMNS = 2, THREE_COLUMNS = 3, FOUR_COLUMNS = 4, FIVE_COLUMNS = 5, SIX_COLUMNS = 6};
    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool loadedDataSet;
    
    
signals:
    void transforming(QRectF f);
    void updateCanvas();
    void exporting(QString);
    void initialized();
    void featureChanged(string);
    
    
public slots:
    void quitProgram();
    void openLists();
    void openTable();
    void loadTable(string file_name, vector<TableColumnType> *column_types, TableType table_type);
    void resetAnalysis();
    void showMessage(QString message);
    void updateGUI();
    void showHideDendrogram();
    void showHideGlobalLipidome();
    void showHideQuant();
    void toggleQuant();
    void toggleBoundMetric();
    void setAutomaticLayout();
    void set1ColumnLayout();
    void set2ColumnLayout();
    void set3ColumnLayout();
    void set4ColumnLayout();
    void set5ColumnLayout();
    void set6ColumnLayout();
    void setSnPositions();
    void openManageLipidomesWindow();
    void runAnalysis();
    void setTransforming(QRectF f);
    void setDoubleClick(int _num);
    void setExport();
    void setInitialized();
    void openSetAlpha();
    void openSetPCnum();
    void openSelectPC();
    void openAbout();
    void openLog();
    void swapLipidomes(int source, int target);
    void ShowContextMenu(const QPoint);
    void transposeTable();
    void reassembleSelection();
    void setFeature(int pos);
    void SpeciesItemChanged(QListWidgetItem *item);
    void updateSelectionView();
    
private:
    Ui::LipidSpaceGUI *ui;
    bool showDendrogram;
    bool showGlobalLipidome;
    TileLayout tileLayout;
    bool updating;
    Progressbar *progressbar;
    Progress *progress;
    int single_window;
    QTimer timer;
    DragLayer *dragLayer;
    vector<Canvas*> canvases;
    bool table_transposed;
    
    void fill_Table();
};
#endif // LIPIDSPACEGUI_H
