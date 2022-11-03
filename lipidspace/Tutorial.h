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
#include <unistd.h>

#define instanceof(ptr, base) (dynamic_cast<const base*>(ptr) != nullptr)

using namespace std;


enum Arrow {ABL = 0, ABR = 1, ALT = 2, ALB = 3, ATL = 4, ATR = 5, ART = 6, ARB = 7};
enum TutorialType {NoTutorial, FirstTutorial, SecondTutorial, ThirdTutorial, FourthTutorial};

enum FirstSteps {FStart, FDescription, FFindImport, FOpenImport, FEnteredImport, FExplainRow, FExplainColumn, FExplainFlat, FShowPreview, FSelectColumnTable, FExplainColumnField, FExplainLipidColumnField, FExplainSampleColumnField, FExplainStudyFields, FSampleEntryAssignment, FStudyVarAssignment, FLipidAssignment, FStudyVarMapping, FFinishImport, FFinish, FEnd};

enum SecondSteps {SStart, SLoadTable, SSeletionSection1, SSeletionSection2, SSorting, SSortingBars, SSortingPG, SNormalization, SGoToStudVarFilter, SFilterStudyVar, SLeftPanel, SLipidSpaces, SSpacesOptions, SSpacesSingleView, SSpacesSingleViewExplaination, SDendrogramClick, SDendrogram, SDendrogram2, SStatistics, Statistics2, SFinish, SEnd};

enum ThirdSteps {TStart, TEnd};

enum FourthSteps {DStart, DEnd};

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
    map<QObject*, bool> main_widgets;

    explicit Tutorial(LipidSpaceGUI *lipidSpaceGUI, QWidget *parent = 0);
    ~Tutorial();

    static const vector<FirstSteps> first_tutorial_steps_order;
    static const vector<SecondSteps> second_tutorial_steps_order;
    static const vector<ThirdSteps> third_tutorial_steps_order;
    static const vector<FourthSteps> fourth_tutorial_steps_order;

    void show_arrow(Arrow, QWidget *, int x, int y);
    void show_arrow(Arrow, QWidget *, QPoint);
    QPoint map_widget(QWidget *widget, QWidget *main);
    bool can_start_tutorial();
    void first_tutorial_steps();
    void second_tutorial_steps();
    void third_tutorial_steps();
    void fourth_tutorial_steps();
    void continue_tutorial();
    void disable();
    void hide_arrows();
    void move(int x, int y, QWidget *w = 0);
    void changeSize(int w, int h);
    void item_changed(const QModelIndex &parent, int first, int last);

public slots:
    void x_clicked();
    void close_directly_tutorial();
    void close_tutorial(int state);
    void start_first_tutorial();
    void start_second_tutorial();
    void tile_selection_changed();
    void action_performed();
    void tab_changed(int);
    void resize();
    void combobox_changed(int index);
};

#endif /* TUTORIAL_H */
