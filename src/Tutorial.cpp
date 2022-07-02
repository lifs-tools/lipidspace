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

    main_widgets = {ui->actionLoad_list_s, ui->actionLoad_table, ui->actionQuit, ui->actionRemove_all_lipidomes, ui->actionSet_transparency, ui->actionAutomatically, ui->action2_columns, ui->action3_columns, ui->action4_columns, ui->action5_columns, ui->actionShow_global_lipidome, ui->actionShow_dendrogram, ui->action1_column, ui->action6_columns, ui->actionAbout, ui->actionLog_messages, ui->actionShow_quantitative_information, ui->actionIgnoring_lipid_sn_positions, ui->actionManage_lipidomes, ui->actionIgnore_quantitative_information, ui->actionUnbound_lipid_distance_metric, ui->actionExport_Results, ui->actionSet_number_of_principal_components, ui->actionSelect_principal_components, ui->actionImport_data_table, ui->actionImport_pivot_table, ui->actionSingle_linkage_clustering, ui->actionComplete_linkage_clustering, ui->actionAverage_linkage_clustering, ui->actionImport_mzTabM, ui->actionTranslate, ui->itemsTabWidget, ui->speciesComboBox, ui->speciesList, ui->classComboBox, ui->classList, ui->categoryComboBox, ui->categoryList, ui->treeWidget, ui->sampleComboBox, ui->sampleList, ui->normalizationComboBox, ui->applyChangesPushButton, ui->homeGraphicsView->firstTutorialPushButton, ui->homeGraphicsView->secondTutorialPushButton, ui->homeGraphicsView->thirdTutorialPushButton, ui->dendrogramView, ui->featureComboBox, ui->pieTreeSpinBox, ui->dendrogramHeightSpinBox, ui->pieSizeSpinBox, ui->startAnalysisPushButton, ui->statisticsBoxPlot, ui->statisticsHistogram, ui->featureComboBoxStat, ui->tickSizeSpinBox, ui->legendSizeSpinBox, ui->barNumberSpinBox, ui->menubar, ui->menuLipidSpace, ui->menuAnalysis, ui->menuClustering_strategy, ui->menuView, ui->menuTile_layout, ui->menuHelp, ui->viewsTabWidget, ui_it->tabWidget, ui_it->label_15, ui_it->sampleListWidgetRow, ui_it->okButtonRow, ui_it->cancelButtonRow, ui_it->ignoreListWidgetRow, ui_it->lipidListWidgetRow, ui_it->sampleListWidgetCol, ui_it->cancelButtonCol, ui_it->lipidListWidgetCol, ui_it->ignoreListWidgetCol, ui_it->numericalFeatureListWidgetCol, ui_it->nominalFeatureListWidgetCol, ui_it->flatTab, ui_it->lipidListWidgetFlat, ui_it->ignoreListWidgetFlat, ui_it->quantListWidgetFlat, ui_it->okButtonFlat, ui_it->numericalFeatureListWidgetFlat, ui_it->cancelButtonFlat, ui_it->sampleListWidgetFlat, ui_it->nominalFeatureListWidgetFlat, ui_it->tableWidget};


    // tutorial starts
    connect(ui->homeGraphicsView->firstTutorialPushButton, &QPushButton::clicked, this, &Tutorial::start_first_tutorial);
    connect(ui->homeGraphicsView->secondTutorialPushButton, &QPushButton::clicked, this, &Tutorial::start_second_tutorial);

    connect(&lipidSpaceGUI->import_table, &ImportTable::finished, this, &Tutorial::close_tutorial);
    connect(&lipidSpaceGUI->import_table, &ImportTable::rejected, this, &Tutorial::close_directly_tutorial);

    // actions
    connect(&lipidSpaceGUI->import_table, &ImportTable::importOpened, this, &Tutorial::action_performed);
    connect(lipidSpaceGUI, &LipidSpaceGUI::analysisCompleted, this, &Tutorial::action_performed);

    // tabs
    connect(lipidSpaceGUI->import_table.ui->tabWidget, &QTabWidget::currentChanged, this, &Tutorial::tab_changed);

    // lists
    connect(lipidSpaceGUI->import_table.ui->sampleListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(lipidSpaceGUI->import_table.ui->ignoreListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(lipidSpaceGUI->import_table.ui->nominalFeatureListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
    connect(lipidSpaceGUI->import_table.ui->lipidListWidgetCol->model(), &QAbstractItemModel::rowsRemoved, this, &Tutorial::item_changed);
}


Tutorial::~Tutorial() {
}



void Tutorial::x_clicked(){
    if (tutorialType == NoTutorial || (QMessageBox::question(this, "Quit tutorial", "Do you want to quit the tutorial?") == QMessageBox::Yes)){
        close_tutorial(0);
    }
}



void Tutorial::close_directly_tutorial(){
    close_tutorial(0);
}



void Tutorial::close_tutorial(int state){
    if (state) return;

    setVisible(false);
    hide_arrows();
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



void Tutorial::start_second_tutorial(){
    if (!can_start_tutorial()) return;
    step = 0;
    tutorialType = SecondTutorial;


    // TODO: Delete this block
    lipidSpaceGUI->resetAnalysis();
    vector<TableColumnType> *ct = new vector<TableColumnType>(371, LipidColumn);
    ct->at(0) = SampleColumn;
    ct->at(1) = FeatureColumnNominal;
    ct->at(2) = FeatureColumnNominal;
    ct->at(3) = FeatureColumnNominal;
    string path_to_example = QCoreApplication::applicationDirPath().toStdString();
    lipidSpaceGUI->loadTable(path_to_example + "/examples/Example-Dataset.xlsx", ct, COLUMN_PIVOT_TABLE, "Data");




    second_tutorial_steps();
}


void Tutorial::continue_tutorial(){
    step++;
    switch(tutorialType){
        case FirstTutorial: first_tutorial_steps(); break;
        case SecondTutorial: second_tutorial_steps(); break;
        default: close_tutorial(0); break;
    }
}


void Tutorial::disable(){
    continuePushButton->setEnabled(false);
    hide_arrows();
    for (auto obj : main_widgets){
        if (instanceof(obj, QWidget)){
            ((QWidget*)obj)->setEnabled(false);
        }
        else if (instanceof(obj, QAction)){
            ((QAction*)obj)->setEnabled(false);
        }
    }
}


void Tutorial::hide_arrows(){
    for (auto arrow : arrows){
        arrow->setVisible(false);
    }
}


void Tutorial::action_performed(){
    if (step < 0 || tutorialType == NoTutorial || !isVisible()) return;

    switch(tutorialType){
        case FirstTutorial: {

            switch((FirstSteps)step){
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

        } break;

        default:
            break;
    }
}


void Tutorial::tab_changed(int index){
    if (step < 0 || tutorialType == NoTutorial || !isVisible()) return;

    switch(tutorialType){
        case FirstTutorial: {

            switch((FirstSteps)step){
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

        } break;

        default:
            break;
    }
}


//void Tutorial::item_changed(QListWidgetItem *){
void Tutorial::item_changed(const QModelIndex &, int, int){
    if (step < 0 || tutorialType == NoTutorial || !isVisible()) return;

    switch(tutorialType){
        case FirstTutorial: {

            switch((FirstSteps)step){
                case FSampleEntryAssignment:
                    {
                        QListWidget *list = lipidSpaceGUI->import_table.ui->sampleListWidgetCol;
                        list->update();
                        continuePushButton->setEnabled(list->count() == 1 && list->item(0)->text().toStdString() == "Species");
                    }
                    break;

                case FStudyVarAssignment: {
                        set<string> feature_set = {"Type", "Treatment", "Type-Treatment"};
                        QListWidget *list = lipidSpaceGUI->import_table.ui->nominalFeatureListWidgetCol;
                        for (int i = 0; i < list->count(); ++i){
                            string s = list->item(i)->text().toStdString();
                            if (uncontains_val(feature_set, s)) break;
                            feature_set.erase(s);
                        }
                        continuePushButton->setEnabled(list->count() == 3 && feature_set.empty());
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
    pagesLabel->setText((std::to_string(step + 1) + " / 19").c_str());
    setVisible(true);
    titleLabel->setText("");
    informationLabel->setText("");
    QFontMetrics font_metrics(QApplication::font());

    switch((FirstSteps)step){
        case FStart:
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
                show_arrow(ALT, lipidSpaceGUI->ui->centralwidget, font_metrics.boundingRect(lipidSpaceGUI->ui->menuLipidSpace->title()).width() / 2.0, 0);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Where to import data?");
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
            //TODO: delete
            /*
            lipidSpaceGUI->import_table.show();
            lipidSpaceGUI->import_table.ui->tabWidget->setCurrentIndex(1);
            */

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
                QTabWidget *tab_widget = lipidSpaceGUI->import_table.ui->tabWidget;
                QRect r = tab_widget->tabBar()->tabRect(0);
                QRect r2 = tab_widget->tabBar()->tabRect(1);
                QPoint p = map_widget(tab_widget, &lipidSpaceGUI->import_table);
                show_arrow(ALT, &lipidSpaceGUI->import_table, p.x() + r.width() + r2.width() / 2.0, p.y() + r.height());
                titleLabel->setText("Select Column-Based Table for import");
                informationLabel->setText("Since the example table is column-based, please click at the 'Lipid column (pivot) table' tab.");
                lipidSpaceGUI->import_table.ui->tabWidget->setEnabled(true);
                lipidSpaceGUI->import_table.ui->columnTab->setEnabled(true);
            }
            break;


        case FExplainColumnField: {
                move(20, 400);
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
                QWidget *widget = lipidSpaceGUI->import_table.ui->numericalFeatureListWidgetCol;
                QPoint p = map_widget(widget, &lipidSpaceGUI->import_table);
                show_arrow(ARB, &lipidSpaceGUI->import_table, p.x() + widget->width() * 0.8, p.y());
                widget = lipidSpaceGUI->import_table.ui->nominalFeatureListWidgetCol;
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
            lipidSpaceGUI->import_table.ui->nominalFeatureListWidgetCol->setEnabled(true);
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


        case FFinishImport: {
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
            disable();
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
    pagesLabel->setText((std::to_string(step + 1) + " / 19").c_str());
    setVisible(true);
    titleLabel->setText("");
    informationLabel->setText("");
    QFontMetrics font_metrics(QApplication::font());

    switch((SecondSteps)step){
        case SStart:
            move(20, 20);
            titleLabel->setText("Second Tutorial - UI introduction");
            continuePushButton->setEnabled(true);
            informationLabel->setText("Welcome to the second tutorial of LipidSpace. In the previous tutorial, we've seen how to import data. Now we will discover the actual functions and windows / views of LipidSpace. For this, the example dataset will be automatically loaded now.");
            break;


        case SLoadTable:
            {
                move(420, 80);
                lipidSpaceGUI->resetAnalysis();
                vector<TableColumnType> *ct = new vector<TableColumnType>(371, LipidColumn);
                ct->at(0) = SampleColumn;
                ct->at(1) = FeatureColumnNominal;
                ct->at(2) = FeatureColumnNominal;
                ct->at(3) = FeatureColumnNominal;
                string path_to_example = QCoreApplication::applicationDirPath().toStdString();
                lipidSpaceGUI->loadTable(path_to_example + "/examples/Example-Dataset.xlsx", ct, COLUMN_PIVOT_TABLE, "Data");
                continuePushButton->setEnabled(true);
                titleLabel->setText("UI sections of LipidSpace");
                informationLabel->setText("Here we go. As you can see now, LipidSpace is split after data import into two major section. On the left hand side, you find the selection section and on the right hand side the visualization section.");
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

                move(420, 80);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Selection section");
                informationLabel->setText("Here we have the selection section. It contains five tabs, one for all imported lipid species, lipid classes, lipid categories, study varibles, and samples. In each tab, you can find a selection list of the according entity.");
            }
            break;


        case SSeletionSection2:
            {
                QWidget *widget = lipidSpaceGUI->ui->applyChangesPushButton;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ALB, lipidSpaceGUI, p.x() + widget->width() / 2., p.y());
                move(420, 80);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Rerun analysis");
                informationLabel->setText(QString("By default, all entries in each selection list are selected at the beginning. If you want to exclude certain lipid species, lipid classes, lipid categories, or even complete samples for a reanalysis of your data, you simple have to deselect the according entries and hit the '%1' button.").arg(((QPushButton*)widget)->text()));
            }
            break;


        case SVisualizationSection:
            {
                move(420, 80);
                QWidget *widget = lipidSpaceGUI->ui->itemsTabWidget;
                QPoint p = map_widget(widget, lipidSpaceGUI);
                show_arrow(ATL, lipidSpaceGUI, p.x() + widget->width(), p.y() + widget->height() / 2.);
                continuePushButton->setEnabled(true);
                titleLabel->setText("Selection section");
                informationLabel->setText("By defaul.");
            }
            break;




        case SEnd:
        default:
            close_tutorial(0);
            break;
    }
}