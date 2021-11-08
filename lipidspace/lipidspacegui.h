#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_lipidspacegui.h"
#include "lipidspace/lipidspace.h"
#include "cppgoslin/cppgoslin.h"


QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class LipidSpaceGUI : public QMainWindow
{
    Q_OBJECT

public:
    LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    
    enum TileLayout {AUTOMATIC, ONE_COLULMN, TWO_COLUMNS, THREE_COLUMNS, FOUR_COLUMNS, FIVE_COLUMNS, SIX_COLUMNS};
    
    
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
    void setAutomaticLayout();
    void set1ColumnLayout();
    void set2ColumnLayout();
    void set3ColumnLayout();
    void set4ColumnLayout();
    void set5ColumnLayout();
    void set6ColumnLayout();
    void setSnPositions();

private:
    Ui::LipidSpaceGUI *ui;
    LipidSpace* lipid_space;
    bool showQuant;
    bool showDendrogram;
    bool showGlobalLipidome;
    TileLayout tileLayout;
    bool updating;
    
};
#endif // LIPIDSPACEGUI_H
