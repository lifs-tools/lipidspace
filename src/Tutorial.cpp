#include "lipidspace/Tutorial.h"


const vector<FirstSteps> Tutorial::first_tutorial_steps_order{FStart, FDescription, FFindImport, FOpenImport, FEnteredImport, FExplainRow, FExplainColumn, FExplainFlat, FShowPreview, FSelectColumnTable, FExplainColumnField, FExplainLipidColumnField, FExplainSampleColumnField, FExplainStudyFields, FSampleEntryAssignment, FStudyVarAssignment, FLipidAssignment, FStudyVarMapping, FFinishImport, FFinish, FEnd};

const vector<SecondSteps> Tutorial::second_tutorial_steps_order{SStart, SLoadTable, SSeletionSection1, SSeletionSection2, SSorting, SSortingBars, SSortingPG, SNormalization, SGoToStudVarFilter, SFilterStudyVar, SLeftPanel, SLipidSpaces, SSpacesOptions, SSpacesSingleView, SSpacesSingleViewExplaination, SDendrogramClick, SDendrogram, SDendrogram2, SStatistics, SStatistics2, SRawClick, SRawTable, SFinish, SEnd};
//const vector<SecondSteps> Tutorial::second_tutorial_steps_order{SLoadTable, SDendrogram, SDendrogram2, SStatistics, SStatistics2, SRawClick, SRawTable, SFinish, SEnd};

const vector<ThirdSteps> Tutorial::third_tutorial_steps_order{TStart};

const vector<FourthSteps> Tutorial::fourth_tutorial_steps_order{DStart};

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

    QPixmap close_x(QCoreApplication::applicationDirPath() + "/data/images/close-x.png");
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

    QPixmap p_arrow_bl(QCoreApplication::applicationDirPath() + "/data/images/arrow-bottom-left.png");
    arrow_bl = new QLabel();
    arrow_bl->setPixmap(p_arrow_bl);
    r = arrow_bl->geometry();
    r.setWidth(p_arrow_bl.size().width());
    r.setHeight(p_arrow_bl.size().height());
    arrow_bl->setGeometry(r);
    arrow_bl->setVisible(false);
    arrows.push_back(arrow_bl);

    QPixmap p_arrow_br(QCoreApplication::applicationDirPath() + "/data/images/arrow-bottom-right.png");
    arrow_br = new QLabel();
    arrow_br->setPixmap(p_arrow_br);
    r = arrow_br->geometry();
    r.setWidth(p_arrow_br.size().width());
    r.setHeight(p_arrow_br.size().height());
    arrow_br->setGeometry(r);
    arrow_br->setVisible(false);
    arrows.push_back(arrow_br);

    QPixmap p_arrow_lt(QCoreApplication::applicationDirPath() + "/data/images/arrow-left-top.png");
    arrow_lt = new QLabel();
    arrow_lt->setPixmap(p_arrow_lt);
    r = arrow_lt->geometry();
    r.setWidth(p_arrow_lt.size().width());
    r.setHeight(p_arrow_lt.size().height());
    arrow_lt->setGeometry(r);
    arrow_lt->setVisible(false);
    arrows.push_back(arrow_lt);

    QPixmap p_arrow_lb(QCoreApplication::applicationDirPath() + "/data/images/arrow-left-bottom.png");
    arrow_lb = new QLabel();
    arrow_lb->setPixmap(p_arrow_lb);
    r = arrow_lb->geometry();
    r.setWidth(p_arrow_lb.size().width());
    r.setHeight(p_arrow_lb.size().height());
    arrow_lb->setGeometry(r);
    arrow_lb->setVisible(false);
    arrows.push_back(arrow_lb);

    QPixmap p_arrow_tl(QCoreApplication::applicationDirPath() + "/data/images/arrow-top-left.png");
    arrow_tl = new QLabel();
    arrow_tl->setPixmap(p_arrow_tl);
    r = arrow_tl->geometry();
    r.setWidth(p_arrow_tl.size().width());
    r.setHeight(p_arrow_tl.size().height());
    arrow_tl->setGeometry(r);
    arrow_tl->setVisible(false);
    arrows.push_back(arrow_tl);

    QPixmap p_arrow_tr(QCoreApplication::applicationDirPath() + "/data/images/arrow-top-right.png");
    arrow_tr = new QLabel();
    arrow_tr->setPixmap(p_arrow_tr);
    r = arrow_tr->geometry();
    r.setWidth(p_arrow_tr.size().width());
    r.setHeight(p_arrow_tr.size().height());
    arrow_tr->setGeometry(r);
    arrow_tr->setVisible(false);
    arrows.push_back(arrow_tr);

    QPixmap p_arrow_rt(QCoreApplication::applicationDirPath() + "/data/images/arrow-right-top.png");
    arrow_rt = new QLabel();
    arrow_rt->setPixmap(p_arrow_rt);
    r = arrow_rt->geometry();
    r.setWidth(p_arrow_rt.size().width());
    r.setHeight(p_arrow_rt.size().height());
    arrow_rt->setGeometry(r);
    arrow_rt->setVisible(false);
    arrows.push_back(arrow_rt);

    QPixmap p_arrow_rb(QCoreApplication::applicationDirPath() + "/data/images/arrow-right-bottom.png");
    arrow_rb = new QLabel();
    arrow_rb->setPixmap(p_arrow_rb);
    r = arrow_rb->geometry();
    r.setWidth(p_arrow_rb.size().width());
    r.setHeight(p_arrow_rb.size().height());
    arrow_rb->setGeometry(r);
    arrow_rb->setVisible(false);
    arrows.push_back(arrow_rb);

    Ui_LipidSpaceGUI *ui = lipidSpaceGUI->ui;
    Ui_ImportTable *ui_it = lipidSpaceGUI->import_table.ui;

    main_widgets = {{ui->actionLoad_list_s, false}, {ui->actionLoad_table, false}, {ui->actionQuit, false}, {ui->actionRemove_all_lipidomes, false}, {ui->actionSet_transparency, false}, {ui->actionAutomatically, false}, {ui->action2_columns, false}, {ui->action3_columns, false}, {ui->action4_columns, false}, {ui->action5_columns, false}, {ui->actionShow_global_lipidome, false}, {ui->actionShow_study_lipidomes, false}, {ui->action1_column, false}, {ui->action6_columns, false}, {ui->actionAbout, false}, {ui->actionLog_messages, false}, {ui->actionShow_quantitative_information, false}, {ui->actionIgnoring_lipid_sn_positions, false}, {ui->actionManage_lipidomes, false}, {ui->actionIgnore_quantitative_information, false}, {ui->actionUnbound_lipid_distance_metric, false}, {ui->actionExport_Results, false}, {ui->actionSet_number_of_principal_components, false}, {ui->actionSelect_principal_components, false}, {ui->actionImport_data_table, false}, {ui->actionImport_pivot_table, false}, {ui->actionSingle_linkage_clustering, false}, {ui->actionComplete_linkage_clustering, false}, {ui->actionAverage_linkage_clustering, false}, {ui->actionImport_mzTabM, false}, {ui->actionTranslate, false}, {ui->speciesComboBox, false}, {ui->speciesList, false}, {ui->classComboBox, false}, {ui->classList, false}, {ui->categoryComboBox, false}, {ui->categoryList, false}, {ui->treeWidget, false}, {ui->sampleComboBox, false}, {ui->sampleList, false}, {ui->normalizationComboBox, false}, {ui->applyChangesPushButton, false}, {ui->homeGraphicsView->firstTutorialPushButton, false}, {ui->homeGraphicsView->secondTutorialPushButton, false}, {ui->homeGraphicsView->thirdTutorialPushButton, false}, {ui->homeGraphicsView->fourthTutorialPushButton, false}, {ui->dendrogramView, false}, {ui->studyVariableComboBox, false}, {ui->pieTreeSpinBox, false}, {ui->dendrogramHeightSpinBox, false}, {ui->pieSizeSpinBox, false}, {ui->startAnalysisPushButton, false}, {ui->statisticsBoxPlot, false}, {ui->statisticsHistogram, false}, {ui->statisticsROCCurve, false}, {ui->statisticsSpeciesCV, false}, {ui->statisticsBarPlot, false}, {ui->studyVariableComboBoxStat, false}, {ui->tickSizeSpinBox, false}, {ui->legendSizeSpinBox, false}, {ui->barNumberSpinBox, false}, {ui->menubar, false}, {ui->menuLipidSpace, false}, {ui->menuAnalysis, false}, {ui->menuClustering_strategy, false}, {ui->menuView, false}, {ui->menuTile_layout, false}, {ui->menuHelp, false}, {ui->viewsTabWidget, false}, {ui_it->tabWidget, false}, {ui_it->label_15, false}, {ui_it->sampleListWidgetRow, false}, {ui_it->okButtonRow, false}, {ui_it->cancelButtonRow, false}, {ui_it->ignoreListWidgetRow, false}, {ui_it->lipidListWidgetRow, false}, {ui_it->sampleListWidgetCol, false}, {ui_it->cancelButtonCol, false}, {ui_it->lipidListWidgetCol, false}, {ui_it->ignoreListWidgetCol, false}, {ui_it->numericalStudyVariableListWidgetCol, false}, {ui_it->nominalStudyVariableListWidgetCol, false}, {ui_it->flatTab, false}, {ui_it->lipidListWidgetFlat, false}, {ui_it->ignoreListWidgetFlat, false}, {ui_it->quantListWidgetFlat, false}, {ui_it->okButtonFlat, false}, {ui_it->numericalStudyVariableListWidgetFlat, false}, {ui_it->cancelButtonFlat, false}, {ui_it->sampleListWidgetFlat, false}, {ui_it->nominalStudyVariableListWidgetFlat, false}, {ui_it->tableWidget, false}, {ui_it->checkBoxMappingFlat, false}, {ui->itemsTabWidget, false},{ui_it->checkBoxMappingCol, false}, {ui->tableView, false}, {ui->actionSelect_tiles, false}, {ui->labelSizeSpinBox, false}, {ui->secondaryComboBox, false}};


    // window resize event
    connect(lipidSpaceGUI, &LipidSpaceGUI::resizing, this, &Tutorial::resize);

    // tutorial starts
    connect(ui->homeGraphicsView->firstTutorialPushButton, &QPushButton::clicked, this, &Tutorial::start_first_tutorial);
    connect(ui->homeGraphicsView->secondTutorialPushButton, &QPushButton::clicked, this, &Tutorial::start_second_tutorial);

    connect(&lipidSpaceGUI->import_table, &ImportTable::finished, this, &Tutorial::close_tutorial);
    connect(&lipidSpaceGUI->import_table, &ImportTable::rejected, this, &Tutorial::close_directly_tutorial);

    // actions
    connect(&lipidSpaceGUI->import_table, &ImportTable::importOpened, this, &Tutorial::action_performed);
    connect(lipidSpaceGUI, &LipidSpaceGUI::analysisCompleted, this, &Tutorial::action_performed);

    // widgets
    connect(lipidSpaceGUI->import_table.ui->tabWidget, &QTabWidget::currentChanged, this, &Tutorial::tab_changed);
    connect(lipidSpaceGUI->ui->itemsTabWidget, &QTabWidget::currentChanged, this, &Tutorial::tab_changed);
    connect(lipidSpaceGUI->ui->viewsTabWidget, &QTabWidget::currentChanged, this, &Tutorial::tab_changed);
    connect(lipidSpaceGUI->import_table.ui->sampleListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(lipidSpaceGUI->import_table.ui->ignoreListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(lipidSpaceGUI->import_table.ui->nominalStudyVariableListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(lipidSpaceGUI->import_table.ui->lipidListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(ui->actionSelection_mode_activated, &QAction::triggered, this, &Tutorial::action_performed);
    connect(ui->studyVariableComboBoxStat, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &Tutorial::combobox_changed);
}


Tutorial::~Tutorial() {
}



void Tutorial::x_clicked(){
    if (tutorialType == NoTutorial || (QMessageBox::question(this, "Quit tutorial", "Do you want to quit the tutorial?") == QMessageBox::Yes)){
        close_tutorial(QDialog::Rejected);
    }
}



void Tutorial::close_directly_tutorial(){
    close_tutorial(QDialog::Rejected);
}



void Tutorial::close_tutorial(int state){
    if (state == QDialog::Accepted) return;

    setVisible(false);
    hide_arrows();
    tutorialType = NoTutorial;
    step = -1;
    GlobalData::in_tutorial = false;

    for (auto kv : main_widgets){
        if (instanceof(kv.first, QWidget)){
            ((QWidget*)kv.first)->setEnabled(kv.second);
        }
        else if (instanceof(kv.first, QAction)){
            ((QAction*)kv.first)->setEnabled(kv.second);
        }
    }
}


QPoint Tutorial::map_widget(QWidget *widget, QWidget *main){
    QPoint p = widget->mapToGlobal(QPoint(0, 0));
    return main->mapFromGlobal(p);
}



void Tutorial::show_arrow(Arrow a, QWidget *widget, QPoint p){
    show_arrow(a, widget, p.x(), p.y());
}


void Tutorial::show_arrow(Arrow a, QWidget *widget, int x, int y){
    QLabel* arrow = arrows[a];
    int offset = 26;
    switch(a){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        case ATL: y -= offset; break;
        case ATR: x -= arrow->pixmap().size().width(); y -= offset; break;
        case ABL: y -= arrow->pixmap().size().height() - offset; break;
        case ABR: x -= arrow->pixmap().size().width(); y -= arrow->pixmap().size().height() - offset; break;
        case ALT: x -= offset; break;
        case ALB: x -= offset; y -= arrow->pixmap().size().height(); break;
        case ART: x -= arrow->pixmap().size().width() - offset; break;
        case ARB: x -= arrow->pixmap().size().width() - offset; y -= arrow->pixmap().size().height(); break;
#else
        case ATL: y -= offset; break;
        case ATR: x -= arrow->pixmap()->size().width(); y -= offset; break;
        case ABL: y -= arrow->pixmap()->size().height() - offset; break;
        case ABR: x -= arrow->pixmap()->size().width(); y -= arrow->pixmap()->size().height() - offset; break;
        case ALT: x -= offset; break;
        case ALB: x -= offset; y -= arrow->pixmap()->size().height(); break;
        case ART: x -= arrow->pixmap()->size().width() - offset; break;
        case ARB: x -= arrow->pixmap()->size().width() - offset; y -= arrow->pixmap()->size().height(); break;
#endif
    }
    arrow->setParent(widget);
    arrow->move(x, y);
    arrow->setVisible(true);
}



bool Tutorial::can_start_tutorial(){
    if (lipidSpaceGUI->lipid_space->lipidomes.empty() || QMessageBox::question(this, "Reset LipidSpace", "In order to start the tutorial, LipidSpace will be reset. Do you want to proceed?") == QMessageBox::Yes){
        lipidSpaceGUI->resetAnalysis();

        // getting the enabled states of all UI widgets
        for (auto &kv : main_widgets){
            if (instanceof(kv.first, QWidget)){
                kv.second = ((QWidget*)kv.first)->isEnabled();
            }
            else if (instanceof(kv.first, QAction)){
                kv.second = ((QAction*)kv.first)->isEnabled();
            }
        }

        return true;
    }
    return false;
}



void Tutorial::start_first_tutorial(){
    if (!can_start_tutorial()) return;
    step = 0;
    tutorialType = FirstTutorial;
    first_tutorial_steps();
    GlobalData::in_tutorial = true;
}



void Tutorial::start_second_tutorial(){
    if (!can_start_tutorial()) return;
    step = 0;
    tutorialType = SecondTutorial;
    second_tutorial_steps();
    GlobalData::in_tutorial = true;

}



void Tutorial::resize(){
    if (tutorialType == NoTutorial) return;
    switch(tutorialType){
        case FirstTutorial: first_tutorial_steps(); break;
        case SecondTutorial: second_tutorial_steps(); break;
        case ThirdTutorial: third_tutorial_steps(); break;
        case FourthTutorial: fourth_tutorial_steps(); break;
        default: close_tutorial(0); break;
    }
}


void Tutorial::continue_tutorial(){
    step++;
    switch(tutorialType){
        case FirstTutorial: first_tutorial_steps(); break;
        case SecondTutorial: second_tutorial_steps(); break;
        case ThirdTutorial: third_tutorial_steps(); break;
        case FourthTutorial: fourth_tutorial_steps(); break;
        default: close_tutorial(0); break;
    }
}


void Tutorial::disable(){
    continuePushButton->setEnabled(false);
    hide_arrows();
    for (auto kv : main_widgets){
        if (instanceof(kv.first, QWidget)){
            ((QWidget*)kv.first)->setEnabled(false);
        }
        else if (instanceof(kv.first, QAction)){
            ((QAction*)kv.first)->setEnabled(false);
        }
    }
}


void Tutorial::hide_arrows(){
    for (auto arrow : arrows){
        arrow->setVisible(false);
    }
}



void Tutorial::tile_selection_changed(){
    if (step < 0 || tutorialType == NoTutorial) return;

    switch(tutorialType){
        case SecondTutorial: {
            if (second_tutorial_steps_order[step] == SSpacesOptions){
                int selected = 0;
                for (auto canvas : lipidSpaceGUI->canvases) selected += canvas->marked_for_selected_view;
                continuePushButton->setEnabled(selected == 2);
            }
        }

        default: break;
    }
}




void Tutorial::combobox_changed(int){
    if (step < 0 || tutorialType == NoTutorial) return;


    switch(tutorialType){
        case SecondTutorial: {
            SecondSteps s_step = second_tutorial_steps_order[step];
            switch(s_step){
                case SSorting:
                    if (lipidSpaceGUI->ui->speciesComboBox->currentText() == "Type classification (Desc)"){
                        continue_tutorial();
                    }
                    break;

                case SStatistics:
                    if (lipidSpaceGUI->ui->studyVariableComboBoxStat->currentText() == "Type"){
                        continue_tutorial();
                    }
                    break;

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
}




void Tutorial::action_performed(){
    if (step < 0 || tutorialType == NoTutorial) return;

    switch(tutorialType){
        case FirstTutorial: {
            FirstSteps f_step = first_tutorial_steps_order[step];

            switch(f_step){
                case FOpenImport:
                    {
                        ImportTable &it = lipidSpaceGUI->import_table;
                        QFileInfo qFileInfo(it.file_name);
                        string cleaned_file_name = qFileInfo.baseName().toStdString();
                        if (cleaned_file_name != "Example-Dataset" || it.sheet != "Data"){
                            QMessageBox::warning(this, "Wrong table", "You have selected the wrong file, please select the correct file according to the tutorial.");
                            it.accept();
                        }
                        else {
                            continue_tutorial();
                        }
                    }
                    break;

                case FFinishImport:
                    continue_tutorial();
                    break;

                default:
                    break;
            }

        } break;

        case SecondTutorial: {
            SecondSteps s_step = second_tutorial_steps_order[step];

            switch(s_step){
                case SSpacesSingleView:
                    continue_tutorial();
                    break;

                default:
                    break;
            }
        } break;

        default:
            break;
    }
}


void Tutorial::tab_changed(int index){
    if (step < 0 || tutorialType == NoTutorial) return;

    switch(tutorialType){
        case FirstTutorial: {
            FirstSteps f_step = first_tutorial_steps_order[step];

            switch(f_step){
                case FSelectColumnTable:
                    if (index == 1) continue_tutorial();
                    break;

                case FSampleEntryAssignment:
                case FStudyVarAssignment:
                case FLipidAssignment:
                    if (index != 1) lipidSpaceGUI->import_table.ui->tabWidget->setCurrentIndex(1);

                default:
                    break;
            }

        } break;

        case SecondTutorial: {
            SecondSteps s_step = second_tutorial_steps_order[step];

            switch(s_step){
                case SSorting:
                    if (lipidSpaceGUI->ui->itemsTabWidget->currentIndex() != 0){
                        lipidSpaceGUI->ui->itemsTabWidget->setCurrentIndex(0);
                    }
                    break;

                case SGoToStudVarFilter:
                    if (lipidSpaceGUI->ui->itemsTabWidget->currentIndex() == 3){
                        continue_tutorial();
                    }
                    break;

                case SLeftPanel:
                    if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() == 1){
                        continue_tutorial();
                    }
                    else if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() != 1){
                        lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(2);
                    }
                    break;

                case SLipidSpaces:
                    if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() != 1){
                        lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(1);
                    }
                    break;

                case SSpacesOptions:
                    if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() != 1){
                        lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(1);
                    }
                    break;

                case SDendrogramClick:
                    if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() == 2){
                        continue_tutorial();
                    }
                    else if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() != 2){
                        lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(1);
                    }
                    break;

                case SDendrogram:
                    lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(2);
                    break;

                case SDendrogram2:
                    if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() == 3){
                        continue_tutorial();
                    }
                    else if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() != 3){
                        lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(2);
                    }
                    break;

                case SStatistics:
                    lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(3);
                    break;

                case SRawClick:
                    if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() == 4){
                        continue_tutorial();
                    }
                    else if (lipidSpaceGUI->ui->viewsTabWidget->currentIndex() != 4){
                        lipidSpaceGUI->ui->viewsTabWidget->setCurrentIndex(3);
                    }
                    break;

                default:
                    break;
            }


        } break;

        default:
            break;
    }
}


void Tutorial::item_changed(const QModelIndex &, int, int){
    if (step < 0 || tutorialType == NoTutorial) return;

    switch(tutorialType){
        case FirstTutorial: {
            FirstSteps f_step = first_tutorial_steps_order[step];

            switch(f_step){
                case FSampleEntryAssignment:
                    {
                        QListWidget *list = lipidSpaceGUI->import_table.ui->sampleListWidgetCol;
                        list->update();
                        continuePushButton->setEnabled(list->count() == 1 && list->item(0)->text().toStdString() == "Species");
                    }
                    break;

                case FStudyVarAssignment: {
                        set<string> study_variable_set = {"Type", "Treatment", "Type-Treatment"};
                        QListWidget *list = lipidSpaceGUI->import_table.ui->nominalStudyVariableListWidgetCol;
                        for (int i = 0; i < list->count(); ++i){
                            string s = list->item(i)->text().toStdString();
                            if (uncontains_val(study_variable_set, s)) break;
                            study_variable_set.erase(s);
                        }
                        continuePushButton->setEnabled(list->count() == 3 && study_variable_set.empty());
                    }
                    break;

                case FLipidAssignment:
                    continuePushButton->setEnabled(lipidSpaceGUI->import_table.ui->ignoreListWidgetCol->count() == 0);
                    break;

                default:
                    break;
            }

        } break;

        case SecondTutorial: {

        } break;

        default:
            break;
    }
}


void Tutorial::move(int x, int y, QWidget *w){
    if (w) setParent(w);
    QFrame::move(x, y);
    setVisible(true);
}



void Tutorial::changeSize(int w, int h){
    continuePushButton->move(w - 105, 50);
    pagesLabel->move(w - 126, h - 31);
    xLabel->move(w - 25, 10);
    informationLabel->setGeometry(QRect(20, 90, w - 40, h - 120));
    QPoint p = pos();
    setGeometry(p.x(), p.y(), w, h);
}



void Tutorial::first_tutorial_steps(){
    disable();
    pagesLabel->setText((std::to_string(step + 1) + " / "  + std::to_string(first_tutorial_steps_order.size() - 1)).c_str());
    setVisible(true);
    titleLabel->setText("");
    informationLabel->setText("");
    QFontMetrics font_metrics(QApplication::font());

    FirstSteps f_step = first_tutorial_steps_order[step];

    switch(f_step){
        case FStart:
            changeSize(650, 250);
            move(20, 20);
            titleLabel->setText("First Tutorial - Data Import");
            continuePushButton->setEnabled(true);
            informationLabel->setText("Welcome to the first tutorial of LipidSpace. The tutorials are designed to interactively guide you through the actual interface of LipidSpace. In this tutorial, we will go through the ways to import your lipidomics data into LipidSpace.");
            break;


        case FDescription:
            move(20, 20);
            titleLabel->setText("What is LipidSpace");
            continuePushButton->setEnabled(true);
            informationLabel->setText("LipidSpace is a tool to analyse a multitude of lipidomes, putting them together in one model, and offer several functions for a deeper and quicker investigation of your lipidomic data. Usually, data of indentified and quantified lipidomics analyses are structured as a table of size (# of lipids) x (# of samples).");
            break;


        case FFindImport: {
                move(200, 100);
                show_arrow(ALT, lipidSpaceGUI->ui->centralwidget, lipidSpaceGUI->ui->menubar->actionGeometry(lipidSpaceGUI->ui->menuLipidSpace->menuAction()).width() / 2.0, 0);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Where to import Data?");
                informationLabel->setText("LipidSpace supports three types of input data, pure csv lists containing lipid names (one per line), as an already mentioned table, or formatted in the mzTab format.");
            }
            break;

        case FOpenImport:
            titleLabel->setText("Open Import Dialog");
            informationLabel->setText("Please click on File > Import table. Browse to the directory where LipidSpace is stored, go into the folder 'examples', select 'Example-Dataset.xlsx', and select in the following dialog the work sheet 'Data'. Be aware, that LipidSpace supports xlsx and csv files, but not the deprecated xls format.");
            lipidSpaceGUI->ui->menubar->setEnabled(true);
            lipidSpaceGUI->ui->menuLipidSpace->setEnabled(true);
            lipidSpaceGUI->ui->actionLoad_table->setEnabled(true);
            break;



        case FEnteredImport:
            move(20, 20, &(lipidSpaceGUI->import_table));
            titleLabel->setText("Import Dialog");
            informationLabel->setText("Here we see the import dialog for tables. Tables can be structured in multiple ways. The three most common structures are lipid row-based, lipid column-based and flat-based.");
            continuePushButton->setEnabled(true);
            break;



        case FExplainRow: {
                move(20, 300);
                QTabWidget *tab_widget = lipidSpaceGUI->import_table.ui->tabWidget;
                QRect r = tab_widget->tabBar()->tabRect(0);
                QPoint p = map_widget(tab_widget, &lipidSpaceGUI->import_table);
                show_arrow(ALT, &lipidSpaceGUI->import_table, p.x() + r.width() / 2.0, p.y() + r.height());
                titleLabel->setText("Lipid Row-Based Tables");
                informationLabel->setText("The first type of table structures is called lipid row table. The abundences over all samples for one specific lipid are structured row-wise. Usually, all lipids are written in one column. This structure requires that all sample names are denoted in the top row.");
                continuePushButton->setEnabled(true);
            }
            break;



        case FExplainColumn: {
                move(20, 300);
                QTabWidget *tab_widget = lipidSpaceGUI->import_table.ui->tabWidget;
                QRect r = tab_widget->tabBar()->tabRect(0);
                QRect r2 = tab_widget->tabBar()->tabRect(1);
                QPoint p = map_widget(tab_widget, &lipidSpaceGUI->import_table);
                show_arrow(ALT, &lipidSpaceGUI->import_table, p.x() + r.width() + r2.width() / 2.0, p.y() + r.height());
                titleLabel->setText("Lipid Column-Based Tables");
                informationLabel->setText("The second type of table structures is called lipid column table. The abundences over all samples for one specific lipid are structured column-wise. This structure requires that all lipid names are denoted in the top row. Additionally, specific study variables assigned to the samples such as age, condition, treatment, etc may be present column-wise.");
                continuePushButton->setEnabled(true);
            }
            break;



        case FExplainFlat: {
                move(20, 300);
                QTabWidget *tab_widget = lipidSpaceGUI->import_table.ui->tabWidget;
                QRect r = tab_widget->tabBar()->tabRect(0);
                QRect r2 = tab_widget->tabBar()->tabRect(1);
                QRect r3 = tab_widget->tabBar()->tabRect(2);
                QPoint p = map_widget(tab_widget, &lipidSpaceGUI->import_table);
                show_arrow(ART, &lipidSpaceGUI->import_table, p.x() + r.width() + r2.width() + r3.width() / 2.0, p.y() + r.height());
                titleLabel->setText("Flat Tables");
                informationLabel->setText("The third type of table structures is called flat table. Here, the lipid names, the sample names, and their according abundence are written in separate columns. Typically, in this table many additional columns with additional information (e.g., study variables) are stored.");
                continuePushButton->setEnabled(true);
            }
            break;



        case FShowPreview: {
                move(20, 20);
                QTableWidget *table_widget = lipidSpaceGUI->import_table.ui->tableWidget;
                QPoint p = map_widget(table_widget, &lipidSpaceGUI->import_table);
                show_arrow(ARB, &lipidSpaceGUI->import_table, p.x() + table_widget->width() / 2.0, p.y());
                titleLabel->setText("Table Preview");
                informationLabel->setText("Here you have a preview of our opened example table to decide, of which structure type your table is.");
                continuePushButton->setEnabled(true);
            }
            break;


        case FSelectColumnTable: {
                move(20, 300);
                changeSize(650, 180);
                QTabWidget *tab_widget = lipidSpaceGUI->import_table.ui->tabWidget;
                QRect r = tab_widget->tabBar()->tabRect(0);
                QRect r2 = tab_widget->tabBar()->tabRect(1);
                QPoint p = map_widget(tab_widget, &lipidSpaceGUI->import_table);
                show_arrow(ALT, &lipidSpaceGUI->import_table, p.x() + r.width() + r2.width() / 2.0, p.y() + r.height());
                titleLabel->setText("Select Column-Based Table for Import");
                informationLabel->setText("Since the example table is column-based, please click at the 'Lipid column (pivot) table' tab.");
                lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
                lipidSpaceGUI->import_table.ui->columnTab->setEnabled(true);
            }
            break;


        case FExplainColumnField: {
                move(20, 400);
                changeSize(650, 220);
                QWidget *widget = lipidSpaceGUI->import_table.ui->ignoreListWidgetCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ABL, &lipidSpaceGUI->import_table, p.x() + widget->width(), p.y() + widget->height() / 2.0);
                titleLabel->setText("The Input Columns Field");
                informationLabel->setText("To let LipidSpace know which information is stored in which column, you must assign the columns from your table to the specific content fields. On the left, you can see a field with all columns within your table.");
                continuePushButton->setEnabled(true);
            }
            break;


        case FExplainLipidColumnField: {
                move(20, 400);
                changeSize(650, 250);
                QWidget *widget = lipidSpaceGUI->import_table.ui->lipidListWidgetCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ABR, &lipidSpaceGUI->import_table, p.x(), p.y() + widget->height() / 2.0);
                titleLabel->setText("The Lipid Column Field");
                informationLabel->setText("In the middle, you have the first content field called 'Lipid columns'. Please notice that the asterisk (*) denotes a mandatory field. Here you have to drag'n'drop the columns from the input field that contain lipid names and their column-wise stored data. Multiple columns can be assigned here.");
                lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
                lipidSpaceGUI->import_table.ui->columnTab->setEnabled(true);
                continuePushButton->setEnabled(true);
            }
            break;


        case FExplainSampleColumnField: {
                move(20, 400);
                changeSize(650, 220);
                QWidget *widget = lipidSpaceGUI->import_table.ui->sampleListWidgetCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ATR, &lipidSpaceGUI->import_table, p.x(), p.y() + widget->height() / 2.0);
                titleLabel->setText("The Sample Column Field");
                informationLabel->setText("The second mandatory field is the sample column field. Here one has to assign the input column with all distinct sample names. Only one input column can be assigned here.");
                continuePushButton->setEnabled(true);
            }
            break;


        case FExplainStudyFields: {
                move(20, 400);
                QWidget *widget = lipidSpaceGUI->import_table.ui->numericalStudyVariableListWidgetCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ARB, &lipidSpaceGUI->import_table, p.x() + widget->width() * 0.8, p.y());
                widget = lipidSpaceGUI->import_table.ui->nominalStudyVariableListWidgetCol;
                p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ABR, &lipidSpaceGUI->import_table, p.x(), p.y() + widget->height() / 2.0);
                titleLabel->setText("Study Variables");
                informationLabel->setText("Additionally, if your table contains columns with study variable data, these colums can be assigned, either of numerical type (age, BMI) or of nominal / categorical type (condition, treatment). These fields can be optionally assigned.");
                continuePushButton->setEnabled(true);
            }
            break;


        case FSampleEntryAssignment:
            changeSize(650, 220);
            move(20, 450);
            titleLabel->setText("Assign Sample Column");
            informationLabel->setText("It's your time, please assign the 'Species' entry from the 'Input columns' field to the 'Sample column' field.");
            lipidSpaceGUI->import_table.ui->ignoreListWidgetCol->setEnabled(true);
            lipidSpaceGUI->import_table.ui->sampleListWidgetCol->setEnabled(true);
            lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
            break;


        case FStudyVarAssignment:
            move(20, 450);
            titleLabel->setText("Assign Study Variables");
            informationLabel->setText("Great, please assign as next the optional study variables, that is 'Type', 'Treatment', and 'Type-Treatment'.");
            lipidSpaceGUI->import_table.ui->ignoreListWidgetCol->setEnabled(true);
            lipidSpaceGUI->import_table.ui->nominalStudyVariableListWidgetCol->setEnabled(true);
            lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
            break;


        case FLipidAssignment:
            move(20, 450);
            titleLabel->setText("Assign Lipid Names");
            informationLabel->setText("Awesome, now only column remained in the input field containg lipid names. Please drag them all into the 'Lipid column' field. You can select all entries by using the [CTRL] + A key combination.");
            lipidSpaceGUI->import_table.ui->ignoreListWidgetCol->setEnabled(true);
            lipidSpaceGUI->import_table.ui->lipidListWidgetCol->setEnabled(true);
            lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
            break;


        case FStudyVarMapping: {
                move(20, 20);
                changeSize(650, 260);
                QWidget *widget = lipidSpaceGUI->import_table.ui->checkBoxMappingCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ATR, &lipidSpaceGUI->import_table, p.x(), p.y() + widget->height() / 2.);
                titleLabel->setText("Mapping of Study Variables");
                informationLabel->setText("You have the possibility to load multiple tables into LipidSpace at once. However, it is probable that study variables may have different names or values in different tables. In that case, you can activate the 'Mapping of study variables' function where you can map the names and values of study variables to already registered ones.");
                continuePushButton->setEnabled(true);
            }
            break;


        case FFinishImport:
            {
                changeSize(650, 180);
                move(20, 20);
                QWidget *widget = lipidSpaceGUI->import_table.ui->okButtonCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ART, &lipidSpaceGUI->import_table, p.x() + widget->width() / 2., p.y() + widget->height());
                titleLabel->setText("Finish Import");
                informationLabel->setText("To finish now the import, simply press the 'Ok' button.");
                widget->setEnabled(true);
                lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
            }
            break;


        case FFinish:
            changeSize(650, 180);
            move(20, 20, lipidSpaceGUI->ui->centralwidget);
            titleLabel->setText("First Tutorial Completed");
            continuePushButton->setEnabled(true);
            informationLabel->setText("Congratulations, you managed to import a first lipid data set into LipidSpace. Feel free to play around with the data set and explore LipidSpace or just start the second tutorial.");
            break;


        case FEnd:
        default:
            close_tutorial(0);
            break;
    }
}



void Tutorial::second_tutorial_steps(){
    disable();
    pagesLabel->setText((std::to_string(step + 1) + " / "  + std::to_string(second_tutorial_steps_order.size() - 1)).c_str());
    setVisible(true);
    titleLabel->setText("");
    informationLabel->setText("");
    QFontMetrics font_metrics(QApplication::font());
    SecondSteps s_step = second_tutorial_steps_order[step];

    switch(s_step){
        case SStart:
            changeSize(650, 220);
            move(20, 20);
            titleLabel->setText("Second Tutorial - UI Introduction");
            continuePushButton->setEnabled(true);
            informationLabel->setText("Welcome to the second tutorial of LipidSpace. In the previous tutorial, we've seen how to import datasets. Now we will discover the actual functions and windows / views of LipidSpace. To do so, the example dataset will be automatically loaded now.");
            break;


        case SLoadTable:
            {
                changeSize(650, 220);
                move(lipidSpaceGUI->width() - width() - 20, 20);
                if (lipidSpaceGUI->lipid_space->lipidomes.empty()){
                    lipidSpaceGUI->resetAnalysis();
                    vector<TableColumnType> *ct = new vector<TableColumnType>(369, LipidColumn);
                    ct->at(0) = SampleColumn;
                    ct->at(1) = StudyVariableColumnNominal;
                    ct->at(2) = StudyVariableColumnNominal;
                    ct->at(3) = StudyVariableColumnNominal;
                    string path_to_example = QCoreApplication::applicationDirPath().toStdString();
                    lipidSpaceGUI->loadTable(new ImportData(path_to_example + "/examples/Example-Dataset.xlsx", "Data", COLUMN_PIVOT_TABLE, ct));

                    main_widgets[lipidSpaceGUI->ui->menuAnalysis] = true;
                    main_widgets[lipidSpaceGUI->ui->menuView] = true;
                    main_widgets[lipidSpaceGUI->ui->actionExport_Results] = true;
                    main_widgets[lipidSpaceGUI->ui->menuClustering_strategy] = true;
                    main_widgets[lipidSpaceGUI->ui->menuTile_layout] = true;
                    main_widgets[lipidSpaceGUI->ui->menuSelected_tiles_mode] = true;

                    for (auto canvas : lipidSpaceGUI->canvases) connect(canvas, &Canvas::tileSelected, this, &Tutorial::tile_selection_changed);
                }
                continuePushButton->setEnabled(true);
                titleLabel->setText("UI Sections of LipidSpace");
                informationLabel->setText("Here we go. As you can see now, LipidSpace is split after data import into two major sections. On the left hand side, you find the selection section and on the right hand side the visualization section.");
                break;
            }
            break;


        case SSeletionSection1:
            {
                QTimer::singleShot(100, [this]() {
                    QWidget *widget = lipidSpaceGUI->ui->itemsTabWidget;
                    QPoint p = map_widget(widget, lipidSpaceGUI);
                    show_arrow(ATL, lipidSpaceGUI, p.x() + widget->width(), p.y() + widget->height() / 2.);
                });

                changeSize(650, 200);
                move(lipidSpaceGUI->width() - width() - 20, 20);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Selection Section");
                informationLabel->setText("Here we have the selection section. It contains five tabs, one for all imported lipid species, lipid classes, lipid categories, study varibles, and samples. In each tab, you can find a selection list of the respective entity.");
            }
            break;


        case SSeletionSection2:
            {
                changeSize(650, 250);
                QWidget *widget = lipidSpaceGUI->ui->applyChangesPushButton;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ALB, lipidSpaceGUI, p.x() + widget->width() / 2., p.y());
                move(lipidSpaceGUI->width() - width() - 20, 20);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Rerun Analysis");
                informationLabel->setText(QString("By default, all entries in each selection list are selected at the beginning. If you want to exclude certain lipid species, lipid classes, lipid categories, or even complete samples for a reanalysis of your data, you simple have to deselect the respective entries and click the '%1' button.").arg(((QPushButton*)widget)->text()));
            }
            break;


        case SSorting:
            {
                connect(lipidSpaceGUI->ui->speciesComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &Tutorial::combobox_changed);
                move(lipidSpaceGUI->width() - width() - 20, 20);
                QWidget *widget = lipidSpaceGUI->ui->speciesComboBox;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ATL, lipidSpaceGUI, p.x() + widget->width(), p.y() + widget->height() / 2.);
                lipidSpaceGUI->ui->itemsTabWidget->setEnabled(true);
                widget->setEnabled(true);
                titleLabel->setText("Selection Sorting I");
                informationLabel->setText("All tabs (except the 'Study Variables' tab) contain a possibility to alternate the sorting of the entities (alphabetically by default). For the 'Species' tab, additional sorting possibilities dependent on the number of registered study variables show up. Please select the 'Type classification (Desc)' sorting.");
            }
            break;


        case SSortingBars:
            {
                move(lipidSpaceGUI->width() - width() - 20, 20);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Selection Sorting II");
                informationLabel->setText("The lipid species are now sorted according to their accuracy to correctly separate the values (here wildtype and knockout) the respective study variable 'Type'. Be aware, that this is not an accuracy on multiple lipid species' but on each species individually. The grey bars are indicating the accuracy from 0 to 1 (perfect separation behavior).");
            }
            break;


        case SSortingPG:
            {
                changeSize(650, 220);
                move(lipidSpaceGUI->width() - width() - 20, 20);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Selection Sorting III");
                informationLabel->setText("Here, the lipid species 'PG 18:1_18:2' is on top with a grey bar going to the right edge. This means, that for all imported samples a prediction model can be found separating the 'Type' study variable perfectly with respect to the provided lipid quant value (e.g., concentrations).");
            }
            break;


        case SNormalization:
            {
                move(lipidSpaceGUI->width() - width() - 20, 20);
                changeSize(650, 250);
                QWidget *widget = lipidSpaceGUI->ui->normalizationComboBox;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ALB, lipidSpaceGUI, p.x() + widget->width() / 2., p.y());
                continuePushButton->setEnabled(true);
                titleLabel->setText("Different Normalizations");
                informationLabel->setText("LipidSpace supports several types of lipid intensity normalizations. Absolute normalization (aka no normalization) is set by default. Relative normalization normalizes all lipidomes against each other. When nominal study variables are introduced, the user can select study based normalizations. That means, that all lipidomes assigned to the same variable remain in the same relation to each other.");
            }
            break;

        case SGoToStudVarFilter:
            {
                move(lipidSpaceGUI->width() - width() - 20, 20);
                changeSize(650, 180);
                QWidget *widget = lipidSpaceGUI->ui->speciesComboBox;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ALT, lipidSpaceGUI, p.x() + widget->width() / 2., p.y());
                lipidSpaceGUI->ui->itemsTabWidget->setEnabled(true);
                titleLabel->setText("Filtering Study Variables I");
                informationLabel->setText("If you want (temporarily) filter with respect to your study variables, please click on the 'Study Variables' tab.");
            }
            break;


        case SFilterStudyVar:
            {
                move(lipidSpaceGUI->width() - width() - 20, 20);
                changeSize(650, 180);
                QWidget *widget = lipidSpaceGUI->ui->itemsTabWidget;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ATL, lipidSpaceGUI, p.x() + widget->width(), p.y() + widget->height() / 2.);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Filtering Study Variables II");
                informationLabel->setText("Here, one can either (de)select certain values for nominal study variables or set comparable filters for numerical study variables (less than x, between y, etc). To apply these filters, the analysis must be rerun.");
            }
            break;


        case SLeftPanel:
            {
                changeSize(650, 220);
                move(20, lipidSpaceGUI->height() - height() - 80);
                QWidget *widget = lipidSpaceGUI->ui->homeGraphicsView;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                int x = lipidSpaceGUI->ui->viewsTabWidget->mapTo(lipidSpaceGUI->ui->centralwidget, QPoint(lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(1).x(), 0)).x();
                show_arrow(ALT, lipidSpaceGUI, x + lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(1).width() / 2., p.y());
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                titleLabel->setText("Results Section");
                informationLabel->setText("On the right hand side you can see the results section. It contains and tabs (plus the 'Home' tab) showing different aspects of your data. You can choose between a view on all individual lipidomes, a dendrogram putting all lipidomes into relation, a statistics module and your imported raw data. Please select the 'Lipidomes' tab.");
            }
            break;


        case SLipidSpaces:
            {
                changeSize(400, 320);
                move(20, lipidSpaceGUI->height() - height() - 80);
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                continuePushButton->setEnabled(true);
                titleLabel->setText("The Lipid Spaces");
                informationLabel->setText("The lipid spaces are a represention of the respective lipidomes. Lipids of the same lipid class have the same bubble color. The size of the bubbles indicates the lipids quantity. For each lipidome, you have an interactive lipid space tile plus a global tile containing all lipid species (with averaged lipid values) within this analysis. All views are connected to each other. You can try move the views or zoom in / out. Afterwards, please continue.");
            }
            break;


        case SSpacesOptions:
            {
                changeSize(650, 250);
                move(20, lipidSpaceGUI->height() - height() - 80);
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                titleLabel->setText("Additional Options I");
                informationLabel->setText("Additional options for the lipid space tiles are available. When holding the [CTRL] key, you may swap the tiles. On right click, a context menu is opening with options like (de)selecting these lipid species where the mouse cursor hovers above. On double click, the tile gets marked for single view mode. Please mark two tiles.");
                tile_selection_changed();
            }
            break;




        case SSpacesSingleView:
            {
                changeSize(650, 220);
                move(20, lipidSpaceGUI->height() - height() - 80);

                show_arrow(ATR, lipidSpaceGUI, lipidSpaceGUI->ui->menubar->actionGeometry(lipidSpaceGUI->ui->menuView->menuAction()).x(), lipidSpaceGUI->ui->menubar->height() / 2.);

                lipidSpaceGUI->ui->menubar->setEnabled(true);
                lipidSpaceGUI->ui->menuView->setEnabled(true);
                lipidSpaceGUI->ui->menuSelected_tiles_mode->setEnabled(true);
                lipidSpaceGUI->ui->actionSelection_mode_activated->setEnabled(true);

                titleLabel->setText("Additional Options II");
                informationLabel->setText("When clicking on View -> Selected tile(s) mode -> Activated you have the possibility to switch between displaying all tiles and only the selected ones. Be aware that when your dataset contains more than 100 lipidomes, the selected tile mode will be automatically. Please activate the selected tile mode now.");

            }
            break;




        case SSpacesSingleViewExplaination:
            {
                changeSize(650, 220);
                move(20, lipidSpaceGUI->height() - height() - 80);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Single view mode");
                informationLabel->setText("Especially when you are interested only in one ore a subset of lipidomes, you can activate that mode. Now, all other tiles vanish from the grid. If you want to add / remove tiles from this mode while it is activated, you can do so by opening a selection window with View -> Selected tile(s) mode -> Select tyle(s).");
            }
            break;


        case SDendrogramClick:
            {
                changeSize(600, 180);
                move(20, lipidSpaceGUI->height() - height() - 80);
                QWidget *widget = lipidSpaceGUI->ui->homeGraphicsView;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                int x = lipidSpaceGUI->ui->viewsTabWidget->mapTo(lipidSpaceGUI->ui->centralwidget, QPoint(lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(2).x(), 0)).x();
                show_arrow(ALT, lipidSpaceGUI, x + lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(2).width() / 2., p.y());
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                titleLabel->setText("Click on 'Dendrogram' tab");
                informationLabel->setText("Please switch now to the dendrogram view clicking on the 'Dendrogram' tab.");
            }
            break;


        case SDendrogram:
            {
                changeSize(650, 220);
                move(20, 40);
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                continuePushButton->setEnabled(true);
                lipidSpaceGUI->ui->dendrogramView->setEnabled(true);
                titleLabel->setText("Dendrogram View I");
                informationLabel->setText("The dendrogram view summarizes the structural distances of all lipidomes based on their lipid space in one figure (which is the very heart of LipidSpace). It is also interactive, one can zoom in / out or (de)mark (sub)branches of the tree. Please try it out and continue the tutorial.");
            }
            break;


        case SDendrogram2:
            {
                changeSize(650, 220);
                move(20, lipidSpaceGUI->height() - height() - 80);
                QWidget *widget = lipidSpaceGUI->ui->homeGraphicsView;
                QPoint p = map_widget(widget, lipidSpaceGUI);

                int x = lipidSpaceGUI->ui->viewsTabWidget->mapTo(lipidSpaceGUI->ui->centralwidget, QPoint(lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(3).x(), 0)).x();
                show_arrow(ALT, lipidSpaceGUI, x + lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(3).width() / 2., p.y());

                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                titleLabel->setText("Dendrogram View II");
                informationLabel->setText("On right click, a context menu allows you to take a pdf of the current dendrogram view. When right clicking over a dendrogram edge (branch), you can perform a subselection of only the lipidomes which are leaves of this branch. To continue, please click now on the 'Statistics' tab.");
            }
            break;




        case SStatistics:
            {
                changeSize(650, 220);
                move(20, 40);
                QWidget *widget = lipidSpaceGUI->ui->studyVariableComboBoxStat;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ALB, lipidSpaceGUI, p.x() + widget->width() / 4., p.y());
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                widget->setEnabled(true);
                titleLabel->setText("Statistics view I");
                informationLabel->setText("The statistics module contains interactive statistics figures comprising several aspects of your data. For instance, you can adjust the views by changing study variable. Please select 'Type' study variable in the combo box.");
            }
            break;




        case SStatistics2:
            {
                changeSize(650, 220);
                move(20, lipidSpaceGUI->height() - height() - 80);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Statistics view II");
                informationLabel->setText("In the 'Quantitative data' box, you can select the source of your quant data for the figures. With a right click, you can activate additional options, such as showing data points on the box or bar graphs or export either the data of which the respective figure was drawn or the figure itself as pdf.");
            }
            break;




        case SRawClick:
            {
                changeSize(600, 180);
                move(20, lipidSpaceGUI->height() - height() - 80);
                QWidget *widget = lipidSpaceGUI->ui->homeGraphicsView;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                int x = lipidSpaceGUI->ui->viewsTabWidget->mapTo(lipidSpaceGUI->ui->centralwidget, QPoint(lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(4).x(), 0)).x();
                show_arrow(ALT, lipidSpaceGUI, x + lipidSpaceGUI->ui->viewsTabWidget->tabBar()->tabRect(4).width() / 2., p.y());
                lipidSpaceGUI->ui->viewsTabWidget->setEnabled(true);
                titleLabel->setText("Click on 'Raw data table' tab");
                informationLabel->setText("For the review of the last module, please click on the 'Raw data table' tab.");
            }
            break;




        case SRawTable:
            {
                changeSize(600, 220);
                move(20, lipidSpaceGUI->height() - height() - 80);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Raw Data Table");
                informationLabel->setText("The raw data table comprises all imported data, that is the lipid species, lipidomes, their quantitative data and all study variables. The values in the table do NOT alter when picking a different normalization mode. The table can be transposed or export on right click when desired.");
            }
            break;


        case SFinish:
            {
                move(20, 20, lipidSpaceGUI->ui->centralwidget);
                changeSize(650, 180);
                titleLabel->setText("Second Tutorial Completed");
                continuePushButton->setEnabled(true);
                informationLabel->setText("Congratulations, you should be able now to operate on LipidSpace. Try to load your own data in LipidSpace and explore it or just start the third tutorial.");
            }
            break;



        case SEnd:
        default:
            close_tutorial(0);
            break;
    }
}





void Tutorial::third_tutorial_steps(){
    disable();
    pagesLabel->setText((std::to_string(step + 1) + " / "  + std::to_string(third_tutorial_steps_order.size() - 1)).c_str());
    setVisible(true);
    titleLabel->setText("");
    informationLabel->setText("");
    QFontMetrics font_metrics(QApplication::font());
    ThirdSteps t_step = third_tutorial_steps_order[step];

    switch(t_step){
        case TStart:
        case TEnd:
        default:
            close_tutorial(0);
            break;
    }
}





void Tutorial::fourth_tutorial_steps(){
    disable();
    pagesLabel->setText((std::to_string(step + 1) + " / "  + std::to_string(fourth_tutorial_steps_order.size() - 1)).c_str());
    setVisible(true);
    titleLabel->setText("");
    informationLabel->setText("");
    QFontMetrics font_metrics(QApplication::font());
    FourthSteps d_step = fourth_tutorial_steps_order[step];

    switch(d_step){
        case DStart:
        case DEnd:
        default:
            close_tutorial(0);
            break;
    }
}
