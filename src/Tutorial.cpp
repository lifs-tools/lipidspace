#include "lipidspace/Tutorial.h"


Tutorial::Tutorial(LipidSpaceGUI * _lipidSpaceGUI, QWidget *parent) : QFrame(parent), lipidSpaceGUI(_lipidSpaceGUI) {
    setObjectName(QString::fromUtf8("TutorialFrame"));
    setGeometry(QRect(20, 110, 621, 251));
    setStyleSheet(QString::fromUtf8("QFrame#TutorialFrame {\n"
"	border: 7px solid #000000;\n"
"	background-color: rgb(255, 255, 255);\n"
"}"));
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setLineWidth(10);
    titleLabel = new QLabel(this);
    titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
    titleLabel->setGeometry(QRect(20, 20, 481, 61));
    QFont font2;
    font2.setPointSize(16);
    font2.setBold(true);
    titleLabel->setFont(font2);
    titleLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    titleLabel->setWordWrap(true);
    informationLabel = new QLabel(this);
    informationLabel->setObjectName(QString::fromUtf8("informationLabel"));
    informationLabel->setGeometry(QRect(20, 90, 581, 131));
    QFont font3;
    font3.setPointSize(12);
    informationLabel->setFont(font3);
    informationLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    informationLabel->setWordWrap(true);
    continuePushButton = new QPushButton(this);
    continuePushButton->setObjectName(QString::fromUtf8("continuePushButton"));
    continuePushButton->setGeometry(QRect(width() - 105, 50, 80, 26));
    continuePushButton->setStyleSheet(QString::fromUtf8(""));
    continuePushButton->setText(QApplication::translate("LipidSpaceGUI", "Continue", nullptr));
    connect(continuePushButton, &QPushButton::clicked, this, &Tutorial::continue_tutorial);
    pagesLabel = new QLabel(this);
    pagesLabel->setObjectName(QString::fromUtf8("pagesLabel"));
    pagesLabel->setGeometry(QRect(width() - 126, 220, 101, 20));
    pagesLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    xLabel = new ClickableLabel(this);
    xLabel->setObjectName(QString::fromUtf8("xLabel"));
    xLabel->setGeometry(QRect(550, 18, 51, 20));
    xLabel->move(width() - 25, 10);
    connect(xLabel, &ClickableLabel::clicked, this, &Tutorial::x_clicked);
    
    QPixmap close_x("data/images/close-x.png");
    xLabel->setPixmap(close_x);
    xLabel->setAlignment(Qt::AlignLeading|Qt::AlignRight|Qt::AlignTop);
    QRect r = xLabel->geometry();
    r.setWidth(close_x.size().width());
    r.setHeight(close_x.size().height());
    xLabel->setGeometry(r);
    
    titleLabel->setText("Welcome");
    informationLabel->setText("Welcome to the first tutorial. We will guide you interactively through LipidSpace.");
    pagesLabel->setText("1 / 29");
    setVisible(false);
    
    QPixmap p_arrow_bl("data/images/arrow-bottom-left.png");
    arrow_bl = new QLabel();
    arrow_bl->setPixmap(p_arrow_bl);
    r = arrow_bl->geometry();
    r.setWidth(p_arrow_bl.size().width());
    r.setHeight(p_arrow_bl.size().height());
    arrow_bl->setGeometry(r);
    arrow_bl->setVisible(false);
    arrows.push_back(arrow_bl);
    
    QPixmap p_arrow_br("data/images/arrow-bottom-right.png");
    arrow_br = new QLabel();
    arrow_br->setPixmap(p_arrow_br);
    r = arrow_br->geometry();
    r.setWidth(p_arrow_br.size().width());
    r.setHeight(p_arrow_br.size().height());
    arrow_br->setGeometry(r);
    arrow_br->setVisible(false);
    arrows.push_back(arrow_br);
    
    QPixmap p_arrow_lt("data/images/arrow-left-top.png");
    arrow_lt = new QLabel();
    arrow_lt->setPixmap(p_arrow_lt);
    r = arrow_lt->geometry();
    r.setWidth(p_arrow_lt.size().width());
    r.setHeight(p_arrow_lt.size().height());
    arrow_lt->setGeometry(r);
    arrow_lt->setVisible(false);
    arrows.push_back(arrow_lt);
    
    QPixmap p_arrow_lb("data/images/arrow-left-bottom.png");
    arrow_lb = new QLabel();
    arrow_lb->setPixmap(p_arrow_lb);
    r = arrow_lb->geometry();
    r.setWidth(p_arrow_lb.size().width());
    r.setHeight(p_arrow_lb.size().height());
    arrow_lb->setGeometry(r);
    arrow_lb->setVisible(false);
    arrows.push_back(arrow_lb);
    
    QPixmap p_arrow_tl("data/images/arrow-top-left.png");
    arrow_tl = new QLabel();
    arrow_tl->setPixmap(p_arrow_tl);
    r = arrow_tl->geometry();
    r.setWidth(p_arrow_tl.size().width());
    r.setHeight(p_arrow_tl.size().height());
    arrow_tl->setGeometry(r);
    arrow_tl->setVisible(false);
    arrows.push_back(arrow_tl);
    
    QPixmap p_arrow_tr("data/images/arrow-top-right.png");
    arrow_tr = new QLabel();
    arrow_tr->setPixmap(p_arrow_tr);
    r = arrow_tr->geometry();
    r.setWidth(p_arrow_tr.size().width());
    r.setHeight(p_arrow_tr.size().height());
    arrow_tr->setGeometry(r);
    arrow_tr->setVisible(false);
    arrows.push_back(arrow_tr);
    
    QPixmap p_arrow_rt("data/images/arrow-right-top.png");
    arrow_rt = new QLabel();
    arrow_rt->setPixmap(p_arrow_rt);
    r = arrow_rt->geometry();
    r.setWidth(p_arrow_rt.size().width());
    r.setHeight(p_arrow_rt.size().height());
    arrow_rt->setGeometry(r);
    arrow_rt->setVisible(false);
    arrows.push_back(arrow_rt);
    
    QPixmap p_arrow_rb("data/images/arrow-right-bottom.png");
    arrow_rb = new QLabel();
    arrow_rb->setPixmap(p_arrow_rb);
    r = arrow_rb->geometry();
    r.setWidth(p_arrow_rb.size().width());
    r.setHeight(p_arrow_rb.size().height());
    arrow_rb->setGeometry(r);
    arrow_rb->setVisible(false);
    arrows.push_back(arrow_rb);
    
    Ui_LipidSpaceGUI *ui = lipidSpaceGUI->ui;
    main_widgets = {ui->actionLoad_list_s, ui->actionLoad_table, ui->actionQuit, ui->actionRemove_all_lipidomes, ui->actionSet_transparency, ui->actionAutomatically, ui->action2_columns, ui->action3_columns, ui->action4_columns, ui->action5_columns, ui->actionShow_global_lipidome, ui->actionShow_dendrogram, ui->action1_column, ui->action6_columns, ui->actionAbout, ui->actionLog_messages, ui->actionShow_quantitative_information, ui->actionIgnoring_lipid_sn_positions, ui->actionManage_lipidomes, ui->actionIgnore_quantitative_information, ui->actionUnbound_lipid_distance_metric, ui->actionExport_Results, ui->actionSet_number_of_principal_components, ui->actionSelect_principal_components, ui->actionImport_data_table, ui->actionImport_pivot_table, ui->actionSingle_linkage_clustering, ui->actionComplete_linkage_clustering, ui->actionAverage_linkage_clustering, ui->actionImport_mzTabM, ui->actionTranslate, ui->itemsTabWidget, ui->speciesComboBox, ui->speciesList, ui->classComboBox, ui->classList, ui->categoryComboBox, ui->categoryList, ui->treeWidget, ui->sampleComboBox, ui->sampleList, ui->normalizationComboBox, ui->applyChangesPushButton, ui->firstTutorialPushButton, ui->dendrogramView, ui->featureComboBox, ui->pieTreeSpinBox, ui->dendrogramHeightSpinBox, ui->pieSizeSpinBox, ui->startAnalysisPushButton, ui->statistics, ui->featureComboBoxStat, ui->tickSizeSpinBox, ui->legendSizeSpinBox, ui->menubar, ui->menuLipidSpace, ui->menuAnalysis, ui->menuClustering_strategy, ui->menuView, ui->menuTile_layout, ui->menuHelp, ui->viewsTabWidget};
    
    connect(lipidSpaceGUI->ui->firstTutorialPushButton, &QPushButton::clicked, this, &Tutorial::start_first_tutorial);
    
}


Tutorial::~Tutorial() {
}



void Tutorial::x_clicked(){
    if (tutorialType == NoTutorial || (QMessageBox::question(this, "Quit tutorial", "Do you want to quit the tutorial?") == QMessageBox::Yes)){
        close_tutorial();
    }
}



void Tutorial::close_tutorial(){
    setVisible(false);
    tutorialType = NoTutorial;
    step = -1;
    
    for (auto obj : main_widgets){
        if (instanceof(obj, QWidget)){
            ((QWidget*)obj)->setEnabled(true);
        }
        else if (instanceof(obj, QAction)){
            ((QAction*)obj)->setEnabled(true);
        }
    }
}


QPoint Tutorial::map_widget(QWidget *widget){
    QPoint p = widget->mapToGlobal(QPoint(0, 0));
    return lipidSpaceGUI->ui->centralwidget->mapFromGlobal(p);
}



void Tutorial::show_arrow(Arrow a, QWidget *widget, QPoint p){
    show_arrow(a, widget, p.x(), p.y());
}


void Tutorial::show_arrow(Arrow a, QWidget *widget, int x, int y){
    QLabel* arrow = arrows[a];
    int offset = 26;
    switch(a){
        case ATL: y -= offset; break;
        case ATR: x -= arrow->pixmap().size().width(); y -= offset; break;
        case ABL: y -= arrow->pixmap().size().height() - offset; break;
        case ABR: x -= arrow->pixmap().size().width(); y -= arrow->pixmap().size().height() - offset; break;
        case ALT: x -= offset; break;
        case ALB: x -= offset; y -= arrow->pixmap().size().height(); break;
        case ART: x -= arrow->pixmap().size().width() - offset; break;
        case ARB: x -= arrow->pixmap().size().width() - offset; y -= arrow->pixmap().size().height(); break;
    }
    arrow->setParent(widget);
    arrow->setVisible(true);
    arrow->move(x, y);
}



bool Tutorial::can_start_tutorial(){
    if (lipidSpaceGUI->lipid_space->lipidomes.empty()) return true;
    if (QMessageBox::question(this, "Reset LipidSpace", "In order to start the tutorial, LipidSpace will be reset. Do you want to proceed?") == QMessageBox::Yes){
        lipidSpaceGUI->resetAnalysis();
        return true;
    }
    return false;
}



void Tutorial::start_first_tutorial(){
    if (!can_start_tutorial()) return;
    step = 0;
    tutorialType = FirstTutorial;
    first_tutorial_steps();
}


void Tutorial::continue_tutorial(){
    step++;
    switch(tutorialType){
        case FirstTutorial: first_tutorial_steps(); break;
        default: close_tutorial(); break;
    }
}


void Tutorial::disable(){
    for (auto obj : main_widgets){
        if (instanceof(obj, QWidget)){
            ((QWidget*)obj)->setEnabled(false);
        }
        else if (instanceof(obj, QAction)){
            ((QAction*)obj)->setEnabled(false);
        }
    }
}



void Tutorial::first_tutorial_steps(){
    disable();
    pagesLabel->setText((std::to_string(step + 1) + " / 3").c_str());
    setVisible(true);
    
    switch((FirstSteps)step){
        case FStart:
            move(20, 20);
            titleLabel->setText("First Tutorial");
            informationLabel->setText("Welcome to the first tutorial of LipidSpace. The tutorials are designed to interactively guide you through the interface of LipidSpace. ");
            break;
            
        
        case FDescription:
            move(20, 20);
            titleLabel->setText("What is LipidSpace");
            informationLabel->setText("LipidSpace is a tool to analyse a multitude of lipidomes, putting them together in one model, and offer several functions for a deeper and quicker investigation of your lipidomic data.");
            break;
            
            
        case FEnd:
            titleLabel->setText("Bla bla");
            informationLabel->setText("bla bla bla");
            break;
            
        default:
            close_tutorial();
            break;
    } 
    // QPoint p = map_widget(lipidSpaceGUI->ui->firstTutorialPushButton);
    // show_arrow(ARB, lipidSpaceGUI->ui->centralwidget,  p);
}
