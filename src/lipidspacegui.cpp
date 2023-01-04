#include "lipidspace/lipidspacegui.h"

DragLayer::DragLayer(LipidSpaceGUI *_lipid_space_gui, QWidget *parent) : QWidget(parent) {
    lipid_space_gui = _lipid_space_gui;

}

void DragLayer::mousePressEvent(QMouseEvent*, Canvas *canvas){
    if (!isVisible() && GlobalData::ctrl_pressed){
        source_tile = canvas->canvas_id;
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
    /*
    if (event->key() == Qt::Key_1){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 325; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        for (int i = 1; i <= 44; ++i) ct->at(i) = StudyVariableColumnNumerical;
        ct->at(1) = StudyVariableColumnNominal;
        ct->at(42) = StudyVariableColumnNominal;
        ct->at(43) = StudyVariableColumnNominal;
        loadTable(new ImportData("examples/Sales-Extended.xlsx", "Data", COLUMN_PIVOT_TABLE, ct));
    }

    else if (event->key() == Qt::Key_2){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 296; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(3) = StudyVariableColumnNominal;
        ct->at(4) = StudyVariableColumnNominal;
        ct->at(13) = StudyVariableColumnNominal;

        ct->at(1) = StudyVariableColumnNumerical;
        ct->at(2) = StudyVariableColumnNumerical;
        ct->at(5) = StudyVariableColumnNumerical;
        ct->at(6) = StudyVariableColumnNumerical;
        ct->at(7) = StudyVariableColumnNumerical;
        ct->at(8) = StudyVariableColumnNumerical;
        ct->at(9) = StudyVariableColumnNumerical;
        ct->at(10) = StudyVariableColumnNumerical;
        ct->at(11) = StudyVariableColumnNumerical;
        ct->at(12) = StudyVariableColumnNumerical;
        loadTable(new ImportData("examples/Tablesets/Plasma-Singapore.csv", "", COLUMN_PIVOT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_3){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 10; ++i) ct->push_back(IgnoreColumn);
        ct->at(3) = LipidColumn;
        ct->at(7) = QuantColumn;
        ct->at(4) = StudyVariableColumnNominal;
        ct->at(5) = StudyVariableColumnNominal;
        ct->at(0) = SampleColumn;

        loadTable(new ImportData("Platelets_Peng.csv", "", FLAT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_4){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 392; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(1) = StudyVariableColumnNominal;
        ct->at(2) = StudyVariableColumnNominal;
        ct->at(3) = StudyVariableColumnNominal;

        loadTable(new ImportData("Platelets_Peng_WT-vs-KO.csv", "", COLUMN_PIVOT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_5){
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 10; ++i) ct->push_back(IgnoreColumn);
        ct->at(1) = LipidColumn;
        ct->at(8) = QuantColumn;
        ct->at(3) = SampleColumn;
        ct->at(2) = StudyVariableColumnNominal;

        loadTable(new ImportData("Heart_GPL.csv", "", FLAT_TABLE, ct));
    }
    else if (event->key() == Qt::Key_6){


        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 297; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(1) = StudyVariableColumnNominal;
        ct->at(2) = StudyVariableColumnNominal;
        ct->at(3) = StudyVariableColumnNominal;
        loadTable(new ImportData("experiment/Ishikawa-et-al_2014.xlsx", "Data", COLUMN_PIVOT_TABLE, ct), false);


        vector<TableColumnType> *ct2 = new vector<TableColumnType>();
        for (int i = 0; i < 286; ++i) ct2->push_back(LipidColumn);
        ct2->at(0) = SampleColumn;
        ct2->at(1) = StudyVariableColumnNominal;
        ct2->at(2) = StudyVariableColumnNominal;
        ct2->at(3) = StudyVariableColumnNominal;
        loadTable(new ImportData("experiment/Saw-et-al_2017.csv", "", COLUMN_PIVOT_TABLE, ct2), false);

        vector<TableColumnType> *ct3 = new vector<TableColumnType>();
        for (int i = 0; i < 285; ++i) ct3->push_back(LipidColumn);
        ct3->at(0) = SampleColumn;
        ct3->at(1) = StudyVariableColumnNominal;
        ct3->at(2) = StudyVariableColumnNominal;
        ct3->at(3) = StudyVariableColumnNominal;
        loadTable(new ImportData("experiment/Sales-et-al_2016.csv", "", COLUMN_PIVOT_TABLE, ct3));

    }
    else if (event->key() == Qt::Key_7){
        string data_path = "/home/dominik/Nextcloud/Lipidomics/5. Projects/5.7 Dominik/LipidSpace/Manuscript/Figures/Fig2-Experiments/Part4-QC";
        resetAnalysis();
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 206; ++i) ct->push_back(LipidColumn);
        ct->at(3) = SampleColumn;
        ct->at(0) = StudyVariableColumnNominal;
        ct->at(1) = StudyVariableColumnNominal;
        ct->at(2) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Wolrab-et-al.xlsx", "SFC-HR", COLUMN_PIVOT_TABLE, ct), false);


        lipid_space->study_variable_values["State"].nominal_values["Pan"] = false;
        lipid_space->study_variable_values["State"].nominal_values["QC"] = false;
        lipid_space->study_variable_values["State"].nominal_values["T"] = false;
        lipid_space->study_variable_values["State"].nominal_values["T "] = false;


        vector<TableColumnType> *ct2 = new vector<TableColumnType>();
        for (int i = 0; i < 187; ++i) ct2->push_back(LipidColumn);
        ct2->at(3) = SampleColumn;
        ct2->at(0) = StudyVariableColumnNominal;
        ct2->at(1) = StudyVariableColumnNominal;
        ct2->at(2) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Wolrab-et-al.xlsx", "Shotgun-HR", COLUMN_PIVOT_TABLE, ct2), false);


        vector<TableColumnType> *ct3 = new vector<TableColumnType>();
        for (int i = 0; i < 236; ++i) ct3->push_back(LipidColumn);
        ct3->at(3) = SampleColumn;
        ct3->at(0) = StudyVariableColumnNominal;
        ct3->at(1) = StudyVariableColumnNominal;
        ct3->at(2) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Wolrab-et-al.xlsx", "Shotgun-LR", COLUMN_PIVOT_TABLE, ct3), false);


        vector<TableColumnType> *ct4 = new vector<TableColumnType>();
        for (int i = 0; i < 380; ++i) ct4->push_back(LipidColumn);
        ct4->at(3) = SampleColumn;
        ct4->at(0) = StudyVariableColumnNominal;
        ct4->at(1) = StudyVariableColumnNominal;
        ct4->at(2) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Wolrab-et-al.xlsx", "RP-LR", COLUMN_PIVOT_TABLE, ct4), false);


        vector<TableColumnType> *ct5 = new vector<TableColumnType>();
        for (int i = 0; i < 297; ++i) ct5->push_back(LipidColumn);
        ct5->at(0) = SampleColumn;
        ct5->at(1) = StudyVariableColumnNominal;
        ct5->at(2) = StudyVariableColumnNominal;
        ct5->at(3) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Ishikawa-et-al.xlsx", "RP-HR", COLUMN_PIVOT_TABLE, ct5), false);


        vector<TableColumnType> *ct6 = new vector<TableColumnType>();
        for (int i = 0; i < 285; ++i) ct6->push_back(LipidColumn);
        ct6->at(0) = SampleColumn;
        ct6->at(1) = StudyVariableColumnNominal;
        ct6->at(2) = StudyVariableColumnNominal;
        ct6->at(3) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Sales-et-al.xlsx", "Mixed", COLUMN_PIVOT_TABLE, ct6), false);


        vector<TableColumnType> *ct7 = new vector<TableColumnType>();
        for (int i = 0; i < 286; ++i) ct7->push_back(LipidColumn);
        ct7->at(0) = SampleColumn;
        ct7->at(1) = StudyVariableColumnNominal;
        ct7->at(2) = StudyVariableColumnNominal;
        ct7->at(3) = StudyVariableColumnNominal;
        loadTable(new ImportData(data_path + "/Saw-et-al.xlsx", "RP-LR", COLUMN_PIVOT_TABLE, ct7));



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
        ct->at(5) = StudyVariableColumnNominal;
        ct->at(6) = StudyVariableColumnNumerical;
        ct->at(7) = StudyVariableColumnNominal;

        loadTable(new ImportData("Bakerpanel.csv", "", COLUMN_PIVOT_TABLE, ct));
    }

    else */ if (event->key() == Qt::Key_Control){
        GlobalData::ctrl_pressed = true;
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
    select_lipidomes = new SelectLipidomes(this);
    select_lipidomes->setModal(true);
    keystrokes = "";
    selected_d_lipidomes = 0;
    knubbel = false;
    GlobalData::ctrl_pressed = false;
    tutorial = new Tutorial(this, ui->centralwidget);
    selected_tiles_mode = false;
    hovered_box_plot_lipid = "";
    lipid_for_deselect = "";
    raw_data_model = new RawDataModel(lipid_space, ui->tableView);
    ui->tableView->setModel(raw_data_model);

    statisticsBoxPlot.load_data(lipid_space, ui->statisticsBoxPlot);
    statisticsBarPlot.load_data(lipid_space, ui->statisticsBarPlot);
    statisticsHistogram.load_data(lipid_space, ui->statisticsHistogram);
    statisticsSpeciesCV.load_data(lipid_space, ui->statisticsSpeciesCV);
    statisticsROCCurve.load_data(lipid_space, ui->statisticsROCCurve);
    statisticsPCA.load_data(lipid_space, ui->statisticsPCA);

    connect(&statisticsBarPlot, &Statistics::enterLipid, this, &LipidSpaceGUI::lipidEntered);
    connect(&statisticsBarPlot, &Statistics::exitLipid, this, &LipidSpaceGUI::lipidExited);

    qRegisterMetaType<string>("string");
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    setWindowTitle(QApplication::translate("LipidSpaceGUI", ("LipidSpace - " + GlobalData::LipidSpace_version).c_str(), nullptr));


    connect(lipid_space, &LipidSpace::fileLoaded, this, &LipidSpaceGUI::updateSelectionView);
    connect(lipid_space, &LipidSpace::reassembled, this, &LipidSpaceGUI::updateSelectionView);

    connect(lipid_space, &LipidSpace::reassembled, this, &LipidSpaceGUI::checkBenford);
    connect(lipid_space, &LipidSpace::fileLoaded, this, &LipidSpaceGUI::checkBenford);

    dragLayer = new DragLayer(this, ui->centralwidget);
    dragLayer->move(0, 0);
    dragLayer->setVisible(false);
    dragLayer->setWindowFlags(Qt::FramelessWindowHint);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->speciesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->classList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->categoryList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->sampleList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->statisticsBoxPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsBarPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsHistogram->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsSpeciesCV->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsROCCurve->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->statisticsPCA->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->actionLoad_list_s, &QAction::triggered, this, &LipidSpaceGUI::openLists);
    connect(ui->actionLoad_table, &QAction::triggered, this, &LipidSpaceGUI::openTable);
    connect(ui->actionImport_mzTabM, &QAction::triggered, this, &LipidSpaceGUI::openMzTabM);
    connect(ui->actionQuit, &QAction::triggered, this, &LipidSpaceGUI::quitProgram);
    connect(ui->actionComplete_linkage_clustering, &QAction::triggered, this, &LipidSpaceGUI::setCompleteLinkage);
    connect(ui->actionAverage_linkage_clustering, &QAction::triggered, this, &LipidSpaceGUI::setAverageLinkage);
    connect(ui->actionSingle_linkage_clustering, &QAction::triggered, this, &LipidSpaceGUI::setSingleLinkage);
    connect(ui->actionAutomatically, &QAction::triggered, this, &LipidSpaceGUI::setAutomaticLayout);
    connect(ui->actionShow_quantitative_information, &QAction::triggered, this, &LipidSpaceGUI::showHideQuant);
    connect(ui->actionShow_global_lipidome, &QAction::triggered, this, &LipidSpaceGUI::showHideGlobalLipidome);
    connect(ui->actionShow_group_lipidomes, &QAction::triggered, this, &LipidSpaceGUI::showHideGroupLipidomes);
    connect(ui->actionSelection_mode_activated, &QAction::triggered, this, &LipidSpaceGUI::setSelectedTilesMode);
    connect(ui->actionTranslate, &QAction::triggered, this, &LipidSpaceGUI::toggleLipidNameTranslation);
    connect(ui->actionImport_eample_dataset, &QAction::triggered, this, &LipidSpaceGUI::openExampleDataset);
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
    connect(ui->tableView, &CBTableView::zooming, raw_data_model, &RawDataModel::updateTable);
    connect(ui->tableView, &CBTableView::cornerButtonClick, raw_data_model, &RawDataModel::transpose);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &LipidSpaceGUI::ShowTableContextMenu);
    connect(ui->studyVariableComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setStudyVariable);
    connect(ui->studyVariableComboBoxStat, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setStudyVariable);
    connect(this, &LipidSpaceGUI::studyVariableChanged, ui->dendrogramView, &Canvas::setStudyVariable);
    connect(ui->speciesList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->classList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->categoryList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &LipidSpaceGUI::studyVariableItemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &LipidSpaceGUI::studyVariableItemDoubleClicked);
    connect(ui->sampleList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->applyChangesPushButton, &QPushButton::clicked, this, &LipidSpaceGUI::runAnalysis);
    connect(ui->pieTreeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &LipidSpaceGUI::setPieTree);
    connect(ui->dendrogramHeightSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &LipidSpaceGUI::setDendrogramHeight);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBoxPlot, &Statistics::setLegendSize);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBarPlot, &Statistics::setLegendSize);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsHistogram, &Statistics::setLegendSize);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsSpeciesCV, &Statistics::setLegendSize);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsROCCurve, &Statistics::setLegendSize);
    connect(ui->legendSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsPCA, &Statistics::setLegendSize);
    connect(ui->barNumberSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsHistogram, &Statistics::setBarNumberHistogram);
    connect(ui->barNumberSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsSpeciesCV, &Statistics::setBarNumberSpeciesCV);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBoxPlot, &Statistics::setTickSize);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsBarPlot, &Statistics::setTickSize);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsHistogram, &Statistics::setTickSize);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsSpeciesCV, &Statistics::setTickSize);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsROCCurve, &Statistics::setTickSize);
    connect(ui->tickSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, &statisticsPCA, &Statistics::setTickSize);
    connect(ui->labelSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, ui->dendrogramView, &Canvas::setLabelSize);
    connect(ui->pieSizeSpinBox, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &LipidSpaceGUI::setPieSize);
    connect(ui->normalizationComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setNormalization);
    connect(ui->labelPieSize, &ClickableLabel::doubleClicked, this, &LipidSpaceGUI::setKnubbel);
    connect(ui->startAnalysisPushButton, &QPushButton::clicked, this, &LipidSpaceGUI::startFeatureAnalysis);
    connect(ui->secondaryComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::updateSecondarySorting);
    connect(ui->actionSelect_tiles, &QAction::triggered, this, &LipidSpaceGUI::openSelectLipidomes);

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
    connect(ui->statisticsSpeciesCV, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsSpeciesCV);
    connect(ui->statisticsROCCurve, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsROCCurve);
    connect(ui->statisticsPCA, &Chart::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuStatisticsPCA);

    sorting_boxes.push_back(ui->speciesComboBox);
    sorting_boxes.push_back(ui->classComboBox);
    sorting_boxes.push_back(ui->categoryComboBox);
    sorting_boxes.push_back(ui->sampleComboBox);
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderLabels({"Feature", "Filter"});

    tileLayout = AUTOMATIC;
    GlobalData::showQuant = true;
    showGroupLipidomes = false;
    showGlobalLipidome = true;
    updating = false;
    GlobalData::color_counter = 0;
    ui->frame->setVisible(false);
    table_transposed = false;

    progressbar = new Progressbar(this);
    progress = new Progress();
    lipid_space->progress = progress;
    connect(progress, &Progress::finish, progressbar, &Progressbar::finish);
    connect(progress, &Progress::set_current, progressbar, &Progressbar::set_current);
    connect(progress, &Progress::set_max, progressbar, &Progressbar::set_max);
    connect(progress, &Progress::error, progressbar, &Progressbar::errorClose);
    connect(progressbar, &Progressbar::interrupt, progress, &Progress::interrupt);
    connect(progressbar, &Progressbar::resetAnalysis, this, &LipidSpaceGUI::resetAnalysis);
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


    connect(&import_table, &ImportTable::importTable, this, (void (LipidSpaceGUI::*)(ImportData*))&LipidSpaceGUI::loadTable);
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

    select_tiles_information = new QLabel();
    select_tiles_information->setAlignment(Qt::AlignCenter);
    select_tiles_information->setText("No lipidome tile is selected. To select one or more tiles, go on: View > Selected tile(s) mode > Select tile(s).");

    updateGUI();
}



LipidSpaceGUI::~LipidSpaceGUI(){
    delete ui;
    delete progress;
    delete progressbar;
    delete dragLayer;
    delete select_tiles_information;
    delete raw_data_model;
    delete tutorial;
    delete select_lipidomes;
}





void LipidSpaceGUI::openExampleDataset(){
    string file_name = QCoreApplication::applicationDirPath().toStdString() + "/examples/Example-Dataset.xlsx";
    if (!QFile::exists(file_name.c_str())){
        QMessageBox::information(this, "Import error", "It seems that the example lipidomics dataset file cannot be found on the disk. Please get in contact with the developers and send a bug report.");
        Logging::write_log("It seems that the example lipidomics dataset file cannot be found on the disk. Please get in contact with the developers and send a bug report.");
        return;
    }

    vector<TableColumnType> *ct = new vector<TableColumnType>(369, LipidColumn);
    ct->at(0) = SampleColumn;
    ct->at(1) = StudyVariableColumnNominal;
    ct->at(2) = StudyVariableColumnNominal;
    ct->at(3) = StudyVariableColumnNominal;
    loadTable(new ImportData(file_name, "Data", COLUMN_PIVOT_TABLE, ct));

}



void LipidSpaceGUI::completeFeatureAnalysis(){
    QString file_name = QFileDialog::getSaveFileName(this, "Export feature analysis table", GlobalData::last_folder, "Worksheet *.xlsx (*.xlsx);;Data Table *.csv (*.csv);;Data Table *.tsv (*.tsv)");
    if (!file_name.length()) return;

    lipid_space->process_id = 3;
    progress->reset();
    lipid_space->start();
    progressbar->exec();

    if (lipid_space->complete_feature_analysis_table.size() == 0 || lipid_space->complete_feature_analysis_lipids.size() == 0) return;


    for (int i = 0; i < ui->studyVariableComboBox->count(); ++i){
        if(ui->studyVariableComboBox->itemText(i).toStdString() == lipid_space->target_variable){
            emit ui->studyVariableComboBox->currentIndexChanged(i);
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
            for (auto kv : lipid_space->study_variable_values){
                if (kv.first == FILE_STUDY_VARIABLE_NAME) continue;
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
            for (auto kv : lipid_space->study_variable_values){
                if (kv.first == FILE_STUDY_VARIABLE_NAME) continue;
                fv.push_back(kv.first);
                wks.cell(1, col++).value() = kv.first;
            }

            for (uint r = 0; r < lipid_space->complete_feature_analysis_table.size(); ++r){
                uint col = 1;
                uint row = r + 2;
                wks.cell(row, col++).value() = fv[r];
                for (auto val : lipid_space->complete_feature_analysis_table[r]) wks.cell(row, col++).value() = val;
            }

            // all selected lipids for each study variable
            for (uint i = 0; i < fv.size(); ++i){
                wbk.addWorksheet(fv[i]);
                auto wks_sv = doc.workbook().worksheet(fv[i]);
                auto &selected_lipids = lipid_space->complete_feature_analysis_lipids[i];
                int lip = 1;
                for (auto lipid_name : selected_lipids){
                    wks_sv.cell(lip++, 1).value() = lipid_name;
                }
            }
            doc.save();
            doc.close();

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



void LipidSpaceGUI::setStudyVariable(int c){
    disconnect(ui->studyVariableComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setStudyVariable);
    disconnect(ui->studyVariableComboBoxStat, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setStudyVariable);
    disconnect(ui->secondaryComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::updateSecondarySorting);

    ui->studyVariableComboBox->setCurrentIndex(c);
    ui->studyVariableComboBoxStat->setCurrentIndex(c);
    GlobalData::gui_string_var["study_var"] = ui->studyVariableComboBox->currentText().toStdString();
    GlobalData::gui_string_var["study_var_stat"] = ui->studyVariableComboBoxStat->currentText().toStdString();
    setSecondarySorting();
    GlobalData::gui_string_var["secondary_var"] = ui->secondaryComboBox->currentText().toStdString();

    statisticsBoxPlot.updateBoxPlot();
    statisticsBarPlot.updateBarPlot();
    statisticsHistogram.updateHistogram();
    statisticsSpeciesCV.updateSpeciesCV();
    statisticsROCCurve.updateROCCurve();
    statisticsPCA.updatePCA();
    studyVariableChanged(ui->studyVariableComboBox->currentText().toStdString());

    connect(ui->studyVariableComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setStudyVariable);
    connect(ui->studyVariableComboBoxStat, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::setStudyVariable);
    connect(ui->secondaryComboBox, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &LipidSpaceGUI::updateSecondarySorting);
}




void LipidSpaceGUI::quitProgram(){
    close();
}




void LipidSpaceGUI::openTable(){
    import_table.show(lipid_space);
}



void LipidSpaceGUI::updateSecondarySorting(int){
    GlobalData::gui_string_var["secondary_var"] = ui->secondaryComboBox->currentText().toStdString();
    statisticsBoxPlot.updateBoxPlot();
    statisticsBarPlot.updateBarPlot();
    statisticsHistogram.updateHistogram();
    statisticsSpeciesCV.updateSpeciesCV();
    statisticsROCCurve.updateROCCurve();
    statisticsPCA.updatePCA();
}




void LipidSpaceGUI::setSecondarySorting(){
    ui->secondaryComboBox->clear();
    string target_variable = GlobalData::gui_string_var["study_var_stat"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished) return;

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;
    if (is_nominal){
        ui->secondaryComboBox->addItem("Selected lipid species");
        for (auto kv : lipid_space->study_variable_values){
            if (kv.second.study_variable_type == NumericalStudyVariable){
                ui->secondaryComboBox->addItem(kv.first.c_str());
            }
        }
    }
    else {
        ui->secondaryComboBox->addItem("Selected lipid species");
        for (auto kv : lipid_space->study_variable_values){
            if (target_variable != kv.first && kv.first != FILE_STUDY_VARIABLE_NAME){
                ui->secondaryComboBox->addItem(kv.first.c_str());
            }
        }
    }
}



void LipidSpaceGUI::updateSelectionView(){

    // compute sort order for the groups
    disconnect(ui->speciesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    disconnect(ui->classComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    disconnect(ui->categoryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));
    disconnect(ui->sampleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateView(int)));


    for (int i = 0; i < 4; ++i){

        sortings[i].clear();
        sortings[i].insert({ALPHABETICALLY_ASC, SortVector<string, double>()});
        sortings[i].insert({ALPHABETICALLY_DESC, SortVector<string, double>()});

        SortVector<string, double> &sorting = sortings[i][ALPHABETICALLY_ASC];
        sorting.reserve(lipid_space->selection[i].size());
        for (auto &kv : lipid_space->selection[i]) sorting.push_back({kv.first, 0});
        sorting.sort_asc();


        SortVector<string, double> &sorting_desc = sortings[i][ALPHABETICALLY_DESC];
        sorting_desc.reserve(lipid_space->selection[i].size());
        for (auto &kv : lipid_space->selection[i]) sorting_desc.push_back({kv.first, 0});
        sorting_desc.sort_desc();
    }


    // sorting lipidomes according to origin
    {
        sortings[3].insert({"Origin (ASC)", SortVector<string, double>()});
        sortings[3].insert({"Origin (DESC)", SortVector<string, double>()});

        SortVector<string, string> sorting;
        sorting.reserve(lipid_space->lipidomes.size());
        for (auto lipidome : lipid_space->lipidomes) sorting.push_back({lipidome->file_name, lipidome->cleaned_name});
        sorting.sort_asc();
        for (auto p : sorting) sortings[3]["Origin (ASC)"].push_back({p.second, 0});
        sorting.sort_desc();
        for (auto p : sorting) sortings[3]["Origin (DESC)"].push_back({p.second, 0});


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
    SortVector<string, double> &sort_species_labels = sortings[0][sorting_boxes[0]->currentText().toStdString()];
    map<string, string> &translations = lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];
    for (int i = 0; i < (int)sort_species_labels.size(); ++i){
        string lipid_name = sort_species_labels[i].first;
        ListItem *item = new ListItem(translations[lipid_name], SPECIES_ITEM, ui->speciesList, lipid_name);
        item->length = sort_species_labels[i].second;
        item->setCheckState(lipid_space->selection[0][lipid_name] ? Qt::Checked : Qt::Unchecked);
        ui->speciesList->addItem(item);
    }


    map<string, int> sort_class;
    SortVector<string, double> &sort_class_labels = sortings[1][sorting_boxes[1]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_class_labels.size(); ++i){
        sort_class.insert({sort_class_labels[i].first, i});
        string class_name = sort_class_labels[i].first;
        ListItem *item = new ListItem(class_name, CLASS_ITEM, ui->classList);
        item->setCheckState(lipid_space->selection[1][class_name] ? Qt::Checked : Qt::Unchecked);
        ui->classList->addItem(item);
    }

    map<string, int> sort_category;
    SortVector<string, double> &sort_category_labels = sortings[2][sorting_boxes[2]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_category_labels.size(); ++i){
        sort_category.insert({sort_category_labels[i].first, i});
        string category_name = sort_category_labels[i].first;
        ListItem *item = new ListItem(category_name, CATEGORY_ITEM, ui->categoryList);
        item->setCheckState(lipid_space->selection[2][category_name] ? Qt::Checked : Qt::Unchecked);
        ui->categoryList->addItem(item);
    }

    map<string, int> sort_sample;
    SortVector<string, double> &sort_sample_labels = sortings[3][sorting_boxes[3]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_sample_labels.size(); ++i){
        sort_sample.insert({sort_sample_labels[i].first, i});
        string sample_name = sort_sample_labels[i].first;
        ListItem *item = new ListItem(sample_name, SAMPLE_ITEM, ui->sampleList);
        item->setCheckState(lipid_space->selection[3][sample_name] ? Qt::Checked : Qt::Unchecked);
        ui->sampleList->addItem(item);
    }

    for (auto kv : lipid_space->study_variable_values){
        TreeItem *item = new TreeItem(0, QString(kv.first.c_str()), ui->treeWidget);
        ui->treeWidget->addTopLevelItem(item);
        if (kv.second.study_variable_type == NominalStudyVariable){
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
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &LipidSpaceGUI::studyVariableItemChanged);
    connect(ui->sampleList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);

    GlobalData::gui_string_var["species_selection"] = ui->speciesComboBox->currentText().toStdString();
}



void LipidSpaceGUI::loadTable(ImportData *import_data){
    loadTable(import_data, true);
}

void LipidSpaceGUI::loadTable(ImportData *import_data, bool start_analysis){
    bool repeat_loading = true;
    ui->normalizationComboBox->clear();
    ui->normalizationComboBox->addItem("Absolute normalization", "absolute");
    GlobalData::normalization = "absolute";
    ui->normalizationComboBox->addItem("Relative normalization", "relative");
    while (repeat_loading){
        try {
            lipid_space->load_table(import_data);
            if (start_analysis) runAnalysis();
            for (auto study_variable : lipid_space->study_variable_values){
                if (study_variable.second.study_variable_type == NumericalStudyVariable) continue;
                ui->normalizationComboBox->addItem(QString("%1 grouped normalization").arg(study_variable.first.c_str()), QVariant(study_variable.first.c_str()));
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
    lipid_space->target_variable = ui->studyVariableComboBox->currentText().toStdString();
    lipid_space->process_id = 2;

    progress->reset();
    lipid_space->start();
    progressbar->exec();
    ui->itemsTabWidget->setCurrentIndex(0);
}



void LipidSpaceGUI::checkBenford(){
    GlobalData::benford_warning = !test_benford(lipid_space->lipidomes);
}



void LipidSpaceGUI::runAnalysis(){
    string species_selection = GlobalData::gui_string_var["species_selection"];
    string study_var = GlobalData::gui_string_var["study_var"];
    string study_var_stat = GlobalData::gui_string_var["study_var_stat"];


    //lipidomes_from_previous_analysis.clear();
    //for (auto lipidome : lipid_space->selected_lipidomes) lipidomes_from_previous_analysis.insert(lipidome->cleaned_name);
    set<QString> selected_tiles;


    // clear all windows with canvases
    ui->dendrogramView->clear();
    lipid_space->analysis_finished = false;
    lipid_space->process_id = 1;
    ui->startAnalysisPushButton->setEnabled(false);
    disconnect(this, SIGNAL(transforming(QRectF)), 0, 0);
    disconnect(this, SIGNAL(updateCanvas()), 0, 0);

    for (auto canvas : canvases){
        if (canvas->marked_for_selected_view && canvas->pointSet) selected_tiles.insert(canvas->pointSet->title);
        disconnect(canvas, &Canvas::transforming, 0, 0);
        disconnect(canvas, &Canvas::showMessage, 0, 0);
        disconnect(canvas, &Canvas::mouse, 0, 0);
        disconnect(canvas, &Canvas::swappingLipidomes, 0, 0);
        disconnect(canvas, &QGraphicsView::customContextMenuRequested, 0, 0);
        disconnect(canvas, &Canvas::lipidsForSelection, 0, 0);
        delete canvas;
    }
    canvases.clear();
    updateGUI();


    // (re)start analysis
    progress->reset();
    lipid_space->start();
    progressbar->exec(); // waiting for the progress bar to finish

    if (GlobalData::benford_warning){
        QMessageBox::warning(this, "Warning", "Please be aware that your current raw data do not conform to Benfords law. For further details, please have a look in the logs (Help → Log messages).");
        Logging::write_log("Your data does not conform to Benfords law. The law says that for real datsets with empirical or measured data ranging over several orders the distribution of the first digit frequency is reciprocal, not equally distributed. Deviations from the rule may be caused by several factors:\n - The data is in low orders of magnitude range\n - The data contains only few data points\n - Incorrect data imputation was previously performed\nIn any case, please check your raw data manually in the 'Raw data table' tab for abnormalities before continuing your analysis.");
        GlobalData::benford_warning = false;
    }
    if (!lipid_space->analysis_finished) return;

    if (lipid_space->study_variable_values.size() > 1 || lipid_space->study_variable_values[FILE_STUDY_VARIABLE_NAME].nominal_values.size() > 1) ui->startAnalysisPushButton->setEnabled(true);

    if (lipid_space->global_lipidome->lipids.size() < 3 && !GlobalData::gui_num_var["dont_less_3_message"]){
        QMessageBox msg;
        msg.setWindowTitle("LipidSpace Analysis");
        msg.setWindowFlags(Qt::WindowStaysOnTopHint);
        msg.setText("Less than three lipids were taken for analysis. Therefore, no lipid spaces could be computed. However, the dendrogram and statistics view can still be investigated.");
        QCheckBox *check_box = new QCheckBox("Don't show this again");
        msg.setCheckBox(check_box);
        msg.exec();
        GlobalData::gui_num_var["dont_less_3_message"] = check_box->isChecked();
    }

    // reset parameters
    GlobalData::color_counter = 0;
    GlobalData::study_variable_counter = 0;
    GlobalData::colorMap.clear();
    GlobalData::colorMapStudyVariables.clear();

    // setup all space canvases
    for (auto lipid_class : lipid_space->global_lipidome->classes){
        if (uncontains_val(GlobalData::colorMap, lipid_class)){
            GlobalData::colorMap.insert({lipid_class, GlobalData::COLORS[GlobalData::color_counter++ % GlobalData::COLORS.size()]});
        }
    }

    ui->dendrogramView->resetDendrogram();
    int n = 0;

    // insert global lipidome canvases
    Canvas* canvas = new Canvas(lipid_space, n++, -1, ui->speciesList, GlobalSpaceCanvas, ui->centralwidget);
    if (canvas->pointSet && contains_val(selected_tiles, canvas->pointSet->title)) canvas->marked_for_selected_view = true;
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
    connect(ui->speciesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), canvas, SLOT(moveToPoint(QListWidgetItem*)));
    connect(canvas, &Canvas::lipidsForSelection, this, &LipidSpaceGUI::setLipidsForSelection);
    canvases.push_back(canvas);


    // insert group lipidomes
    for (auto &kv : lipid_space->group_lipidomes){
        if (kv.second.size() <= 1) continue;

        for (uint i = 0; i < kv.second.size(); ++i){
            Canvas* canvas = new Canvas(lipid_space, n++, i, ui->speciesList, GroupSpaceCanvas, ui->centralwidget, kv.first);
            if (canvas->pointSet && contains_val(selected_tiles, canvas->pointSet->title)) canvas->marked_for_selected_view = true;
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
            connect(canvas, &Canvas::lipidsForSelection, this, &LipidSpaceGUI::setLipidsForSelection);
            canvases.push_back(canvas);
        }
    }

    // insert single lipidomes
    for (uint i = 0; i < lipid_space->selected_lipidomes.size(); ++i){
        Canvas* canvas = new Canvas(lipid_space, n++, i, ui->speciesList, SampleSpaceCanvas, ui->centralwidget);
        if (canvas->pointSet && contains_val(selected_tiles, canvas->pointSet->title)) canvas->marked_for_selected_view = true;
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
        connect(canvas, &Canvas::lipidsForSelection, this, &LipidSpaceGUI::setLipidsForSelection);
        canvases.push_back(canvas);
    }


    ui->menuAnalysis->setEnabled(true);
    ui->menuView->setEnabled(true);
    ui->actionExport_Results->setEnabled(true);
    ui->menuClustering_strategy->setEnabled(true);
    ui->menuTile_layout->setEnabled(true);
    ui->menuSelected_tiles_mode->setEnabled(true);

    if (canvases.size() > 100 && !selected_tiles_mode){
        QMessageBox::warning(this, "Warning", QString("Since %1 lipidome spaces are registered, LipidSpace will be immediately set to 'Selected tile(s) mode' to keep performance.").arg(canvases.size()));
        ui->actionSelection_mode_activated->setChecked(true);
        setSelectedTilesMode();
    }


    // define colors of study variables
    for (auto kv : lipid_space->study_variable_values){
        if (kv.second.study_variable_type == NominalStudyVariable){
            string study_variable_prefix = kv.first + "_";
            for (auto &kv_nom : kv.second.nominal_values){
                string study_variable = study_variable_prefix + kv_nom.first;
                if (uncontains_val(GlobalData::colorMapStudyVariables, study_variable)){
                    GlobalData::colorMapStudyVariables.insert({study_variable, GlobalData::COLORS[GlobalData::study_variable_counter++ % GlobalData::COLORS.size()]});
                }
            }
        }
        else {
            string study_variable_le = kv.first + "_le";
            if (uncontains_val(GlobalData::colorMapStudyVariables, study_variable_le)){
                GlobalData::colorMapStudyVariables.insert({study_variable_le, GlobalData::COLORS[GlobalData::study_variable_counter++ % GlobalData::COLORS.size()]});
            }
            string study_variable_gr = kv.first + "_gr";
            if (uncontains_val(GlobalData::colorMapStudyVariables, study_variable_gr)){
                GlobalData::colorMapStudyVariables.insert({study_variable_gr, GlobalData::COLORS[GlobalData::study_variable_counter++ % GlobalData::COLORS.size()]});
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
    statisticsSpeciesCV.updateSpeciesCV();
    statisticsROCCurve.updateROCCurve();
    statisticsPCA.updatePCA();

    int pos = ui->speciesComboBox->findText(species_selection.c_str());
    if (pos >= 0) ui->speciesComboBox->setCurrentIndex(pos);
    pos = ui->studyVariableComboBox->findText(study_var.c_str());
    if (pos >= 0) ui->studyVariableComboBox->setCurrentIndex(pos);
    pos = ui->studyVariableComboBoxStat->findText(study_var_stat.c_str());
    if (pos >= 0) ui->studyVariableComboBoxStat->setCurrentIndex(pos);
    if (ui->viewsTabWidget->currentIndex() == 0) ui->viewsTabWidget->setCurrentIndex(2);

    // reset splitters for statistics tile view
    int h = (double)(ui->splitterStat->height()) * 0.333;
    ui->splitterStatV1->setSizes(QList<int>{h, h, h});
    ui->splitterStatV2->setSizes(QList<int>{h, h, h});
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


void LipidSpaceGUI::studyVariableItemChanged(QTreeWidgetItem *item, int col){
    if (item == 0) return;
    TreeItem *tree_item = (TreeItem*)item;
    string study_variable = tree_item->study_variable;
    if (study_variable.length() == 0) return;
    if (contains_val(lipid_space->study_variable_values, study_variable)){
        string study_variable_value = item->text(col).toStdString();
        if (contains_val(lipid_space->study_variable_values[study_variable].nominal_values, study_variable_value)){
            lipid_space->study_variable_values[study_variable].nominal_values[study_variable_value] = (item->checkState(col) == Qt::Checked);
        }
        else {
            Logging::write_log("Error: Study variable value element '" + study_variable_value + "' was not found in the study variable values map.");
            QMessageBox::critical(this, "Damn it, error", "Oh no, when you read this, an error happened that should never be expected to happen. Please check the log messages and send them to the developers. Thank you and sorry.");
        }
    }
    else {
        Logging::write_log("Error: study variable selection element '" + study_variable + "' was not found in the study variable seletion map.");
        QMessageBox::critical(this, "Damn it, error", "Oh no, when you read this, an error happened that should never be expected to happen. Please check the log messages and send them to the developers. Thank you and sorry.");
    }
}



void LipidSpaceGUI::setLipidsForSelection(vector<string> &list){
    lipids_for_selection.clear();
    for (auto s : list) lipids_for_selection.push_back(s);
}


void LipidSpaceGUI::studyVariableItemDoubleClicked(QTreeWidgetItem *item, int){
    if (item == 0) return;
    TreeItem *tree_item = (TreeItem*)item;
    if (tree_item->study_variable.length() != 0) return;
    string study_variable = tree_item->text(0).toStdString();
    if (contains_val(lipid_space->study_variable_values, study_variable)){
        if (lipid_space->study_variable_values[study_variable].study_variable_type == NumericalStudyVariable){
            FilterDialog fd(lipid_space->study_variable_values[study_variable].numerical_filter);
            fd.setModal(true);
            fd.exec();
            updateSelectionView();
        }
    }
    else {
        Logging::write_log("Error: study variable selection element '" + study_variable + "' was not found in the study variable seletion map.");
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



void LipidSpaceGUI::swapLipidomes(int source, int target){
    if (source == target) return;
    if (target < source) swap(source, target);

    swap(canvases[source]->canvas_id, canvases[target]->canvas_id);
    swap(canvases[source], canvases[target]);

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


void LipidSpaceGUI::showHideGroupLipidomes(){
    showGroupLipidomes = ui->actionShow_group_lipidomes->isChecked();
    updateGUI();
}

void LipidSpaceGUI::setSelectedTilesMode(){
    selected_tiles_mode = ui->actionSelection_mode_activated->isChecked();
    if (!selected_tiles_mode && canvases.size() > 100){
        if (QMessageBox::question(this, "Warning", QString("Currently %1 lipidome spaces are available. Showing all will reduce performance significantly. Do you want to continue?").arg(canvases.size())) != QMessageBox::Yes){
            selected_tiles_mode = true;
            GlobalData::selected_view = selected_tiles_mode;
            ui->actionSelection_mode_activated->setChecked(true);
        }
        else {
            GlobalData::selected_view = selected_tiles_mode;
            updateGUI();
        }
    }
    else {
        GlobalData::selected_view = selected_tiles_mode;
        updateGUI();
    }
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


void LipidSpaceGUI::openSelectLipidomes(){
    select_lipidomes->init();
    select_lipidomes->exec();
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
    ui->dendrogramView->updateDendrogram();
}


void LipidSpaceGUI::setNormalization(int){
    GlobalData::normalization = ui->normalizationComboBox->currentData().toString().toStdString();
}


void LipidSpaceGUI::setDendrogramHeight(int height){
    GlobalData::gui_num_var["dendrogram_height"] = height;
    ui->dendrogramView->setDendrogramHeight();
}


void LipidSpaceGUI::setPieSize(int size){
    GlobalData::gui_num_var["pie_size"] = size;
    ui->dendrogramView->updateDendrogram();
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
    emit resizing();
    event->ignore();
}


void LipidSpaceGUI::updateGUI(){
    updating = true;

    QTabBar *bar = ui->viewsTabWidget->tabBar();
    for (int t = 1; t <= 4; ++t) bar->setTabEnabled(t, lipid_space->lipidomes.size() > 0);

    ui->studyVariableComboBox->clear();
    ui->studyVariableComboBoxStat->clear();

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
    while ((wItem = ui->gridLayout->layout()->takeAt(0)) != 0) {
        delete wItem;
    };

    if (!lipid_space->analysis_finished || !canvases.size()) return;


    for (auto canvas : canvases) canvas->setVisible(false);
    int numTiles = 0;
    int num_lipidomes = lipid_space->selected_lipidomes.size();

    if (selected_tiles_mode){
        for (auto canvas : canvases) numTiles += canvas->marked_for_selected_view;
    }
    else {
        numTiles = (lipid_space->selected_lipidomes.size() > 1 && showGlobalLipidome) + num_lipidomes;
        if (showGroupLipidomes){
            for (auto &kv : lipid_space->group_lipidomes){
                numTiles += kv.second.size() * (kv.second.size() > 1);
            }
        }
    }

    int tileColumns = tileLayout == AUTOMATIC ? ceil(sqrt((double)numTiles)) : (int)tileLayout;
    int c = 0, r = 0, n = 0;

    // show global lipidome if enabled
    for (auto canvas : canvases) {
        if (selected_tiles_mode && (!selected_tiles_mode || !canvas->marked_for_selected_view)) continue;

        if ((selected_tiles_mode && canvas->marked_for_selected_view) ||
            (canvas->canvas_type == GlobalSpaceCanvas && showGlobalLipidome && num_lipidomes > 1) ||
            (canvas->canvas_type == GroupSpaceCanvas && showGroupLipidomes) ||
            canvas->canvas_type == SampleSpaceCanvas){
            ui->gridLayout->addWidget(canvas, r, c);
            if (++c == tileColumns){
                c = 0;
                ++r;
            }
            canvas->setVisible(true);
        }
        ++n;
    }

    if (selected_tiles_mode && ui->gridLayout->count() == 0){
        select_tiles_information->setVisible(true);
        ui->gridLayout->addWidget(select_tiles_information, 0, 0);
    }
    else {
        select_tiles_information->setVisible(false);
    }


    for (auto kv : lipid_space->study_variable_values){
        ui->studyVariableComboBox->addItem(kv.first.c_str());
        ui->studyVariableComboBoxStat->addItem(kv.first.c_str());
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
    if (lipid_space->study_variable_values.size() > 1){
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

    QTableView *t = ui->tableView;
    QItemSelectionModel *select = t->selectionModel();
    if (!select->hasSelection() || select->selectedIndexes().size() == 0) return;

    int top_row = select->selectedIndexes().first().row();
    int bottom_row = select->selectedIndexes().last().row();
    int left_column = select->selectedIndexes().first().column();
    int right_column = select->selectedIndexes().last().column();

    QString str;
    for (int r = top_row; r <= bottom_row; ++r){
        for (int c = left_column; c <= right_column; ++c){

            QVariant v = raw_data_model->getData(r, c);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (v.typeId() == QMetaType::Double) QTextStream(&str) << QString::number(v.toDouble());
            else if (v.typeId() == QMetaType::QString) QTextStream(&str) << v.toString();
#else
            if (v.type() == QVariant::Double) QTextStream(&str) << QString::number(v.toDouble());
            else if (v.type() == QVariant::String) QTextStream(&str) << v.toString();
#endif
            if (c < right_column) QTextStream(&str) << "\t";
        }
        QTextStream(&str) << "\n";
    }

    QApplication::clipboard()->setText(str);
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




void LipidSpaceGUI::lipidEntered(string _lipid_name){
    hovered_box_plot_lipid = _lipid_name;
}


void LipidSpaceGUI::lipidExited(){
    hovered_box_plot_lipid = "";
}



void LipidSpaceGUI::ShowContextMenuLipidome(Canvas *canvas, QPoint pos){
    if (GlobalData::in_tutorial) return;

    lipids_for_selection_menu.clear();
    for (auto l : lipids_for_selection) lipids_for_selection_menu.push_back(l);

    QMenu *menu = new QMenu(this);
    if (lipids_for_selection_menu.size()){
        QMenu *action_selectLipids = new QMenu(menu);
        action_selectLipids->setTitle("Select");
        QMenu *action_deselectLipids = new QMenu(menu);
        action_deselectLipids->setTitle("Deselect");

        QAction *action_select_all = new QAction("All hovered", this);
        action_selectLipids->addAction(action_select_all);
        connect(action_select_all, &QAction::triggered, [=](){ spaceLipidsSelect(-1, true); });

        QAction *action_deselect_all = new QAction("All hovered", this);
        action_deselectLipids->addAction(action_deselect_all);
        connect(action_deselect_all, &QAction::triggered, [=](){ spaceLipidsSelect(-1, false); });


        for(int i = 0; i < (int)lipids_for_selection_menu.size(); ++i){
            auto l = lipids_for_selection_menu[i];
            QAction *action_select = new QAction(l.c_str(), this);
            action_selectLipids->addAction(action_select);
            connect(action_select, &QAction::triggered, [=](){ spaceLipidsSelect(i, true); });

            QAction *action_deselect = new QAction(l.c_str(), this);
            action_deselectLipids->addAction(action_deselect);
            connect(action_deselect, &QAction::triggered, [=](){ spaceLipidsSelect(i, false); });
        }

        menu->addAction(action_selectLipids->menuAction());
        menu->addAction(action_deselectLipids->menuAction());
    }
    QAction *exportAsPdf = new QAction("Export as pdf", this);
    menu->addAction(exportAsPdf);
    menu->popup(canvas->viewport()->mapToGlobal(pos));
    connect(exportAsPdf, &QAction::triggered, canvas, &Canvas::exportAsPdf);
}



void LipidSpaceGUI::spaceLipidsSelect(int index, bool do_select){
    if (index < 0){
        for (auto l : lipids_for_selection_menu){
            if (contains_val(lipid_space->selection[0], l)){
                lipid_space->selection[0][l] = do_select;
            }
        }
    }
    else if (index < (int)lipids_for_selection_menu.size()) {
        auto l = lipids_for_selection_menu[index];
        if (contains_val(lipid_space->selection[0], l)){
            lipid_space->selection[0][l] = do_select;
        }
    }

    updateSelectionView();
    lipids_for_selection_menu.clear();
}



void LipidSpaceGUI::ShowTableContextMenu(const QPoint pos){
    QMenu *menu = new QMenu(this);
    QAction *copyToClipboard = new QAction("Copy selection", this);
    QAction *exportTable = new QAction("Export complete Table", this);
    menu->addAction(copyToClipboard);
    menu->addAction(exportTable);

    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
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


        int cols = raw_data_model->cols;
        int rows = raw_data_model->rows;

        wks_data.cell(1, 1).value() = "Sample";
        for (int col = 0; col < cols; ++col) wks_data.cell(1, 2 + col).value() = raw_data_model->column_headers[col].toStdString();
        for (int row = 0; row < rows; ++row) wks_data.cell(2 + row, 1).value() = raw_data_model->row_headers[row].toStdString();

        for (int col = 0; col < cols; ++col){
            for (int row = 0; row < rows; ++row){
                QVariant variant = raw_data_model->getData(row, col);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                if (variant.typeId() == QMetaType::Double){
#else
                if (variant.type() == QVariant::Double){
#endif
                    wks_data.cell(2 + row, 2 + col).value() = variant.toDouble();
                }
                else {
                    wks_data.cell(2 + row, 2 + col).value() = variant.toString().toStdString();
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


void LipidSpaceGUI::ShowContextMenuStatisticsPCA(const QPoint pos){
    if (statisticsPCA.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    connect(actionData, &QAction::triggered, &statisticsPCA, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsPCA, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsPCA->viewport()->mapToGlobal(pos));
}


void LipidSpaceGUI::ShowContextMenuStatisticsBarPlot(const QPoint pos){
    if (statisticsBarPlot.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionLogScale = new QAction("Y-axis in log scale", this);
    QAction *actionSelectLipid = 0;

    map<string, string> &translations = lipid_space->lipid_name_translations[NORMALIZED_NAME];
    bool hover_over_lipid = hovered_box_plot_lipid.length() > 0 && contains_val(translations, hovered_box_plot_lipid) && contains_val(lipid_space->selection[0], translations[hovered_box_plot_lipid]);

    if (hover_over_lipid){
        lipid_for_deselect = translations[hovered_box_plot_lipid];
        bool deselect = lipid_space->selection[0][lipid_for_deselect];
        actionSelectLipid = new QAction(QString("%1elect %2").arg(deselect ? "Des" : "S").arg(hovered_box_plot_lipid.c_str()), this);
    }

    QAction *actionPValues = new QAction("Show statistical results", this);
    QAction *actionShowData = new QAction("Show data", this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    actionLogScale->setCheckable(true);
    actionLogScale->setChecked(statisticsBarPlot.log_scale);
    actionPValues->setCheckable(true);
    actionPValues->setChecked(statisticsBarPlot.show_pvalues);
    actionShowData->setCheckable(true);
    actionShowData->setChecked(statisticsBarPlot.show_data);
    if (actionSelectLipid) menu->addAction(actionSelectLipid);
    menu->addAction(actionLogScale);
    menu->addAction(actionPValues);
    menu->addAction(actionShowData);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    if (actionSelectLipid) connect(actionSelectLipid, &QAction::triggered, this, &LipidSpaceGUI::deselectHoveredLipid);
    connect(actionPValues, &QAction::triggered, &statisticsBarPlot, &Statistics::setStatResults);
    connect(actionLogScale, &QAction::triggered, &statisticsBarPlot, &Statistics::setLogScaleBarPlot);
    connect(actionShowData, &QAction::triggered, &statisticsBarPlot, &Statistics::setShowDataBarPlot);
    connect(actionData, &QAction::triggered, &statisticsBarPlot, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsBarPlot, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsBarPlot->viewport()->mapToGlobal(pos));
}



void LipidSpaceGUI::deselectHoveredLipid(){
    if (lipid_for_deselect.length() > 0 && contains_val(lipid_space->selection[0], lipid_for_deselect)){
        lipid_space->selection[0][lipid_for_deselect] = !lipid_space->selection[0][lipid_for_deselect];
        updateSelectionView();
        lipid_for_deselect = "";
    }
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



void LipidSpaceGUI::ShowContextMenuStatisticsSpeciesCV(const QPoint pos){
    if (statisticsSpeciesCV.chart->chart_plots.size() == 0) return;
    QMenu *menu = new QMenu(this);
    QAction *actionData = new QAction("Export data", this);
    QAction *actionExportPdf = new QAction("Export as pdf", this);
    menu->addAction(actionData);
    menu->addAction(actionExportPdf);
    connect(actionData, &QAction::triggered, &statisticsSpeciesCV, &Statistics::exportData);
    connect(actionExportPdf, &QAction::triggered, &statisticsSpeciesCV, &Statistics::exportAsPdf);
    menu->popup(ui->statisticsSpeciesCV->viewport()->mapToGlobal(pos));
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
        connect(actionSelectAll, &QAction::triggered, this, [=](){ check_all_entities(true); });
        connect(actionDeselectAll, &QAction::triggered, this, [=](){ check_all_entities(false); });
        connect(actionToggleAll, &QAction::triggered, this, [=](){ toggle_entities(true); });
        connect(actionSelectSelected, &QAction::triggered, this, [=](){ check_selected_entities(true); });
        connect(actionDeselectSelected, &QAction::triggered, this, [=](){ check_selected_entities(false); });
        connect(actionToggleSelected, &QAction::triggered, this, [=](){ toggle_entities(false); });
        connect(actionExportList, &QAction::triggered, this, &LipidSpaceGUI::export_list);
    }
    else {
        QAction *actionSelectAll = new QAction("Select all nominal study variables", this);
        QAction *actionDeselectAll = new QAction("Deselect all nominal study variables", this);
        QAction *actionResetAll = new QAction("Reset all numerical study variables", this);
        menu->addAction(actionSelectAll);
        menu->addAction(actionDeselectAll);
        menu->addAction(actionResetAll);
        menu->popup(ui->treeWidget->viewport()->mapToGlobal(pos));
        connect(actionSelectAll, &QAction::triggered, this, [=](){ select_all_study_variables(true); });
        connect(actionDeselectAll, &QAction::triggered, this, [=](){ select_all_study_variables(false); });
        connect(actionResetAll, &QAction::triggered, this, &LipidSpaceGUI::reset_all_study_variables);
    }
}



void LipidSpaceGUI::check_all_entities(bool checked){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (int i = 0; i < widget->count(); ++i){
        widget->item(i)->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}



void LipidSpaceGUI::check_selected_entities(bool checked){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    for (auto item : widget->selectedItems()){
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}



void LipidSpaceGUI::toggle_entities(bool all_entities){
    QListWidget *widget = nullptr;
    switch(ui->itemsTabWidget->currentIndex()){
        case 0: widget = ui->speciesList; break;
        case 1: widget = ui->classList; break;
        case 2: widget = ui->categoryList; break;
        case 3: return;
        case 4: widget = ui->sampleList; break;
    }
    if (all_entities){
        for (int i = 0; i < widget->count(); ++i){
            widget->item(i)->setCheckState(widget->item(i)->checkState() ? Qt::Unchecked :  Qt::Checked);
        }
    }
    else {
        for (auto item : widget->selectedItems()){
            item->setCheckState(item->checkState() ? Qt::Unchecked :  Qt::Checked);
        }
    }
}



void LipidSpaceGUI::select_all_study_variables(bool select){
    vector<QTreeWidgetItem*> stack;
    stack.push_back(ui->treeWidget->invisibleRootItem());
    while(stack.size() > 0){
        TreeItem *item = (TreeItem*)stack.back();
        stack.pop_back();
        if (item->study_variable != "") item->setCheckState(0, select ? Qt::Checked : Qt::Unchecked);
        for (int i = 0; i < (int)item->childCount(); ++i) stack.push_back(item->child(i));
    }
}


void LipidSpaceGUI::reset_all_study_variables(){
    for (auto &kv : lipid_space->study_variable_values){
        kv.second.numerical_filter.first = NoFilter;
        kv.second.numerical_filter.second.clear();
    }
    updateView(0);
}



void LipidSpaceGUI::fill_table(){
    raw_data_model->reload();
    raw_data_model->updateTable();
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







RawDataModel::RawDataModel(LipidSpace *_lipid_space, QObject *parent) : QAbstractTableModel(parent) {
    lipid_space = _lipid_space;
    rows = 0;
    cols = 0;
    transposed = false;
}



void RawDataModel::reload(){
    raw_data.clear();
    column_headers.clear();
    row_headers.clear();
    present_columns.clear();
    present_rows.clear();
    column_sizes.clear();
    row_sizes.clear();
    cols = 0;
    rows = 0;
    transposed = false;

    if (!lipid_space->analysis_finished) return;

    cols = lipid_space->all_lipids.size() + lipid_space->study_variable_values.size();
    rows = lipid_space->lipidomes.size();
    raw_data.resize(rows, vector<QVariant>(cols));
    column_sizes.resize(cols, 1);
    row_headers.resize(rows);
    row_sizes.resize(rows);


    map<string, string> &translations = lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];
    set<string> present_lipids;
    set<string> present_lipidomes;
    map<string, int> column_map;

    // getting header data for study variables
    for (auto kv : lipid_space->study_variable_values){
        present_columns.insert(column_headers.size());
        column_headers.push_back(kv.first.c_str());
        column_map.insert({kv.first, column_map.size()});
    }


    for (auto lipid_name : lipid_space->global_lipidome->species) present_lipids.insert(lipid_name);
    for (auto lipidome : lipid_space->selected_lipidomes) present_lipidomes.insert(lipidome->cleaned_name);

    // getting header data for lipids
    for (auto kv : lipid_space->all_lipids){
        column_map.insert({kv.first, column_map.size()});
        if (contains_val(present_lipids, kv.first)) present_columns.insert(column_headers.size());
        column_headers.push_back(translations[kv.first].c_str());
    }


    QFontMetrics fm(QFont("Helvetica", 10));

    // getting header data for lipidomes and data
    #pragma omp parallel for
    for (uint r = 0; r < lipid_space->lipidomes.size(); ++r){
        auto lipidome = lipid_space->lipidomes[r];
        row_headers[r] = lipidome->cleaned_name.c_str();
        row_sizes[r] = __max(row_sizes[r], (double)fm.horizontalAdvance(row_headers[r]));

        #pragma omp critical
        {
            if (contains_val(present_lipidomes, lipidome->cleaned_name)) present_rows.insert(r);
        }

        for (auto kv : lipidome->study_variables){
            if (!kv.second.missing && contains_val(column_map, kv.first)){
                int c = column_map[kv.first];
                if (kv.second.study_variable_type == NumericalStudyVariable){
                    raw_data[r][c] = kv.second.numerical_value;
                }
                else if (kv.second.study_variable_type == NominalStudyVariable){
                    raw_data[r][c] = kv.second.nominal_value.c_str();
                    column_sizes[c] = __max(column_sizes[c], (double)fm.horizontalAdvance(raw_data[r][c].toString()));
                }
            }
        }

        for (uint i = 0; i < lipidome->species.size(); ++i){
            int c = column_map[lipidome->species[i]];
            raw_data[r][c] = lipidome->original_intensities[i];
        }
    }

    for (uint c = 0; c < column_headers.size(); c++){
        column_sizes[c] = __max(column_sizes[c], (double)fm.horizontalAdvance(column_headers[c]));
    }

    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rowCount() - 1, columnCount() - 1);

    emit dataChanged(topLeft, bottomRight);
    emit layoutChanged();
}


int RawDataModel::rowCount(const QModelIndex &) const {
    return rows;
}


int RawDataModel::columnCount(const QModelIndex &) const {
    return cols;
}


QVariant RawDataModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == Qt::ForegroundRole){
        return (contains_val(present_rows, index.row()) && contains_val(present_columns, index.column())) ? QColor(Qt::black) : QColor(180, 180, 180);
    }
    else if (role == Qt::DisplayRole) {
        return transposed ? raw_data[index.column()][index.row()] : raw_data[index.row()][index.column()];
    }
    else if (role == Qt::FontRole){
        return QFont("Helvetica", GlobalData::gui_num_var["table_zoom"]);
    }
    return QVariant();
}


QVariant RawDataModel::getData(int row, int col){
    return transposed ? raw_data[col][row] : raw_data[row][col];
}


void RawDataModel::transpose(){
    swap(column_headers, row_headers);
    swap(present_columns, present_rows);
    swap(cols, rows);
    swap(column_sizes, row_sizes);
    transposed = !transposed;

    updateTable();
}

void RawDataModel::updateTable(){

    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rowCount() - 1, columnCount() - 1);

    emit dataChanged(topLeft, bottomRight);
    emit layoutChanged();

    QTableView *t = (QTableView *)parent();
    double font_zoom = (double)GlobalData::gui_num_var["table_zoom"] / 7.5;
    for (int i = 0; i < rows; ++i) t->setRowHeight(i, GlobalData::gui_num_var["table_zoom"] * 2);
    for (int i = 0; i < cols; ++i) t->setColumnWidth(i, (double)column_sizes[i] * font_zoom);
}



QVariant RawDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (!lipid_space->analysis_finished) return QVariant();

    if (role == Qt::ForegroundRole){
        if (orientation == Qt::Horizontal && contains_val(present_columns, section)) return QColor(Qt::black);
        if (orientation == Qt::Vertical && contains_val(present_rows, section)) return QColor(Qt::black);
        return QColor(128, 128, 128);
    }

    if (role == Qt::DisplayRole){
        if (orientation == Qt::Horizontal && (int)column_headers.size() > section) {
            return column_headers[section];
        }
        if (orientation == Qt::Vertical && (int)row_headers.size() > section) {
            return row_headers[section];
        }
    }
    else if (role == Qt::FontRole){
        return QFont("Helvetica", GlobalData::gui_num_var["table_zoom"]);
    }
    return QVariant();
}
