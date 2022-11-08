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

enum SecondSteps {SStart, SLoadTable, SSeletionSection1, SSeletionSection2, SSorting, SSortingBars, SSortingPG, SNormalization, SGoToStudVarFilter, SFilterStudyVar, SLeftPanel, SLipidSpaces, SSpacesOptions, SSpacesSingleView, SSpacesSingleViewExplaination, SDendrogramClick, SDendrogram, SDendrogram2, SStatistics, SStatistics2, SRawClick, SRawTable, SFinish, SEnd};

enum ThirdSteps {TStart, TLoadTable, TFeaturePanel, TFeatureVisualization, TSwitchToStat1, TAssessStatistics1, TAssessStatistics2, TFeatureAnalysis, TFeatureAnalysis2, TAssessFeatureAnalysis1, TAssessFeatureAnalysis2, TAssessStatistics3, TAssessStatistics4, TAssessStatistics5, TFinish, TEnd};

enum FourthSteps {DStart, DFinish, DEnd};


class LipidSpaceGUI;


class ImageWidget : public QWidget {
    Q_OBJECT

public:
    QPixmap pixmap;
    double angle;
    int timerId;
    bool is_rotating;

    ImageWidget(QWidget *parent, QString s);


protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *) override;

public slots:
    void rotate();

signals:
    void clicked();
    void doubleClicked();
};


class Tutorial : public QFrame {
	Q_OBJECT

public:
    QFrame *tutorialFrame;
    QLabel *titleLabel;
    QLabel *informationLabel;
    ImageWidget *arrow_bl;
    ImageWidget *arrow_br;
    ImageWidget *arrow_lt;
    ImageWidget *arrow_lb;
    ImageWidget *arrow_tl;
    ImageWidget *arrow_tr;
    ImageWidget *arrow_rt;
    ImageWidget *arrow_rb;
    QPushButton *continuePushButton;
    QLabel *pagesLabel;
    ImageWidget *xLabel;
    vector<ImageWidget*> arrows;
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
    void wheelEvent(QWheelEvent *) override;

public slots:
    void x_clicked();
    void close_directly_tutorial();
    void close_tutorial(int state);
    void start_tutorial(TutorialType _tutorial_type);
    void tile_selection_changed();
    void action_performed();
    void tab_changed(int);
    void resize();
    void combobox_changed(int index);
    void spinbox_changed(int value);
};

#endif /* TUTORIAL_H */
