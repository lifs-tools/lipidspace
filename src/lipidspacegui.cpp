#include "lipidspace/lipidspacegui.h"


DragLayer::DragLayer(LipidSpaceGUI *_lipid_space_gui, QWidget *parent) : QWidget(parent) {
    lipid_space_gui = _lipid_space_gui;

}

void DragLayer::mousePressEvent(QMouseEvent*, Canvas *canvas){
    if (!isVisible() && canvas->num >= 0 && GlobalData::ctrl_pressed){
        source_tile = canvas->num;
        QWidget *current_widget = canvas;
        start_position = QPoint(0, 0);
        while (current_widget != parentWidget()){
            start_position += current_widget->pos();
            current_widget = current_widget->parentWidget();
        }
        resize(canvas->size());
        move(start_position);
        setVisible(true);

        delta = QCursor::pos();
        grabMouse();
        setMouseTracking(true);
    }
}

void DragLayer::mouseMoveEvent(QMouseEvent*){
    QPoint mouse = QCursor::pos();
    move(start_position + (mouse - delta));
    hover();
}


void DragLayer::mouseReleaseEvent(QMouseEvent*){
    releaseMouse();
    swapping(source_tile);
    setVisible(false);
    move(0, 0);
}


void DragLayer::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QColor qc = QColor(0, 110, 255, 50);
    painter.fillRect(QRect(0, 0, width(), height()), qc);
}



void LipidSpaceGUI::closeEvent(QCloseEvent *event){
    if (lipid_space->lipidomes.empty() || (QMessageBox::question(this, "Quit LipidSpace", "Do you want to quit LipidSpace?") == QMessageBox::Yes)) {
        event->accept();
    } else {
        event->ignore();
    }
}





void LipidSpaceGUI::keyReleaseEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Control){
        GlobalData::ctrl_pressed = false;
    }
}




void LipidSpaceGUI::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_1){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 324; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        for (int i = 1; i <= 40; ++i) ct->at(i) = FeatureColumnNumerical;
        ct->at(41) = FeatureColumnNominal;
        ct->at(42) = FeatureColumnNominal;
        loadTable(new ImportData("examples/Sales-Extended.xlsx", "Data", COLUMN_PIVOT_TABLE, ct));

    }
    else if (event->key() == Qt::Key_2){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 296; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(3) = FeatureColumnNominal;
        ct->at(4) = FeatureColumnNominal;
        ct->at(13) = FeatureColumnNominal;

        ct->at(1) = FeatureColumnNumerical;
        ct->at(2) = FeatureColumnNumerical;
        ct->at(5) = FeatureColumnNumerical;
        ct->at(6) = FeatureColumnNumerical;
        ct->at(7) = FeatureColumnNumerical;
        ct->at(8) = FeatureColumnNumerical;
        ct->at(9) = FeatureColumnNumerical;
        ct->at(10) = FeatureColumnNumerical;
        ct->at(11) = FeatureColumnNumerical;
        ct->at(12) = FeatureColumnNumerical;
        loadTable(new ImportData("examples/Tablesets/Plasma-Singapore.csv", "", COLUMN_PIVOT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_3){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 10; ++i) ct->push_back(IgnoreColumn);
        ct->at(3) = LipidColumn;
        ct->at(7) = QuantColumn;
        ct->at(4) = FeatureColumnNominal;
        ct->at(5) = FeatureColumnNominal;
        ct->at(0) = SampleColumn;

        loadTable(new ImportData("Platelets_Peng.csv", "", FLAT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_4){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 392; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(1) = FeatureColumnNominal;
        ct->at(2) = FeatureColumnNominal;
        ct->at(3) = FeatureColumnNominal;

        loadTable(new ImportData("Platelets_Peng_WT-vs-KO.csv", "", COLUMN_PIVOT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_5){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 10; ++i) ct->push_back(IgnoreColumn);
        ct->at(1) = LipidColumn;
        ct->at(8) = QuantColumn;
        ct->at(3) = SampleColumn;
        ct->at(2) = FeatureColumnNominal;

        loadTable(new ImportData("Heart_GPL.csv", "", FLAT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_6){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 20; ++i) ct->push_back(IgnoreColumn);
        ct->at(0) = LipidColumn;
        ct->at(18) = QuantColumn;
        ct->at(19) = SampleColumn;
        ct->at(13) = FeatureColumnNominal;
        ct->at(15) = FeatureColumnNominal;

        loadTable(new ImportData("Data_Thrombocytes_UKR_04042022.csv", "", FLAT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_7){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 13; ++i) ct->push_back(IgnoreColumn);
        ct->at(9) = LipidColumn;
        ct->at(10) = QuantColumn;
        ct->at(1) = SampleColumn;
        ct->at(2) = FeatureColumnNominal;
        ct->at(4) = FeatureColumnNominal;
        ct->at(5) = FeatureColumnNominal;
        ct->at(6) = FeatureColumnNominal;

        loadTable(new ImportData("examples/000.csv", "", FLAT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_8){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 327; ++i) ct->push_back(LipidColumn);
        ct->at(0) = IgnoreColumn;
        ct->at(1) = SampleColumn;
        ct->at(2) = IgnoreColumn;
        ct->at(3) = IgnoreColumn;
        ct->at(4) = IgnoreColumn;
        ct->at(5) = FeatureColumnNominal;
        ct->at(6) = FeatureColumnNumerical;
        ct->at(7) = FeatureColumnNominal;

        loadTable(new ImportData("Bakerpanel.csv", "", COLUMN_PIVOT_TABLE, ct));
    }

    else if (event->key() == Qt::Key_Control){
        GlobalData::ctrl_pressed = true;
    }

    else if (event->key() == Qt::Key_Space){




        /*
        ofstream regression("regression.csv");
        QComboBox *c = ui->featureComboBox;

        for (int i = 0; i < c->count(); ++i){
            if (c->itemText(i) == FILE_FEATURE_NAME) continue;
            regression << "\t" << c->itemText(i).toStdString();
        }
        regression << endl;

        for (int outer = 0; outer < c->count(); ++outer){
            if (c->itemText(outer) == FILE_FEATURE_NAME) continue;
            cout << "analysis for " << c->itemText(outer).toStdString() << endl;
            emit c->currentIndexChanged(outer);
            usleep(1000);
            emit ui->startAnalysisPushButton->click();
            usleep(1000);
            emit ui->applyChangesPushButton->click();
            usleep(1000);
            regression << c->itemText(outer).toStdString();

            for (int inner = 0; inner < c->count(); ++inner){
            if (c->itemText(inner) == FILE_FEATURE_NAME) continue;
                emit c->currentIndexChanged(inner);
                usleep(1000);
                cout << " - check " << c->itemText(inner).toStdString() << endl;

                if (statisticsBoxPlot.stat_results.size() > 0){
                    regression << "\t" << statisticsBoxPlot.stat_results[0].second;
                }
            }
            regression << endl;
        }
        */
    }

    else {
        if (Qt::Key_A <= event->key() && event->key() <= Qt::Key_Z){
            keystrokes += string(1, (char)event->key());
            if (keystrokes.length() > 6) keystrokes = keystrokes.substr(1);
            if (keystrokes == "BUTTER"){
                QMessageBox::information(this, "Important announcement.", "                  The butter, the better!                              ");
            }
            else if (keystrokes.length() >= 3 && keystrokes.substr(keystrokes.length() - 3) == "FAT"){
                QMessageBox::information(this, "Insight of the week.", "                  All that glitters is not fat!!                              ");
            }
        }
    }
}



LipidSpaceGUI::LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent) : QMainWindow(parent) {
    lipid_space = _lipid_space;
    ui = new Ui::LipidSpaceGUI();
    ui->setupUi(this);
    keystrokes = "";
    selected_d_lipidomes = 0;
    knubbel = false;
    GlobalData::ctrl_pressed = false;
    tutorial = new Tutorial(this, ui->centralwidget);

    statisticsBoxPlot.load_data(lipid_space, ui->statisticsBoxPlot);
    statisticsBarPlot.load_data(lipid_space, ui->statisticsBarPlot);
    statisticsHistogram.load_data(lipid_space, ui->statisticsHistogram);
    statisticsROCCurve.load_data(lipid_space, ui->statisticsROCCurve);

    qRegisterMetaType<string>("string");
    setWindowTitle(QApplication::translate("LipidSpaceGUI", ("LipidSpace - " + GlobalData::LipidSpace_version).c_str(), nullptr));


    connect(lipid_space, SIGNAL(fileLoaded()), this, SLOT(updateSelectionView()));
    connect(lipid_space, SIGNAL(reassembled()), this, SLOT(updateSelectionView()));

    dragLayer = new DragLayer(this, ui->centralwidget);
    dragLayer->move(0, 0);
    dragLayer->setVisible(false);
    dragLayer->setWindowFlags(Qt::FramelessWindowHint);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->speciesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->statisticsBoxPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsBarPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsHistogram->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsROCCurve->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->actionLoad_list_s, SIGNAL(triggered()), this, SLOT(openLists()));
    connect(ui->actionLoad_table, SIGNAL(triggered()), this, SLOT(openTable()));
    connect(ui->actionImport_mzTabM, SIGNAL(triggered()), this, SLOT(openMzTabM()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quitProgram()));
    connect(ui->tableWidget, SIGNAL(zooming()), this, SLOT(updateTable()));
    connect(ui->actionComplete_linkage_clustering, &QAction::triggered, this, &LipidSpaceGUI::setCompleteLinkage);
    connect(ui->actionAverage_linkage_clustering, &QAction::triggered, this, &LipidSpaceGUI::setAverageLinkage);
    connect(ui->actionSingle_linkage_clustering, &QAction::triggered, this, &LipidSpaceGUI::setSingleLinkage);
    connect(ui->actionAutomatically, &QAction::triggered, this, &LipidSpaceGUI::setAutomaticLayout);
    connect(ui->actionShow_quantitative_information, &QAction::triggered, this, &LipidSpaceGUI::showHideQuant);
    connect(ui->actionShow_global_lipidome, &QAction::triggered, this, &LipidSpaceGUI::showHideGlobalLipidome);
    connect(ui->actionShow_dendrogram, &QAction::triggered, this, &LipidSpaceGUI::showHideDendrogram);
    connect(ui->actionTranslate, &QAction::triggered, this, &LipidSpaceGUI::toggleLipidNameTranslation);
    connect(ui->action1_column, &QAction::triggered, this, &LipidSpaceGUI::set1ColumnLayout);
    connect(ui->action2_columns, &QAction::triggered, this, &LipidSpaceGUI::set2ColumnLayout);
    connect(ui->action3_columns, &QAction::triggered, this, &LipidSpaceGUI::set3ColumnLayout);
    connect(ui->action4_columns, &QAction::triggered, this, &LipidSpaceGUI::set4ColumnLayout);
    connect(ui->action5_columns, &QAction::triggered, this, &LipidSpaceGUI::set5ColumnLayout);
    connect(ui->action6_columns, &QAction::triggered, this, &LipidSpaceGUI::set6ColumnLayout);
    connect(ui->actionComplete_feature_analysis, &QAction::triggered, this, &LipidSpaceGUI::completeFeatureAnalysis);
    connect(ui->actionIgnoring_lipid_sn_positions, &QAction::triggered, this, &LipidSpaceGUI::setSnPositions);
    connect(ui->actionManage_lipidomes, &QAction::triggered, this, &LipidSpaceGUI::openManageLipidomesWindow);
    connect(ui->actionSet_transparency, &QAction::triggered, this, &LipidSpaceGUI::openSetAlpha);
    connect(ui->actionSet_number_of_principal_components, &QAction::triggered, this, &LipidSpaceGUI::openSetPCnum);
    connect(ui->actionSelect_principal_components, &QAction::triggered, this, &LipidSpaceGUI::openSelectPC);
    connect(ui->actionAbout, &QAction::triggered, this, &LipidSpaceGUI::openAbout);
    connect(ui->actionLog_messages, &QAction::triggered, this, &LipidSpaceGUI::openLog);
    connect(ui->actionIgnore_quantitative_information, &QAction::triggered, this, &LipidSpaceGUI::toggleQuant);
    connect(ui->actionUnbound_lipid_distance_metric, &QAction::triggered, this, &LipidSpaceGUI::toggleBoundMetric);
    connect(ui->actionExport_Results, &QAction::triggered, this, &LipidSpaceGUI::setExport);
    connect(ui->tableWidget, SIGNAL(cornerButtonClick()), this, SLOT(transposeTable()));
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowTableContextMenu);
    connect(ui->featureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setFeature(int)));
    connect(ui->featureComboBoxStat, SIGNAL(currentIndexChanged(int)), this, SLOT(setFeature(int)));
    connect(this, SIGNAL(featureChanged(string)), ui->dendrogramView, SLOT(setFeature(string)));
    connect(ui->speciesList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->classList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->categoryList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &LipidSpaceGUI::featureItemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &LipidSpaceGUI::featureItemDoubleClicked);
    connect(ui->sampleList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->applyChangesPushButton, &QPushButton::clicked, this, &LipidSpaceGUI::runAnalysis);
    connect(ui->pieTreeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &LipidSpaceGUI::setPieTree);
    connect(ui->dendrogramHeightSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &LipidSpaceGUI::setDendrogramHeight);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBoxPlot, &Statistics::setLegendSizeBoxPlot);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBarPlot, &Statistics::setLegendSizeBarPlot);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsHistogram, &Statistics::setLegendSizeHistogram);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsROCCurve, &Statistics::setLegendSizeROCCurve);
    connect(ui->barNumberSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsHistogram, &Statistics::setBarNumber);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBoxPlot, &Statistics::setTickSizeBoxPlot);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBarPlot, &Statistics::setTickSizeBarPlot);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsHistogram, &Statistics::setTickSizeHistogram);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsROCCurve, &Statistics::setTickSizeROCCurve);
    connect(ui->labelSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, ui->dendrogramView, &Canvas::setLabelSize);
    connect(ui->pieSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &LipidSpaceGUI::setPieSize);
    connect(ui->normalizationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setNormalization(int)));
    connect(ui->labelPieSize, SIGNAL(doubleClicked()), this, SLOT(setKnubbel()));
    connect(ui->startAnalysisPushButton, &QPushButton::clicked, this, &LipidSpaceGUI::startFeatureAnalysis);
    connect(ui->secondaryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSecondarySorting(int)));

    ui->speciesList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->classList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->categoryList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->sampleList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->speciesList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->classList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->categoryList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->sampleList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->statisticsBoxPlot, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsBoxPlot);
    connect(ui->statisticsBarPlot, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsBarPlot);
    connect(ui->statisticsHistogram, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsHistogram);
    connect(ui->statisticsROCCurve, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsROCCurve);

    sorting_boxes.push_back(ui->speciesComboBox);
    sorting_boxes.push_back(ui->classComboBox);
    sorting_boxes.push_back(ui->categoryComboBox);
    sorting_boxes.push_back(ui->sampleComboBox);
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderLabels({"Feature", "Filter"});

    tileLayout = AUTOMATIC;
    GlobalData::showQuant = true;
    showDendrogram = true;
    showGlobalLipidome = true;
    updating = false;
    GlobalData::color_counter = 0;
    single_window = -1;
    ui->frame->setVisible(false);
    table_transposed = false;

    progressbar = new Progressbar(this);
    progress = new Progress();
    lipid_space->progress = progress;
    connect(progress, SIGNAL(finish()), progressbar, SLOT(finish()));
    connect(progress, SIGNAL(set_current(int)), progressbar, SLOT(set_current(int)));
    connect(progress, SIGNAL(set_max(int)), progressbar, SLOT(set_max(int)));
    connect(progress, SIGNAL(error(QString)), progressbar, SLOT(errorClose(QString)));
    connect(progressbar, SIGNAL(interrupt()), progress, SLOT(interrupt()));
    connect(progressbar, SIGNAL(resetAnalysis()), this, SLOT(resetAnalysis()));
    progressbar->setModal(true);
    ui->dendrogramView->setDendrogramData(lipid_space);
    ui->dendrogramView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dendrogramView, &Canvas::rightClick, this, &LipidSpaceGUI::ShowContextMenuDendrogram);

    if (GlobalData::linkage == SingleLinkage) ui->actionSingle_linkage_clustering->setChecked(true);
    else if (GlobalData::linkage == AverageLinkage) ui->actionAverage_linkage_clustering->setChecked(true);
    else if (GlobalData::linkage == CompleteLinkage) ui->actionComplete_linkage_clustering->setChecked(true);
    GlobalData::last_folder = QCoreApplication::applicationDirPath();


    ui->speciesList->setItemDelegate(new ItemDelegate(ui->speciesList));
    ui->startAnalysisPushButton->setEnabled(false);


    connect(&import_table, &ImportTable::importTable, this, &LipidSpaceGUI::loadTable);
    import_table.setModal(true);


    // adding scene for home tab
    QGraphicsScene *scene = new QGraphicsScene();
    ui->homeGraphicsView->setFrameStyle(QFrame::NoFrame);
    ui->homeGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->homeGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->homeGraphicsView->setScene(scene);
    ui->homeGraphicsView->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QRectF r(0, 0, ui->homeGraphicsView->width(), ui->homeGraphicsView->height());
    ui->homeGraphicsView->setSceneRect(r);
    scene->addItem(new HomeItem(ui->homeGraphicsView));
    ui->startAnalysisPushButton->setToolTip("This function searches for the subset of lipids with the highest impact on the selected study variable.");

    updateGUI();
}



LipidSpaceGUI::~LipidSpaceGUI(){
    delete ui;
    delete progress;
    delete progressbar;
    delete dragLayer;
}




void LipidSpaceGUI::completeFeatureAnalysis(){
    QString file_name = QFileDialog::getSaveFileName(this, "Export feature analysis table", GlobalData::last_folder, "Worksheet *.xlsx (*.xlsx);;Data Table *.csv (*.csv);;Data Table *.tsv (*.tsv)");
    if (!file_name.length()) return;

    runAnalysisPID(3);
    for (int i = 0; i < ui->featureComboBox->count(); ++i){
        if(ui->featureComboBox->itemText(i).toStdString() == lipid_space->target_variable){
            emit ui->featureComboBox->currentIndexChanged(i);
            break;
        }
    }

    try {
        if (QFile::exists(file_name)){
            QFile::remove(file_name);
        }

        if (file_name.toLower().endsWith("csv") || file_name.toLower().endsWith("tsv")){
            string sep = file_name.toLower().endsWith("csv") ? "," : "\t";

            ofstream off(file_name.toStdString().c_str());

            // export study variables in first column
            vector<string> fv;
            for (auto kv : lipid_space->feature_values){
                if (kv.first == FILE_FEATURE_NAME) continue;
                fv.push_back(kv.first);
                off << sep << kv.first;
            } off << endl;

            for (uint r = 0; r < lipid_space->complete_feature_analysis_table.size(); ++r){
                off << fv[r];
                for (auto val : lipid_space->complete_feature_analysis_table[r]) off << sep << val;
                off << endl;
            }

        }
        else if (file_name.toLower().endsWith("xlsx")) {
            XLDocument doc;
            doc.create(file_name.toStdString());
            auto wbk = doc.workbook();
            wbk.addWorksheet("StudyVariableCorrelations");
            wbk.deleteSheet("Sheet1");
            auto wks = doc.workbook().worksheet("StudyVariableCorrelations");

            vector<string> fv;
            uint col = 2;
            for (auto kv : lipid_space->feature_values){
                if (kv.first == FILE_FEATURE_NAME) continue;
                fv.push_back(kv.first);
                wks.cell(1, col++).value() = kv.first;
            }

            for (uint r = 0; r < lipid_space->complete_feature_analysis_table.size(); ++r){
                uint col = 1;
                uint row = r + 2;
                wks.cell(row, col++).value() = fv[r];
                for (auto val : lipid_space->complete_feature_analysis_table[r]) wks.cell(row, col++).value() = val;
            }
            doc.save();

        }
        else {
            QMessageBox::information(this, "Export error", "Unknown export format for file '" + file_name + "'.");
            return;
        }
        QMessageBox::information(this, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(this, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}



void LipidSpaceGUI::setFeature(int c){
    disconnect(ui->featureComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->featureComboBoxStat, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->secondaryComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);

    ui->featureComboBox->setCurrentIndex(c);
    ui->featureComboBoxStat->setCurrentIndex(c);
    GlobalData::gui_string_var["study_var"] = ui->featureComboBox->currentText().toStdString();
    GlobalData::gui_string_var["study_var_stat"] = ui->featureComboBoxStat->currentText().toStdString();
    setSecondarySorting();
    GlobalData::gui_string_var["secondary_var"] = ui->secondaryComboBox->currentText().toStdString();

    statisticsBoxPlot.updateBoxPlot();
    statisticsBarPlot.updateBarPlot();
    statisticsHistogram.updateHistogram();
    statisticsROCCurve.updateROCCurve();
    featureChanged(ui->featureComboBox->currentText().toStdString());

    connect(ui->featureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setFeature(int)));
    connect(ui->featureComboBoxStat, SIGNAL(currentIndexChanged(int)), this, SLOT(setFeature(int)));
    connect(ui->secondaryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSecondarySorting(int)));
}




void LipidSpaceGUI::quitProgram(){
    close();
}




void LipidSpaceGUI::openTable(){
    import_table.show(lipid_space);
}



bool compare_string_asc(pair<string, double> a, pair<string, double> b){
    return a.first.compare(b.first) < 0;
}




bool compare_string_desc(pair<string, double> a, pair<string, double> b){
    return a.first.compare(b.first) > 0;
}



void LipidSpaceGUI::updateSecondarySorting(int){
    GlobalData::gui_string_var["secondary_var"] = ui->secondaryComboBox->currentText().toStdString();
    statisticsBoxPlot.updateBoxPlot();
    statisticsBarPlot.updateBarPlot();
    statisticsHistogram.updateHistogram();
    statisticsROCCurve.updateROCCurve();
}




void LipidSpaceGUI::setSecondarySorting(){
    ui->secondaryComboBox->clear();
    ui->secondaryLabel->setText("Secondary sorting");
    string target_variable = GlobalData::gui_string_var["study_var_stat"];
    if (!lipid_space || uncontains_val(lipid_space->feature_values, target_variable) || !lipid_space->analysis_finished) return;

    bool is_nominal = lipid_space->feature_values[target_variable].feature_type == NominalFeature;
    if (is_nominal){
        ui->secondaryLabel->setText("Secondary sorting");
        ui->secondaryComboBox->addItem("no secondary sorting");
        for (auto kv : lipid_space->feature_values){
            if (kv.second.feature_type == NumericalFeature){
                ui->secondaryComboBox->addItem(kv.first.c_str());
            }
        }
    }
    else {
        ui->secondaryLabel->setText("Conditional coloring");
        ui->secondaryComboBox->addItem("no conditional coloring");
        for (auto kv : lipid_space->feature_values){
            if (kv.second.feature_type == NominalFeature){
                ui->secondaryComboBox->addItem(kv.first.c_str());
            }
        }
    }
}



void LipidSpaceGUI::updateSelectionView(){

    // compute sort order for the groups
    disconnect(ui->speciesComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->classComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->categoryComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->sampleComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);


    for (int i = 0; i < 4; ++i){

        sortings[i].clear();
        sortings[i].insert({ALPHABETICALLY_ASC, vector<pair<string, double>>()});
        sortings[i].insert({ALPHABETICALLY_DESC, vector<pair<string, double>>()});

        vector<pair<string, double>> &sorting = sortings[i][ALPHABETICALLY_ASC];
        sorting.reserve(lipid_space->selection[i].size());
        for (auto &kv : lipid_space->selection[i]) sorting.push_back({kv.first, 0});
        sort(sorting.begin(), sorting.end(), compare_string_asc);


        vector<pair<string, double>> &sorting_desc = sortings[i][ALPHABETICALLY_DESC];
        sorting_desc.reserve(lipid_space->selection[i].size());
        for (auto &kv : lipid_space->selection[i]) sorting_desc.push_back({kv.first, 0});
        sort(sorting_desc.begin(), sorting_desc.end(), compare_string_desc);
    }

    sortings[0].insert(lipid_space->lipid_sortings.begin(), lipid_space->lipid_sortings.end());
    for (int i = 0; i < 4; ++i){
        sorting_boxes[i]->clear();
        for (auto& kv : sortings[i]){
            sorting_boxes[i]->addItem(kv.first.c_str());
        }
    }


    connect(ui->speciesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    connect(ui->classComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    connect(ui->categoryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    connect(ui->sampleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    updateView(0);
}




void LipidSpaceGUI::updateView(int){
    disconnect(ui->speciesList, &QListWidget::itemChanged, 0, 0);
    disconnect(ui->classList, &QListWidget::itemChanged, 0, 0);
    disconnect(ui->categoryList, &QListWidget::itemChanged, 0, 0);
    disconnect(ui->treeWidget, &QTreeWidget::itemChanged, 0, 0);
    disconnect(ui->sampleList, &QListWidget::itemChanged, 0, 0);

    // remove all items from the lists
    ui->speciesList->clear();
    ui->classList->clear();
    ui->categoryList->clear();
    ui->sampleList->clear();
    ui->treeWidget->clear();


    // load new data
    vector<pair<string, double>> &sort_species_labels = sortings[0][sorting_boxes[0]->currentText().toStdString()];
    map<string, string> &translations = lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];
    for (int i = 0; i < (int)sort_species_labels.size(); ++i){
        string lipid_name = sort_species_labels[i].first;
        ListItem *item = new ListItem(translations[lipid_name], SPECIES_ITEM, ui->speciesList, lipid_name);
        item->length = sort_species_labels[i].second;
        item->setCheckState(lipid_space->selection[0][lipid_name] ? Qt::Checked : Qt::Unchecked);
        ui->speciesList->addItem(item);
    }


    map<string, int> sort_class;
    vector<pair<string, double>> &sort_class_labels = sortings[1][sorting_boxes[1]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_class_labels.size(); ++i){
        sort_class.insert({sort_class_labels[i].first, i});
        string class_name = sort_class_labels[i].first;
        ListItem *item = new ListItem(class_name, CLASS_ITEM, ui->classList);
        item->setCheckState(lipid_space->selection[1][class_name] ? Qt::Checked : Qt::Unchecked);
        ui->classList->addItem(item);
    }

    map<string, int> sort_category;
    vector<pair<string, double>> &sort_category_labels = sortings[2][sorting_boxes[2]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_category_labels.size(); ++i){
        sort_category.insert({sort_category_labels[i].first, i});
        string category_name = sort_category_labels[i].first;
        ListItem *item = new ListItem(category_name, CATEGORY_ITEM, ui->categoryList);
        item->setCheckState(lipid_space->selection[2][category_name] ? Qt::Checked : Qt::Unchecked);
        ui->categoryList->addItem(item);
    }

    map<string, int> sort_sample;
    vector<pair<string, double>> &sort_sample_labels = sortings[3][sorting_boxes[3]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_sample_labels.size(); ++i){
        sort_sample.insert({sort_sample_labels[i].first, i});
        string sample_name = sort_sample_labels[i].first;
        ListItem *item = new ListItem(sample_name, SAMPLE_ITEM, ui->sampleList);
        item->setCheckState(lipid_space->selection[3][sample_name] ? Qt::Checked : Qt::Unchecked);
        ui->sampleList->addItem(item);
    }

    for (auto kv : lipid_space->feature_values){
        TreeItem *item = new TreeItem(0, QString(kv.first.c_str()), ui->treeWidget);
        ui->treeWidget->addTopLevelItem(item);
        if (kv.second.feature_type == NominalFeature){
            for (auto kv_nom : kv.second.nominal_values){
                TreeItem *child = new TreeItem(0, QString(kv_nom.first.c_str()), kv.first, item);
                child->setCheckState(0, kv_nom.second ? Qt::Checked : Qt::Unchecked);
            }
        }
        else {
            QString filter_label = "no filter set";
            switch(kv.second.numerical_filter.first){
                case LessFilter: filter_label = "less filter"; break;
                case GreaterFilter: filter_label = "greater filter"; break;
                case EqualFilter: filter_label = "equals filter"; break;
                case WithinRange: filter_label = "within range filter"; break;
                case OutsideRange: filter_label = "outside range filter"; break;
                default: break;
            }

            item->setText(1, filter_label);
        }
    }

    connect(ui->speciesList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->classList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->categoryList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &LipidSpaceGUI::featureItemChanged);
    connect(ui->sampleList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);

    GlobalData::gui_string_var["species_selection"] = ui->speciesComboBox->currentText().toStdString();
}



void LipidSpaceGUI::loadTable(ImportData *import_data){
    bool repeat_loading = true;
    ui->normalizationComboBox->clear();
    ui->normalizationComboBox->addItem("Absolute normalization", "absolute");
    GlobalData::normalization = "absolute";
    ui->normalizationComboBox->addItem("Relative normalization", "relative");
    while (repeat_loading){
        try {
            lipid_space->load_table(import_data);
            runAnalysis();
            for (auto feature : lipid_space->feature_values){
                if (feature.second.feature_type == NumericalFeature) continue;
                ui->normalizationComboBox->addItem(("Feature-grouped normalization: " + feature.first).c_str(), QVariant(feature.first.c_str()));
            }
            repeat_loading = false;
        }
        catch (LipidSpaceException &e) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Error during table import");
            msgBox.setText(e.what());

            switch(e.type){
                case LipidUnparsable:
                    {
                        msgBox.setInformativeText("Do you want to continue by ignoring unknown lipid species?");
                        QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
                        msgBox.addButton(tr("Abort"), QMessageBox::NoRole);
                        msgBox.exec();
                        if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                            lipid_space->ignore_unknown_lipids = true;
                        }
                        else {
                            repeat_loading = false;
                        }
                    }
                    break;

                case FileUnreadable:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                    }
                    break;

                case LipidDoublette:
                    {
                        msgBox.setInformativeText("Do you want to continue by ignoring doublette lipid species?");
                        QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
                        msgBox.addButton(tr("Abort"), QMessageBox::NoRole);
                        msgBox.exec();
                        if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                            lipid_space->ignore_doublette_lipids = true;
                        }
                        else {
                            repeat_loading = false;
                        }
                    }
                    break;

                case NoColumnFound:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                    }
                    break;

                case ColumnNumMismatch:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                    }
                    break;

                default:
                    {
                        msgBox.setInformativeText("Please check the log message. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                    }
                    break;
            }
        }
        catch (exception &e){
            Logging::write_log(e.what());
            QMessageBox::critical(this, "Unexpected Error", "An unexpected error happened. Please check the log file and get in contact with the developers.");
            repeat_loading = false;
            break;
        }
    }
    delete import_data;
}


void LipidSpaceGUI::startFeatureAnalysis(){
    lipid_space->target_variable = ui->featureComboBox->currentText().toStdString();
    lipid_space->process_id = 2;

    progress->reset();
    lipid_space->start();
    progressbar->exec();
    ui->itemsTabWidget->setCurrentIndex(0);
}



void LipidSpaceGUI::runAnalysis(){
    runAnalysisPID(1);
}


void LipidSpaceGUI::runAnalysisPID(int p_id){
    if (p_id != 1 && p_id != 3) return;

    string species_selection = GlobalData::gui_string_var["species_selection"];
    string study_var = GlobalData::gui_string_var["study_var"];
    string study_var_stat = GlobalData::gui_string_var["study_var_stat"];

    // clear all windows with canvases
    ui->dendrogramView->clear();
    lipid_space->analysis_finished = false;
    lipid_space->process_id = p_id;
    single_window = -1;
    ui->startAnalysisPushButton->setEnabled(false);
    disconnect(this, SIGNAL(transforming(QRectF)), 0, 0);
    disconnect(this, SIGNAL(updateCanvas()), 0, 0);

    for (auto canvas : canvases){
        disconnect(canvas, SIGNAL(transforming(QRectF)), 0, 0);
        disconnect(canvas, SIGNAL(showMessage(QString)), 0, 0);
        disconnect(canvas, SIGNAL(doubleClicked(int)), 0, 0);
        disconnect(canvas, SIGNAL(mouse(QMouseEvent*, Canvas*)), 0, 0);
        disconnect(canvas, SIGNAL(swappingLipidomes(int, int)), 0, 0);
        disconnect(canvas, &QGraphicsView::customContextMenuRequested, 0, 0);
        delete canvas;
    }
    canvases.clear();
    updateGUI();



    // (re)start analysis
    progress->reset();
    lipid_space->start();
    progressbar->exec(); // waiting for the progress bar to finish

    if (!lipid_space->analysis_finished) return;

    if (lipid_space->feature_values.size() > 1 || lipid_space->feature_values[FILE_FEATURE_NAME].nominal_values.size() > 1) ui->startAnalysisPushButton->setEnabled(true);

    if (lipid_space->global_lipidome->lipids.size() < 3 && !GlobalData::gui_num_var["dont_less_3_message"]){
        QMessageBox msg;
        msg.setWindowTitle("LipidSpace Analysis");
        msg.setWindowFlags(Qt::WindowStaysOnTopHint);
        msg.setText("Less than three lipids were taken for analysis. Therefore, no lipid spaces could be computed. However, dendrogram and statistics view can still be investigated.");
        QCheckBox *check_box = new QCheckBox("Don't show this again");
        msg.setCheckBox(check_box);
        msg.exec();
        GlobalData::gui_num_var["dont_less_3_message"] = check_box->isChecked();
    }

    // reset parameters
    GlobalData::color_counter = 0;
    GlobalData::feature_counter = 0;
    GlobalData::colorMap.clear();
    GlobalData::colorMapFeatures.clear();

    // setup all space canvases
    for (auto lipid_class : lipid_space->global_lipidome->classes){
        if (uncontains_val(GlobalData::colorMap, lipid_class)){
            GlobalData::colorMap.insert({lipid_class, GlobalData::COLORS[GlobalData::color_counter++ % GlobalData::COLORS.size()]});
        }
    }
    int numTiles = 2 * (lipid_space->selected_lipidomes.size() > 1) + lipid_space->selected_lipidomes.size();
    ui->dendrogramView->resetDendrogram();

    auto start = high_resolution_clock::now();
    canvases.resize(numTiles, 0);
    for (int n = 0; n < numTiles; ++n){
        int num = 0;
        if ((lipid_space->selected_lipidomes.size() > 1) && (n == 0)) num = -2;
        else if ((lipid_space->selected_lipidomes.size() > 1) && (n == 1)) num = -1;
        else num = max(0, n - 2 * (lipid_space->selected_lipidomes.size() > 1));

        Canvas* canvas = new Canvas(lipid_space, num, ui->speciesList, ui->centralwidget);
        connect(canvas, SIGNAL(doubleClicked(int)), this, SLOT(setDoubleClick(int)));
        if (num != -2){
            connect(canvas, SIGNAL(transforming(QRectF)), this, SLOT(setTransforming(QRectF)));
            connect(this, SIGNAL(transforming(QRectF)), canvas, SLOT(setTransforming(QRectF)));
            connect(canvas, SIGNAL(showMessage(QString)), this, SLOT(showMessage(QString)));
            connect(ui->speciesList, SIGNAL(itemSelectionChanged()), canvas, SLOT(highlightPoints()));
            connect(this, SIGNAL(updateCanvas()), canvas, SLOT(setUpdate()));
            connect(this, SIGNAL(exporting(string)), lipid_space, SLOT(store_results(string)));
            connect(canvas, SIGNAL(mouse(QMouseEvent*, Canvas*)), dragLayer, SLOT(mousePressEvent(QMouseEvent*, Canvas*)));
            connect(dragLayer, SIGNAL(hover()), canvas, SLOT(hoverOver()));
            connect(dragLayer, SIGNAL(swapping(int)), canvas, SLOT(setSwap(int)));
            connect(canvas, SIGNAL(swappingLipidomes(int, int)), this, SLOT(swapLipidomes(int, int)));
            connect(ui->speciesList, SIGNAL(itemSelectionChanged()), canvas, SLOT(highlightPoints()));
            canvas->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(canvas, &QGraphicsView::customContextMenuRequested, canvas, &Canvas::contextMenu);
            connect(canvas, &Canvas::context, this, &LipidSpaceGUI::ShowContextMenuLipidome);
        }
        if (num == -1){
            connect(ui->speciesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), canvas, SLOT(moveToPoint(QListWidgetItem*)));
        }
        canvases[n] = canvas;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "GUI: " << duration.count() << endl;



    // define colors of features
    for (auto kv : lipid_space->feature_values){
        if (kv.second.feature_type == NominalFeature){
            string feature_prefix = kv.first + "_";
            for (auto &kv_nom : kv.second.nominal_values){
                string feature = feature_prefix + kv_nom.first;
                if (uncontains_val(GlobalData::colorMapFeatures, feature)){
                    GlobalData::colorMapFeatures.insert({feature, GlobalData::COLORS[GlobalData::feature_counter++ % GlobalData::COLORS.size()]});
                }
            }
        }
        else {
            string feature_le = kv.first + "_le";
            if (uncontains_val(GlobalData::colorMapFeatures, feature_le)){
                GlobalData::colorMapFeatures.insert({feature_le, GlobalData::COLORS[GlobalData::feature_counter++ % GlobalData::COLORS.size()]});
            }
            string feature_gr = kv.first + "_gr";
            if (uncontains_val(GlobalData::colorMapFeatures, feature_gr)){
                GlobalData::colorMapFeatures.insert({feature_gr, GlobalData::COLORS[GlobalData::feature_counter++ % GlobalData::COLORS.size()]});
            }
        }
    }

    fill_table();
    ui->frame->setVisible(true);
    updateSelectionView();
    updateGUI();


    statisticsBoxPlot.updateBoxPlot();
    statisticsBarPlot.updateBarPlot();
    statisticsHistogram.updateHistogram();
    statisticsROCCurve.updateROCCurve();

    int pos = ui->speciesComboBox->findText(species_selection.c_str());
    if (pos >= 0) ui->speciesComboBox->setCurrentIndex(pos);
    pos = ui->featureComboBox->findText(study_var.c_str());
    if (pos >= 0) ui->featureComboBox->setCurrentIndex(pos);
    pos = ui->featureComboBoxStat->findText(study_var_stat.c_str());
    if (pos >= 0) ui->featureComboBoxStat->setCurrentIndex(pos);
    if (ui->viewsTabWidget->currentIndex() == 0) ui->viewsTabWidget->setCurrentIndex(2);

    // reset splitters for statistics tile view
    ui->splitterStatV1->setSizes(QList<int>{ui->splitterStat->height() >> 1, ui->splitterStat->height() >> 1});
    ui->splitterStatV2->setSizes(QList<int>{ui->splitterStat->height() >> 1, ui->splitterStat->height() >> 1});
    ui->splitterStat->setSizes(QList<int>{ui->splitterStat->width() >> 1, ui->splitterStat->width() >> 1});

    emit analysisCompleted();

}


void LipidSpaceGUI::reassembleSelection(){
    lipid_space->reassembleSelection();
}




void LipidSpaceGUI::itemChanged(QListWidgetItem *item){
    if (item == 0) return;
    ListItem *list_item = (ListItem*)item;
    ListItemType lit = list_item->type;
    string entity = list_item->system_name;
    if (contains_val(lipid_space->selection[(int)lit], entity)){
        lipid_space->selection[(int)lit][entity] = (item->checkState() == Qt::Checked);
    }
    else {
        Logging::write_log("Error: selection element '" + entity + "' was not found in the seletion map.");
        QMessageBox::critical(this, "Damn it, error", "Oh no, when you read this, an error happened that should never be expected to happen. Please check the log messages and send them to the developers. Thank you and sorry.");
    }
}


void LipidSpaceGUI::featureItemChanged(QTreeWidgetItem *item, int col){
    if (item == 0) return;
    TreeItem *tree_item = (TreeItem*)item;
    string feature = tree_item->feature;
    if (feature.length() == 0) return;
    if (contains_val(lipid_space->feature_values, feature)){
        string feature_value = item->text(col).toStdString();
        if (contains_val(lipid_space->feature_values[feature].nominal_values, feature_value)){
            lipid_space->feature_values[feature].nominal_values[feature_value] = (item->checkState(col) == Qt::Checked);
        }
        else {
            Logging::write_log("Error: feature value element '" + feature_value + "' was not found in the feature values map.");
            QMessageBox::critical(this, "Damn it, error", "Oh no, when you read this, an error happened that should never be expected to happen. Please check the log messages and send them to the developers. Thank you and sorry.");
        }
    }
    else {
        Logging::write_log("Error: feature selection element '" + feature + "' was not found in the feature seletion map.");
        QMessageBox::critical(this, "Damn it, error", "Oh no, when you read this, an error happened that should never be expected to happen. Please check the log messages and send them to the developers. Thank you and sorry.");
    }
}


void LipidSpaceGUI::featureItemDoubleClicked(QTreeWidgetItem *item, int){
    if (item == 0) return;
    TreeItem *tree_item = (TreeItem*)item;
    if (tree_item->feature.length() != 0) return;
    string feature = tree_item->text(0).toStdString();
    if (contains_val(lipid_space->feature_values, feature)){
        if (lipid_space->feature_values[feature].feature_type == NumericalFeature){
            FilterDialog fd(lipid_space->feature_values[feature].numerical_filter);
            fd.setModal(true);
            fd.exec();
            updateSelectionView();
        }
    }
    else {
        Logging::write_log("Error: feature selection element '" + feature + "' was not found in the feature seletion map.");
        QMessageBox::critical(this, "Damn it, error", "Oh no, when you read this, an error happened that should never be expected to happen. Please check the log messages and send them to the developers. Thank you and sorry.");
    }
}





void LipidSpaceGUI::setExport(){
    QString outputFolder = QFileDialog::getExistingDirectory(0, "Select Export Folder", GlobalData::last_folder);

    if (outputFolder.length()){
        QFileInfo fi(outputFolder);
        GlobalData::last_folder = fi.absoluteDir().absolutePath();
        exporting(outputFolder.toStdString());
        QMessageBox::information(this, "Export completed", "The export is completed into the folder '" + outputFolder + "'.");
    }
}


void LipidSpaceGUI::setTransforming(QRectF f){
    transforming(f);
}


void LipidSpaceGUI::setDoubleClick(int _num){
    _num += 2 * (lipid_space->selected_lipidomes.size() > 1);
    if (single_window >= 0) {
        single_window = -1;
    }
    else {
        single_window = _num;
    }
    updateGUI();
}



void LipidSpaceGUI::swapLipidomes(int source, int target){
    if (source == target) return;

    swap(lipid_space->lipidomes[source], lipid_space->lipidomes[target]);
    swap(canvases[2 + source]->num, canvases[2 + target]->num);  // offset of 2 due to dendrogram and global lipidome
    swap(canvases[2 + source], canvases[2 + target]);
    updateGUI();
}


void LipidSpaceGUI::resetAnalysis(){
    QLayoutItem *wItem;
    while ((wItem = ui->gridLayout->layout()->takeAt(0)) != 0){
        delete wItem;
    }
    for (auto canvas : canvases) delete canvas;
    canvases.clear();
    lipid_space->reset_analysis();
    GlobalData::PC1 = 0;
    GlobalData::PC2 = 1;
    LipidSpace::cols_for_pca = LipidSpace::cols_for_pca_init;

    ui->dendrogramView->resetDendrogram();
    ui->frame->setVisible(false);
    fill_table();
    updateGUI();
}


void LipidSpaceGUI::showMessage(QString message){
    statusBar()->showMessage(message);
}


void LipidSpaceGUI::toggleQuant(){
    lipid_space->without_quant = ui->actionIgnore_quantitative_information->isChecked();
    runAnalysis();
}



void LipidSpaceGUI::toggleBoundMetric(){
    lipid_space->unboundend_distance = ui->actionUnbound_lipid_distance_metric->isChecked();
    runAnalysis();
}


void LipidSpaceGUI::showHideDendrogram(){
    showDendrogram = ui->actionShow_dendrogram->isChecked();
    updateGUI();
}

void LipidSpaceGUI::toggleLipidNameTranslation(){
    GlobalData::gui_num_var["translate"] = ui->actionTranslate->isChecked() ? TRANSLATED_NAME : IMPORT_NAME;
    fill_table();
    updateView(0);
    updateGUI();
}

void LipidSpaceGUI::showHideGlobalLipidome(){
    showGlobalLipidome = ui->actionShow_global_lipidome->isChecked();
    updateGUI();
}

void LipidSpaceGUI::showHideQuant(){
    GlobalData::showQuant = ui->actionShow_quantitative_information->isChecked();
    updateCanvas();
}



void LipidSpaceGUI::setCompleteLinkage(){
    if (GlobalData::linkage != CompleteLinkage){
        ui->actionAverage_linkage_clustering->setChecked(false);
        ui->actionSingle_linkage_clustering->setChecked(false);
        GlobalData::linkage = CompleteLinkage;
        runAnalysis();
    }
    ui->actionComplete_linkage_clustering->setChecked(true);
}


void LipidSpaceGUI::setAverageLinkage(){
    if (GlobalData::linkage != AverageLinkage){
        ui->actionComplete_linkage_clustering->setChecked(false);
        ui->actionSingle_linkage_clustering->setChecked(false);
        GlobalData::linkage = AverageLinkage;
        runAnalysis();
    }
    ui->actionAverage_linkage_clustering->setChecked(true);
}


void LipidSpaceGUI::setSingleLinkage(){
    if (GlobalData::linkage != SingleLinkage){
        ui->actionComplete_linkage_clustering->setChecked(false);
        ui->actionAverage_linkage_clustering->setChecked(false);
        GlobalData::linkage = SingleLinkage;
        runAnalysis();
    }
    ui->actionSingle_linkage_clustering->setChecked(true);

}


void LipidSpaceGUI::setAutomaticLayout(){
    if (updating) return;
    tileLayout = AUTOMATIC;
    updateGUI();
}


void LipidSpaceGUI::set1ColumnLayout(){
    if (updating) return;
    tileLayout = ONE_COLULMN;
    updateGUI();
}


void LipidSpaceGUI::set2ColumnLayout(){
    if (updating) return;
    tileLayout = TWO_COLUMNS;
    updateGUI();
}


void LipidSpaceGUI::set3ColumnLayout(){
    if (updating) return;
    tileLayout = THREE_COLUMNS;
    updateGUI();
}


void LipidSpaceGUI::set4ColumnLayout(){
    if (updating) return;
    tileLayout = FOUR_COLUMNS;
    updateGUI();
}


void LipidSpaceGUI::set5ColumnLayout(){
    if (updating) return;
    tileLayout = FIVE_COLUMNS;
    updateGUI();
}


void LipidSpaceGUI::set6ColumnLayout(){
    if (updating) return;
    tileLayout = SIX_COLUMNS;
    updateGUI();
}


void LipidSpaceGUI::setSnPositions(){
    lipid_space->keep_sn_position = !ui->actionIgnoring_lipid_sn_positions->isChecked();
    runAnalysis();
    updateGUI();
}


void LipidSpaceGUI::openManageLipidomesWindow(){
    ManageLipidomes manageLipidomes(lipid_space, this);
    connect(&manageLipidomes, SIGNAL(runAnalysis()), this, SLOT(runAnalysis()));
    connect(&manageLipidomes, SIGNAL(resetAnalysis()), this, SLOT(resetAnalysis()));
    connect(&manageLipidomes, SIGNAL(reassembleSelection()), this, SLOT(reassembleSelection()));
    manageLipidomes.setModal(true);
    manageLipidomes.exec();
}


void LipidSpaceGUI::openSetAlpha(){
    SetAlpha setAlpha(this);
    setAlpha.setModal(true);
    setAlpha.exec();
    for (auto canvas : canvases){
        canvas->update_alpha();
    }
    updateGUI();
}


void LipidSpaceGUI::openSelectPC(){
    SelectPC selectPC(this);
    for (int i = (lipid_space->selected_lipidomes.size() > 1); i < (int)canvases.size(); ++i){
        connect(&selectPC, SIGNAL(reloadPoints()), canvases[i], SLOT(reloadPoints()));
    }
    selectPC.setModal(true);
    selectPC.exec();
    disconnect(&selectPC, SIGNAL(reloadPoints()), 0, 0);
    updateGUI();
}


void LipidSpaceGUI::openSetPCnum(){
    int pc_num = LipidSpace::cols_for_pca;
    SetPCnum setPCnum(this);
    setPCnum.setModal(true);
    setPCnum.exec();

    if (pc_num != LipidSpace::cols_for_pca) runAnalysis();
}


void LipidSpaceGUI::setPieTree(int depth){
    GlobalData::gui_num_var["pie_tree_depth"] = depth;
    ui->dendrogramView->setFeature(ui->featureComboBox->currentText().toStdString());
}


void LipidSpaceGUI::setNormalization(int){
    GlobalData::normalization = ui->normalizationComboBox->currentData().toString().toStdString();
}


void LipidSpaceGUI::setDendrogramHeight(int height){
    GlobalData::gui_num_var["dendrogram_height"] = height;
    ui->dendrogramView->setFeature(ui->featureComboBox->currentText().toStdString());
}


void LipidSpaceGUI::setPieSize(int size){
    GlobalData::gui_num_var["pie_size"] = size;
    ui->dendrogramView->setFeature(ui->featureComboBox->currentText().toStdString());
}

void LipidSpaceGUI::openAbout(){
    About about(this);
    about.setModal(true);
    about.exec();
}


void LipidSpaceGUI::openLog(){
    About about(this, true);
    about.setModal(true);
    about.exec();
}


void LipidSpaceGUI::setKnubbel(){
    knubbel = !knubbel;
    ui->labelPieSize->setText(knubbel ? "Knubbelgröße" : "Pie size");
}



void LipidSpaceGUI::resizeEvent(QResizeEvent *event){
    event->ignore();
}


void LipidSpaceGUI::updateGUI(){
    updating = true;

    QTabBar *bar = ui->viewsTabWidget->tabBar();
    for (int t = 1; t <= 4; ++t) bar->setTabEnabled(t, lipid_space->lipidomes.size() > 0);

    ui->featureComboBox->clear();
    ui->featureComboBoxStat->clear();

    ui->menuAnalysis->setEnabled(lipid_space->lipidomes.size());
    ui->menuView->setEnabled(lipid_space->lipidomes.size());
    ui->actionExport_Results->setEnabled(lipid_space->lipidomes.size());


    ui->actionAutomatically->setChecked(false);
    ui->action1_column->setChecked(false);
    ui->action2_columns->setChecked(false);
    ui->action3_columns->setChecked(false);
    ui->action4_columns->setChecked(false);
    ui->action5_columns->setChecked(false);
    ui->action6_columns->setChecked(false);

    switch(tileLayout){
        case AUTOMATIC: ui->actionAutomatically->setChecked(true); break;
        case ONE_COLULMN: ui->action1_column->setChecked(true); break;
        case TWO_COLUMNS: ui->action2_columns->setChecked(true); break;
        case THREE_COLUMNS: ui->action3_columns->setChecked(true); break;
        case FOUR_COLUMNS: ui->action4_columns->setChecked(true); break;
        case FIVE_COLUMNS: ui->action5_columns->setChecked(true); break;
        case SIX_COLUMNS: ui->action6_columns->setChecked(true); break;
    }

    QLayoutItem *wItem;
    while ((wItem = ui->gridLayout->layout()->takeAt(0)) != 0) {};

    if (!lipid_space->analysis_finished || !canvases.size()) return;


    if (single_window < 0){
        for (auto canvas : canvases) canvas->setVisible(false);
        int numTiles = 2 * (lipid_space->selected_lipidomes.size() > 1) + lipid_space->selected_lipidomes.size();
        int tileColumns = tileLayout == AUTOMATIC ? ceil(sqrt((double)numTiles)) : (int)tileLayout;
        set<int> show_canvas;

        int c = 0, r = 0;
        // show dendrogram if enabled
        if (lipid_space->selected_lipidomes.size() > 1){
            if (showDendrogram){
                ui->gridLayout->addWidget(canvases[0], r, c);
                if (++c == tileColumns){
                    c = 0;
                    ++r;
                }
				show_canvas.insert(0);
            }
        }

        // show global lipidome if enabled
        if (lipid_space->selected_lipidomes.size() > 1){
            if (showGlobalLipidome){
                ui->gridLayout->addWidget(canvases[1], r, c);
                if (++c == tileColumns){
                    c = 0;
                    ++r;
                }
				show_canvas.insert(1);
            }
        }

        // show all remaining lipidomes
        for(int n = 2 * (lipid_space->selected_lipidomes.size() > 1); n < numTiles; ++n){
            ui->gridLayout->addWidget(canvases[n], r, c);
            if (++c == tileColumns){
                c = 0;
                ++r;
            }
			show_canvas.insert(n);
        }
        for (auto i : show_canvas) canvases[i]->setVisible(true);
    }
    else {
        for (auto canvas : canvases) canvas->setVisible(false);
        canvases[single_window]->setVisible(true);
        ui->gridLayout->addWidget(canvases[single_window], 0, 0);
    }


    for (auto kv : lipid_space->feature_values){
        ui->featureComboBox->addItem(kv.first.c_str());
        ui->featureComboBoxStat->addItem(kv.first.c_str());
    }


    updating = false;
    dragLayer->raise();

}




void LipidSpaceGUI::openMzTabM(){

    QString file_name = QFileDialog::getOpenFileName(this, "Select one or more lipid lists", GlobalData::last_folder, "mzTabM files *.mzTab *.mzTabM *.mztab *.mztabm (*.mzTab *.mzTabM *.mztab *.mztabm)");
    if (!file_name.length()) return;

    QFileInfo fi(file_name);
    GlobalData::last_folder = fi.absoluteDir().absolutePath();
    bool start_analysis = true;
    bool repeat_loading = true;
    while (repeat_loading){
        try {
            lipid_space->load_mzTabM(file_name.toUtf8().constData());
            repeat_loading = false;
        }
        catch (LipidSpaceException &e) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Error during table import");
            msgBox.setText(e.what());

            switch(e.type){
                case LipidUnparsable:
                    {
                        msgBox.setInformativeText("Do you want to continue by ignoring unknown lipid species?");
                        QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
                        msgBox.addButton(tr("Abort"), QMessageBox::NoRole);
                        msgBox.exec();
                        if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                            lipid_space->ignore_unknown_lipids = true;
                        }
                        else {
                            repeat_loading = false;
                            start_analysis = false;
                        }
                    }
                    break;

                case FileUnreadable:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                        start_analysis = false;
                    }
                    break;

                case LipidDoublette:
                    {
                        msgBox.setInformativeText("Do you want to continue by ignoring doublette lipid species?");
                        QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
                        msgBox.addButton(tr("Abort"), QMessageBox::NoRole);
                        msgBox.exec();
                        if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                            lipid_space->ignore_doublette_lipids = true;
                        }
                        else {
                            repeat_loading = false;
                            start_analysis = false;
                        }
                    }
                    break;

                case NoColumnFound:
                case ColumnNumMismatch:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                        start_analysis = false;
                    }
                    break;

                default:
                    {
                        msgBox.setInformativeText("Please check the log message. In case, please contact the developers.");
                        msgBox.exec();
                        repeat_loading = false;
                        start_analysis = false;
                    }
                    break;
            }
        }
        catch (exception &e){
            Logging::write_log(e.what());
            QMessageBox::critical(this, "Unexpected Error", "An unexpected error happened. Please check the log file and get in contact with the developers.");
            repeat_loading = false;
            break;
        }
    }

    if (start_analysis){
        runAnalysis();
    }
}



void LipidSpaceGUI::openLists(){
    if (lipid_space->feature_values.size() > 1){
        QMessageBox::warning(this, "List conflict", "Study variables have been loaded. Lists do not supported any study variable import routine. Please reset LipidSpace.");
        return;
    }

    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more lipid lists", GlobalData::last_folder, "Lists *.csv *.tsv *.txt (*.csv *.tsv *.txt)");
    if (files.isEmpty()) return;

    bool start_analysis = true;
    for (QString file_name : files){
        if (!start_analysis) break;
        QFileInfo fi(file_name);
        GlobalData::last_folder = fi.absoluteDir().absolutePath();
        bool repeat_loading = true;
        while (repeat_loading){
            try {
                lipid_space->load_list(file_name.toUtf8().constData());
                repeat_loading = false;
            }
            catch (LipidSpaceException &e) {
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Error during table import");
                msgBox.setText(e.what());

                switch(e.type){
                    case LipidUnparsable:
                        {
                            msgBox.setInformativeText("Do you want to continue by ignoring unknown lipid species?");
                            QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
                            msgBox.addButton(tr("Abort"), QMessageBox::NoRole);
                            msgBox.exec();
                            if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                                lipid_space->ignore_unknown_lipids = true;
                            }
                            else {
                                repeat_loading = false;
                                start_analysis = false;
                            }
                        }
                        break;

                    case FileUnreadable:
                        {
                            msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                            msgBox.exec();
                            repeat_loading = false;
                            start_analysis = false;
                        }
                        break;

                    case LipidDoublette:
                        {
                            msgBox.setInformativeText("Do you want to continue by ignoring doublette lipid species?");
                            QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
                            msgBox.addButton(tr("Abort"), QMessageBox::NoRole);
                            msgBox.exec();
                            if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                                lipid_space->ignore_doublette_lipids = true;
                            }
                            else {
                                repeat_loading = false;
                                start_analysis = false;
                            }
                        }
                        break;

                    case NoColumnFound:
                    case ColumnNumMismatch:
                        {
                            msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                            msgBox.exec();
                            repeat_loading = false;
                            start_analysis = false;
                        }
                        break;

                    default:
                        {
                            msgBox.setInformativeText("Please check the log message. In case, please contact the developers.");
                            msgBox.exec();
                            repeat_loading = false;
                            start_analysis = false;
                        }
                        break;
                }
            }
            catch (exception &e){
                Logging::write_log(e.what());
                QMessageBox::critical(this, "Unexpected Error", "An unexpected error happened. Please check the log file and get in contact with the developers.");
                repeat_loading = false;
                break;
            }
        }

    }

    if (start_analysis){
        runAnalysis();
    }
}



void LipidSpaceGUI::selectDendrogramLipidomes(){
    if (selected_d_lipidomes){
        QListWidget *widget = ui->sampleList;
        for (int i = 0; i < widget->count(); ++i){
            widget->item(i)->setCheckState(Qt::Unchecked);
        }
        set<string> lipidome_names;
        for (int c : *selected_d_lipidomes){
            lipidome_names.insert(lipid_space->selected_lipidomes.at(c)->cleaned_name);
        }
        for (int i = 0; i < widget->count(); ++i){
            if (contains_val(lipidome_names, widget->item(i)->text().toStdString())){
                widget->item(i)->setCheckState(Qt::Checked);
            }
        }
        emit ui->itemsTabWidget->setCurrentIndex(4);
        selected_d_lipidomes = 0;
    }
}




void LipidSpaceGUI::copy_to_clipboard(){

    QList<QTableWidgetSelectionRange> ranges = ui->tableWidget->selectedRanges();
    if (ranges.size() != 1) return;
    QTableWidgetSelectionRange range = ranges.first();
    QTableWidget *t = ui->tableWidget;
    QString str;
    for (int r = range.topRow(); r <= range.bottomRow(); ++r){
        for (int c = range.leftColumn(); c <= range.rightColumn(); ++c){
            QTextStream(&str) << t->item(r, c)->text();
            if (c < range.rightColumn()) QTextStream(&str) << "\t";
        }
        QTextStream(&str) << "\n";
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}




void LipidSpaceGUI::ShowContextMenuDendrogram(const QPoint pos, set<int> *selected_d_lipidomes){
    if (!lipid_space->analysis_finished) return;

    if (selected_d_lipidomes){
        this->selected_d_lipidomes = selected_d_lipidomes;
        QMenu *menu = new QMenu(this);
        QAction *actionSelect = new QAction("Select these lipidomes in sample selection", this);
        menu->addAction(actionSelect);
        menu->popup(ui->dendrogramView->viewport()->mapToGlobal(pos));
        connect(actionSelect, &QAction::triggered, this, &LipidSpaceGUI::selectDendrogramLipidomes);
    }
    else {
        QMenu *menu = new QMenu(this);
        QAction *exportAsPdf = new QAction("Export as pdf", this);
        menu->addAction(exportAsPdf);
        menu->popup(ui->dendrogramView->viewport()->mapToGlobal(pos));
        connect(exportAsPdf, &QAction::triggered, ui->dendrogramView, &Canvas::exportAsPdf);
    }

}



void LipidSpaceGUI::ShowContextMenuLipidome(Canvas *canvas, QPoint pos){
    QMenu *menu = new QMenu(this);
    QAction *exportAsPdf = new QAction("Export as pdf", this);
    menu->addAction(exportAsPdf);

    menu->popup(canvas->viewport()->mapToGlobal(pos));
    connect(exportAsPdf, &QAction::triggered, canvas, &Canvas::exportAsPdf);
}


void LipidSpaceGUI::ShowTableContextMenu(const QPoint pos){
    QMenu *menu = new QMenu(this);
    QAction *copyToClipboard = new QAction("Copy selection", this);
    QAction *exportTable = new QAction("Export complete Table", this);
    menu->addAction(copyToClipboard);
    menu->addAction(exportTable);

    menu->popup(ui->tableWidget->viewport()->mapToGlobal(pos));
    connect(copyToClipboard, &QAction::triggered, this, &LipidSpaceGUI::copy_to_clipboard);
    connect(exportTable, &QAction::triggered, this, &LipidSpaceGUI::export_table);
}




void LipidSpaceGUI::export_table(){
    QString file_name = QFileDialog::getSaveFileName(this, "Export table", GlobalData::last_folder, "Worksheet *.xlsx (*.xlsx)");
    if (!file_name.length()) return;

    try {
        if (QFile::exists(file_name)){
            QFile::remove(file_name);
        }

        XLDocument doc;
        doc.create(file_name.toStdString());
        auto wbk = doc.workbook();
        wbk.addWorksheet("LipidomicsData");
        wbk.deleteSheet("Sheet1");
        auto wks_data = doc.workbook().worksheet("LipidomicsData");


        QTableWidget *t = ui->tableWidget;
        int cols = t->columnCount();
        int rows = t->rowCount();

        for (int col = 0; col < cols; ++col) wks_data.cell(1, 2 + col).value() = t->horizontalHeaderItem(col)->text().toStdString();
        for (int row = 0; row < rows; ++row) wks_data.cell(2 + row, 1).value() = t->verticalHeaderItem(row)->text().toStdString();

        for (int col = 0; col < cols; ++col){
            for (int row = 0; row < rows; ++row){
                bool isnumber = false;
                QVariant variant = t->item(row, col)->data(1);
                double value = t->item(row, col)->data(1).toDouble(&isnumber);
                if (isnumber){
                    wks_data.cell(2 + row, 2 + col).value() = value;
                }
                else {
                    wks_data.cell(2 + row, 2 + col).value() = t->item(row, col)->data(1).toString().toStdString();
                }
            }
        }

        doc.save();

        QMessageBox::information(this, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(this, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}






void LipidSpaceGUI::export_list(){
    QString outputFile = QFileDialog::getSaveFileName(this, tr("Save File"), GlobalData::last_folder, tr("csv (*.csv)"));

    if (!outputFile.length()) return;

    QFileInfo fi(outputFile);
    GlobalData::last_folder = fi.absoluteDir().absolutePath();

    int selection = 0;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0:
        case 1:
        case 2: selection = ui->itemsTabWidget->currentIndex(); break;
        case 4: selection = 3; break;
        default: return;
    }

    ofstream output_stream(outputFile.toStdString().c_str());
    map<string, string> &translations = lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];

    vector<pair<string, double>> &sort_species_labels = sortings[selection][sorting_boxes[selection]->currentText().toStdString()];
    map<string, bool> &selection_map = lipid_space->selection[selection];
    for (int i = 0; i < (int)sort_species_labels.size(); ++i){
        string label = sort_species_labels[i].first;
        if (selection_map[label]) output_stream << translations[label] << endl;

    }

    QMessageBox::information(this, "Export completed", "The list was export into the file '" + outputFile + "'.");
}


void LipidSpaceGUI::ShowContextMenuStatisticsBoxPlot(const QPoint pos){
    if (statisticsBoxPlot.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionShowData = new QAction("Show data", this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    actionShowData->setCheckable(true);
    actionShowData->setChecked(statisticsBoxPlot.show_data);
    menu->addAction(actionShowData);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    connect(actionShowData, &QAction::triggered, &statisticsBoxPlot, &Statistics::setShowDataBoxPlot);
    connect(actionData, &QAction::triggered, &statisticsBoxPlot, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsBoxPlot, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsBoxPlot->viewport()->mapToGlobal(pos));
}


void LipidSpaceGUI::ShowContextMenuStatisticsROCCurve(const QPoint pos){
    if (statisticsROCCurve.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    connect(actionData, &QAction::triggered, &statisticsROCCurve, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsROCCurve, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsROCCurve->viewport()->mapToGlobal(pos));
}


void LipidSpaceGUI::ShowContextMenuStatisticsBarPlot(const QPoint pos){
    if (statisticsBarPlot.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionLogScale = new QAction("Y-axis in log scale", this);
    QAction *actionShowData = new QAction("Show data", this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    actionLogScale->setCheckable(true);
    actionLogScale->setChecked(statisticsBarPlot.log_scale);
    actionShowData->setCheckable(true);
    actionShowData->setChecked(statisticsBarPlot.show_data);
    menu->addAction(actionLogScale);
    menu->addAction(actionShowData);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    connect(actionLogScale, &QAction::triggered, &statisticsBarPlot, &Statistics::setLogScaleBarPlot);
    connect(actionShowData, &QAction::triggered, &statisticsBarPlot, &Statistics::setShowDataBarPlot);
    connect(actionData, &QAction::triggered, &statisticsBarPlot, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsBarPlot, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsBarPlot->viewport()->mapToGlobal(pos));


}


void LipidSpaceGUI::ShowContextMenuStatisticsHistogram(const QPoint pos){
    if (statisticsHistogram.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    connect(actionData, &QAction::triggered, &statisticsHistogram, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsHistogram, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsHistogram->viewport()->mapToGlobal(pos));


}


void LipidSpaceGUI::ShowContextMenu(const QPoint pos){
    QMenu *menu = new QMenu(this);
    if (ui->itemsTabWidget->currentIndex() != 3){
        QAction *actionSelectAll = new QAction("Check all", this);
        QAction *actionDeselectAll = new QAction("Uncheck all", this);
        QAction *actionToggleAll = new QAction("Toggle all", this);
        QAction *actionSelectSelected = new QAction("Check selected", this);
        QAction *actionDeselectSelected = new QAction("Uncheck selected", this);
        QAction *actionToggleSelected = new QAction("Toggle selected", this);
        QAction *actionExportList = new QAction("Export list (selected only)", this);
        menu->addAction(actionSelectSelected);
        menu->addAction(actionDeselectSelected);
        menu->addAction(actionToggleSelected);
        menu->addSeparator();
        menu->addAction(actionSelectAll);
        menu->addAction(actionDeselectAll);
        menu->addAction(actionToggleAll);
        menu->addSeparator();
        menu->addAction(actionExportList);

        QListWidget *widget = nullptr;
        switch(ui->itemsTabWidget->currentIndex()){
            case 0: widget = ui->speciesList; break;
            case 1: widget = ui->classList; break;
            case 2: widget = ui->categoryList; break;
            case 4: widget = ui->sampleList; break;
            default: return;
        }
        menu->popup(widget->viewport()->mapToGlobal(pos));
        connect(actionSelectAll, &QAction::triggered, this, &LipidSpaceGUI::check_all_entities);
        connect(actionDeselectAll, &QAction::triggered, this, &LipidSpaceGUI::uncheck_all_entities);
        connect(actionToggleAll, &QAction::triggered, this, &LipidSpaceGUI::toggle_all_entities);
        connect(actionSelectSelected, &QAction::triggered, this, &LipidSpaceGUI::check_selected_entities);
        connect(actionDeselectSelected, &QAction::triggered, this, &LipidSpaceGUI::uncheck_selected_entities);
        connect(actionToggleSelected, &QAction::triggered, this, &LipidSpaceGUI::toggle_selected_entities);
        connect(actionExportList, &QAction::triggered, this, &LipidSpaceGUI::export_list);
    }
    else {
        QAction *actionSelectAll = new QAction("Select all nominal features", this);
        QAction *actionDeselectAll = new QAction("Deselect all nominal features", this);
        QAction *actionResetAll = new QAction("Reset all numerical features", this);
        menu->addAction(actionSelectAll);
        menu->addAction(actionDeselectAll);
        menu->addAction(actionResetAll);
        menu->popup(ui->treeWidget->viewport()->mapToGlobal(pos));
        connect(actionSelectAll, &QAction::triggered, this, &LipidSpaceGUI::select_all_features);
        connect(actionDeselectAll, &QAction::triggered, this, &LipidSpaceGUI::deselect_all_features);
        connect(actionResetAll, &QAction::triggered, this, &LipidSpaceGUI::reset_all_features);
    }
}



void LipidSpaceGUI::check_all_entities(){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (int i = 0; i < widget->count(); ++i){
        widget->item(i)->setCheckState(Qt::Checked);
    }
}



void LipidSpaceGUI::check_selected_entities(){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (auto item : widget->selectedItems()){
        item->setCheckState(Qt::Checked);
    }
}



void LipidSpaceGUI::uncheck_all_entities(){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (int i = 0; i < widget->count(); ++i){
        widget->item(i)->setCheckState(Qt::Unchecked);
    }
}



void LipidSpaceGUI::uncheck_selected_entities(){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (auto item : widget->selectedItems()){
        item->setCheckState(Qt::Unchecked);
    }
}



void LipidSpaceGUI::toggle_all_entities(){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (int i = 0; i < widget->count(); ++i){
        widget->item(i)->setCheckState(widget->item(i)->checkState() ? Qt::Unchecked :  Qt::Checked);
    }
}



void LipidSpaceGUI::toggle_selected_entities(){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (auto item : widget->selectedItems()){
        item->setCheckState(item->checkState() ? Qt::Unchecked :  Qt::Checked);
    }
}



void LipidSpaceGUI::select_all_features(){
    vector<QTreeWidgetItem*> stack;
    stack.push_back(ui->treeWidget->invisibleRootItem());
    while(stack.size() > 0){
        TreeItem *item = (TreeItem*)stack.back();
        stack.pop_back();
        if (item->feature != "") item->setCheckState(0, Qt::Checked);
        for (int i = 0; i < (int)item->childCount(); ++i) stack.push_back(item->child(i));
    }
}



void LipidSpaceGUI::deselect_all_features(){
    vector<QTreeWidgetItem*> stack;
    stack.push_back(ui->treeWidget->invisibleRootItem());
    while(stack.size() > 0){
        TreeItem *item = (TreeItem*)stack.back();
        stack.pop_back();
        if (item->feature != "") item->setCheckState(0, Qt::Unchecked);
        for (int i = 0; i < (int)item->childCount(); ++i) stack.push_back(item->child(i));
    }
}


void LipidSpaceGUI::reset_all_features(){
    for (auto &kv : lipid_space->feature_values){
        kv.second.numerical_filter.first = NoFilter;
        kv.second.numerical_filter.second.clear();
    }
    updateView(0);
}



void LipidSpaceGUI::transposeTable(){
    table_transposed = !table_transposed;
    fill_table();
}




void LipidSpaceGUI::updateTable(){
    QTableWidget *t = ui->tableWidget;

    int rows = t->rowCount();
    int cols = t->columnCount();

    if (rows == 0 || cols == 0) return;

    QFont item_font("Helvetica", (int)GlobalData::gui_num_var["table_zoom"]);
    for (int c = 0; c < cols; c++) t->horizontalHeaderItem(c)->setFont(item_font);
    for (int r = 0; r < rows; ++r){
        t->verticalHeaderItem(r)->setFont(item_font);
        for (int c = 0; c < cols; c++) t->item(r, c)->setFont(item_font);
    }
    t->resizeColumnsToContents();
    t->resizeRowsToContents();
}





void LipidSpaceGUI::fill_table(){
    QTableWidget *t = ui->tableWidget;
    QTableWidgetItem *item = 0;

    QFont item_font("Helvetica", (int)GlobalData::gui_num_var["table_zoom"]);


    t->setRowCount(0);
    t->setColumnCount(0);

    if ((int)lipid_space->selected_lipidomes.size() == 0 || (int)lipid_space->global_lipidome->lipids.size() == 0) return;
    map<string, string> &translations = lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];

    int num_features = lipid_space->feature_values.size();
    map<string, int> lipid_index;
    map<string, int> feature_index;

    // achieve sorted list of lipid species
    set< string > sorted_lipid_species;
    set< string > selected_species;
    set< string > selected_lipidomes;
    set< int > unselected_lipidome_indexes;
    for (auto lipidome : lipid_space->lipidomes){
        for (auto lipid_species : lipidome->species) sorted_lipid_species.insert(lipid_species);
    }
    for (auto lipid_species : lipid_space->global_lipidome->species) selected_species.insert(lipid_species);
    for (auto lipidome : lipid_space->selected_lipidomes) selected_lipidomes.insert(lipidome->cleaned_name.c_str());

    if (table_transposed){
        t->setColumnCount(lipid_space->lipidomes.size());
        t->setRowCount(sorted_lipid_species.size() + num_features);

        for (int c = 0; c < (int)lipid_space->lipidomes.size(); c++){
            item = new QTableWidgetItem(QString(lipid_space->lipidomes[c]->cleaned_name.c_str()));
            item->setData(1, lipid_space->lipidomes[c]->cleaned_name.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setFont(item_font);
            t->setHorizontalHeaderItem(c, item);
            if (uncontains_val(selected_lipidomes, lipid_space->lipidomes[c]->cleaned_name.c_str())) unselected_lipidome_indexes.insert(c);
        }

        int f = 0;
        for (auto kv : lipid_space->feature_values){
            item = new QTableWidgetItem(kv.first.c_str());
            item->setData(1, kv.first.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setFont(item_font);
            feature_index.insert({kv.first, f});
            t->setVerticalHeaderItem(f++, item);
        }

        int rrr = 0;
        for (auto header_std : sorted_lipid_species){
            QString header_name = translations[header_std].c_str();

            // dirty way to make the transpose button completely visible
            if (f == 0 && header_name.length() < 10) {
                while (header_name.length() < 14) header_name += " ";
            }
            item = new QTableWidgetItem(header_name);
            item->setData(1, header_name);
            item->setFont(item_font);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setVerticalHeaderItem(num_features + rrr, item);
            lipid_index.insert({header_std, num_features + rrr});
            rrr++;
        }

        // fill features and content
        int R = t->rowCount();
        int C = t->columnCount();
        int n = R * C;
        Bitfield free_cells(n, true);

        for (int c = 0; c < C; c++){
            // add features
            for(auto kv : lipid_space->lipidomes[c]->features){
                if (kv.second.feature_type == NominalFeature){
                    item = new QTableWidgetItem(kv.second.nominal_value.c_str());
                    item->setData(1, kv.second.nominal_value.c_str());
                }
                else {
                    item = new QTableWidgetItem(QString::number(kv.second.numerical_value));
                    item->setData(1, kv.second.numerical_value);
                }
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

                if (contains_val(unselected_lipidome_indexes, c)){
                    item->setData(Qt::ForegroundRole, QBrush(QColor(180, 180, 180)));
                }
                item->setFont(item_font);
                int r = feature_index[kv.first];
                t->setItem(r, c, item);
                free_cells.remove(r * C + c);
            }
            // add lipid quant data
            for (int r = 0; r < (int)lipid_space->lipidomes[c]->species.size(); r++){
                QString qn = QString::number(lipid_space->lipidomes[c]->original_intensities[r]);
                item = new QTableWidgetItem(qn);
                item->setData(1, lipid_space->lipidomes[c]->original_intensities[r]);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                item->setFont(item_font);

                if (uncontains_val(selected_species, lipid_space->lipidomes[c]->species[r]) || contains_val(unselected_lipidome_indexes, c)){
                    item->setData(Qt::ForegroundRole, QBrush(QColor(180, 180, 180)));
                }

                int rr = lipid_index[lipid_space->lipidomes[c]->species[r]];
                t->setItem(rr, c, item);
                free_cells.remove(rr * C + c);
            }
        }
        // fill the empty fields with disabled cells
        for (auto pos : free_cells){
            item = new QTableWidgetItem("");
            item->setData(1, "");
            item->setFont(item_font);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            int r = pos / C;
            int c = pos % C;
            t->setItem(r, c, item);
        }
    }





    else {
        // fill headers
        t->setRowCount(lipid_space->lipidomes.size());
        t->setColumnCount(sorted_lipid_species.size() + num_features);

        for (int r = 0; r < (int)lipid_space->lipidomes.size(); ++r){
            int h = 0;
            QString header_name = lipid_space->lipidomes[r]->cleaned_name.c_str();
            // dirty way to make the transpose button completely visible
            if (h++ == 0 && header_name.length() < 10) {
                while (header_name.length() < 14) header_name += " ";
            }
            item = new QTableWidgetItem(header_name);
            item->setData(1, header_name);
            item->setFont(item_font);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setVerticalHeaderItem(r, item);
            if (uncontains_val(selected_lipidomes, lipid_space->lipidomes[r]->cleaned_name.c_str())) unselected_lipidome_indexes.insert(r);
        }

        int f = 0;
        for (auto kv : lipid_space->feature_values){
            item = new QTableWidgetItem(kv.first.c_str());
            item->setData(1, kv.first.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setFont(item_font);
            feature_index.insert({kv.first, f});
            t->setHorizontalHeaderItem(f++, item);
        }

        int ccc = 0;
        for (auto header_std : sorted_lipid_species){
        //for (int c = 0; c < (int)lipid_space->global_lipidome->species.size(); c++){
            item = new QTableWidgetItem(translations[header_std].c_str());
            item->setData(1, translations[header_std].c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setFont(item_font);
            t->setHorizontalHeaderItem(num_features + ccc, item);
            lipid_index.insert({header_std, num_features + ccc});
            ccc++;
        }


        // fill features and content
        int R = t->rowCount();
        int C = t->columnCount();
        int n = R * C;
        Bitfield free_cells(n, true);

        for (int r = 0; r < R; ++r){
            // add features
            for(auto kv : lipid_space->lipidomes[r]->features){
                if (kv.second.feature_type == NominalFeature){
                    item = new QTableWidgetItem(kv.second.nominal_value.c_str());
                    item->setData(1, kv.second.nominal_value.c_str());
                }
                else {
                    item = new QTableWidgetItem(QString::number(kv.second.numerical_value));
                    item->setData(1, kv.second.numerical_value);
                }
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (contains_val(unselected_lipidome_indexes, r)){
                    item->setData(Qt::ForegroundRole, QBrush(QColor(180, 180, 180)));
                }
                item->setFont(item_font);
                int c = feature_index[kv.first];
                t->setItem(r, c, item);
                free_cells.remove(r * C + c);
            }

            // add lipid quant data
            for (int c = 0; c < (int)lipid_space->lipidomes[r]->species.size(); c++){
                item = new QTableWidgetItem(QString::number(lipid_space->lipidomes[r]->original_intensities[c]));
                    item->setData(1, lipid_space->lipidomes[r]->original_intensities[c]);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (uncontains_val(selected_species, lipid_space->lipidomes[r]->species[c]) || contains_val(unselected_lipidome_indexes, r)){
                    item->setData(Qt::ForegroundRole, QBrush(QColor(180, 180, 180)));
                }
                item->setFont(item_font);
                int cc = lipid_index[lipid_space->lipidomes[r]->species[c]];
                t->setItem(r, cc, item);
                free_cells.remove(r * C + cc);
            }
        }
        // fill the empty fields with disabled cells
        for (auto pos : free_cells){
            item = new QTableWidgetItem("");
            item->setFont(item_font);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setData(1, "");
            int r = pos / C;
            int c = pos % C;
            t->setItem(r, c, item);
        }
    }
    t->resizeColumnsToContents();
    t->resizeRowsToContents();
}


HomeItem::HomeItem(QGraphicsView *v) : view(v) {
}

HomeItem::~HomeItem(){
}


void HomeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QLinearGradient gradient;
    double factor = min((double)view->width() / 1207., (double)view->height() / 483.);

    painter->setRenderHint(QPainter::Antialiasing, true);


    gradient.setStart(0, 0);
    gradient.setFinalStop(0, 1);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(0.04, Qt::white);
    gradient.setColorAt(0.08, QColor("#e4eff7")); // e4eff7 / f7f0e4
    gradient.setColorAt(0.6, QColor("#6babce")); // 6babce / ceab6b
    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    painter->drawRect(0, 0, view->width(), 400 * factor);



    double l_banner = 502.5 * factor;

    QBrush bottom(QColor("#6babce")); // 6babce / ceab6b
    painter->setPen(Qt::NoPen);
    painter->setBrush(bottom);
    painter->drawRect(0., 399. * factor, view->width(), view->height());


    QBrush banner(QColor("#71a9cc")); // 71a9cc / ce956b
    painter->setBrush(banner);
    painter->drawRect(QRectF(0., 90. * factor, 50. * factor, 70. * factor));
    painter->drawRect(QRectF(l_banner + 70. * factor, 90. * factor, view->width(), 70. * factor));


    painter->save();
    painter->translate(QPointF(60. * factor, 190 * factor));
    painter->scale(factor, factor);
    QFont f = painter->font();
    painter->setPen(QPen(Qt::white));
    f.setPointSizeF(11.);
    painter->setFont(f);
    painter->drawText(QRectF(0., 0., 1000., 400.), Qt::AlignTop | Qt::AlignLeft, "Accessing the chemical space of individual lipidomes:\n  • Comprehensive study of multiple lipidomes\n  • Providing mechanism for quality control\n  • Feature analysis and lipid selection\n  • Integration of study variables");

    painter->drawText(QRectF(0, 120., 1000., 400.), Qt::AlignTop | Qt::AlignLeft, "LipidSpace offers several interactive tutorials for an easy introduction\ninto its functionality:");
    painter->restore();
}

QRectF HomeItem::boundingRect() const {
    return QRectF(0, 0, view->width(), view->height());
}

