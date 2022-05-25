#ifndef TUTORIAL_H
#define TUTORIAL_H


#include <QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QMessageBox>
#include <QtWidgets/QWidget>
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/lipidspacegui.h"
#include <iostream>

#define instanceof(ptr, base) (dynamic_cast<const base*>(ptr) != nullptr)

using namespace std;


enum Arrow {ABL, ABR, ALT, ALB, ATL, ATR, ART, ARB};
enum TutorialType {NoTutorial, FirstTutorial};
enum FirstSteps {FStart, FDescription, FEnd};

class LipidSpaceGUI;

class Tutorial : public QFrame {
public:
    QFrame *tutorialFrame;
    QLabel *titleLabel;
    QLabel *informationLabel;
    QLabel *arrow_bl;
    QLabel *arrow_br;
    QLabel *arrow_lt;
    QLabel *arrow_lb;
    QLabel *arrow_tl;
    QLabel *arrow_tr;
    QLabel *arrow_rt;
    QLabel *arrow_rb;
    QPushButton *continuePushButton;
    QLabel *pagesLabel;
    ClickableLabel *xLabel;
    vector<QLabel*> arrows;
    LipidSpaceGUI *lipidSpaceGUI;
    TutorialType tutorialType = NoTutorial;
    int step = -1;
    vector<QObject*> main_widgets;
    
    Tutorial(LipidSpaceGUI *lipidSpaceGUI, QWidget *parent = 0);
    ~Tutorial();
    
    void show_arrow(Arrow, QWidget *, int x, int y);
    void show_arrow(Arrow, QWidget *, QPoint);
    QPoint map_widget(QWidget *widget);
    bool can_start_tutorial();
    void first_tutorial_steps();
    void continue_tutorial();
    void close_tutorial();
    void disable();
    
public slots:
    void x_clicked();
    void start_first_tutorial();
};

#endif /* TUTORIAL_H */
