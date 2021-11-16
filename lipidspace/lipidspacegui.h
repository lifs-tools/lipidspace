#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_lipidspacegui.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/canvas.h"
#include "lipidspace/progressbar.h"
#include "lipidspace/managelipidomes.h"
#include "lipidspace/setalpha.h"
#include "lipidspace/setPCnum.h"
#include "lipidspace/about.h"
#include "cppgoslin/cppgoslin.h"
#include <thread>

#define DEFAULT_ALPHA 128

QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class Canvas;


class DragLayer : public QWidget {
    Q_OBJECT
    
public:
    QPoint delta;
    
    explicit DragLayer(QWidget *parent = 0);
    
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    
public slots:
    void mousePressEvent(QMouseEvent* event, Canvas *canvas);
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
signals:
    void hover();
};


class LipidSpaceGUI : public QMainWindow
{
    Q_OBJECT

public:
    LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    LipidSpace* lipid_space;
    
    enum TileLayout {AUTOMATIC = 0, ONE_COLULMN = 1, TWO_COLUMNS = 2, THREE_COLUMNS = 3, FOUR_COLUMNS = 4, FIVE_COLUMNS = 5, SIX_COLUMNS = 6};
    static int color_counter;
    static const vector<QColor> COLORS;
    static bool showQuant;
    static int alpha;
    static map<string, QColor> colorMap;
    static int PC1;
    static int PC2;
    void resizeEvent(QResizeEvent *) override;
    
    
signals:
    void transforming(QRectF f, int _num);
    void updateCanvas();
    void exporting(QString);
    void initialized();
    
    
public slots:
    void quitProgram();
    void openLists();
    void openTable();
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
    void setTransforming(QRectF f, int _num);
    void setDoubleClick(int _num);
    void setExport();
    void setInitialized();
    void openSetAlpha();
    void openSetPCnum();
    void openAbout();
    
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
};
#endif // LIPIDSPACEGUI_H
