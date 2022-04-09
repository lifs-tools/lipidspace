#include "lipidspace/lipidspacegui.h"


DragLayer::DragLayer(QWidget *parent) : QWidget(parent) {
    
}

void DragLayer::mousePressEvent(QMouseEvent*, Canvas *canvas){
    if (!isVisible() && canvas->num >= 0){
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



void LipidSpaceGUI::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_1 && !loadedDataSet){
        loadedDataSet = true;
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 12; ++i) ct->push_back(IgnoreColumn);
        ct->at(1) = LipidColumn;
        ct->at(6) = SampleColumn;
        ct->at(7) = FeatureColumnNominal;
        ct->at(8) = FeatureColumnNominal;
        ct->at(10) = FeatureColumnNumerical;
        ct->at(11) = QuantColumn;
        loadTable("Anxa7_pivot.csv", ct, PIVOT_TABLE);
        
        /*
        for (int i = 0; i < 32; ++i) ct->push_back(SampleColumn);
        ct->at(0) = LipidColumn;
        loadTable("examples/Tablesets/Plasma-Liebisch.csv", ct, ROW_TABLE);
        */
        
        
    }
    else if (event->key() == Qt::Key_2 && !loadedDataSet){
        loadedDataSet = true;
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 295; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(3) = FeatureColumnNominal;
        ct->at(4) = FeatureColumnNominal;
        ct->at(12) = FeatureColumnNominal;
        ct->at(1) = FeatureColumnNumerical;
        ct->at(2) = FeatureColumnNumerical;
        ct->at(5) = FeatureColumnNumerical;
        ct->at(6) = FeatureColumnNumerical;
        ct->at(7) = FeatureColumnNumerical;
        ct->at(8) = FeatureColumnNumerical;
        ct->at(9) = FeatureColumnNumerical;
        ct->at(10) = FeatureColumnNumerical;
        ct->at(11) = FeatureColumnNumerical;
        loadTable("examples/Tablesets/Plasma-Singapore.csv", ct, COLUMN_TABLE);
    }
    else if (event->key() == Qt::Key_3 && !loadedDataSet){
        loadedDataSet = true;
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 12; ++i) ct->push_back(IgnoreColumn);
        ct->at(11) = LipidColumn;
        ct->at(7) = QuantColumn;
        ct->at(4) = SampleColumn;
        ct->at(9) = SampleColumn;
        
        loadTable("blood.csv", ct, PIVOT_TABLE);
    }
}



LipidSpaceGUI::LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent) : QMainWindow(parent), timer(this) {
    lipid_space = _lipid_space;
    ui = new Ui::LipidSpaceGUI();
    ui->setupUi(this);
    loadedDataSet = false;
    
    connect(lipid_space, SIGNAL(fileLoaded()), this, SLOT(updateSelectionView()));
    connect(lipid_space, SIGNAL(reassembled()), this, SLOT(updateSelectionView()));
    
    dragLayer = new DragLayer(ui->centralwidget);
    dragLayer->move(0, 0);
    dragLayer->setVisible(false);
    dragLayer->setWindowFlags(Qt::FramelessWindowHint);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    ui->speciesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    connect(ui->actionLoad_list_s, SIGNAL(triggered()), this, SLOT(openLists()));
    connect(ui->actionLoad_table, SIGNAL(triggered()), this, SLOT(openTable()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quitProgram()));
    
    connect(ui->actionAutomatically, &QAction::triggered, this, &LipidSpaceGUI::setAutomaticLayout);
    connect(ui->actionShow_quantitative_information, &QAction::triggered, this, &LipidSpaceGUI::showHideQuant);
    connect(ui->actionShow_global_lipidome, &QAction::triggered, this, &LipidSpaceGUI::showHideGlobalLipidome);
    connect(ui->actionShow_dendrogram, &QAction::triggered, this, &LipidSpaceGUI::showHideDendrogram);
    connect(ui->action1_column, &QAction::triggered, this, &LipidSpaceGUI::set1ColumnLayout);
    connect(ui->action2_columns, &QAction::triggered, this, &LipidSpaceGUI::set2ColumnLayout);
    connect(ui->action3_columns, &QAction::triggered, this, &LipidSpaceGUI::set3ColumnLayout);
    connect(ui->action4_columns, &QAction::triggered, this, &LipidSpaceGUI::set4ColumnLayout);
    connect(ui->action5_columns, &QAction::triggered, this, &LipidSpaceGUI::set5ColumnLayout);
    connect(ui->action6_columns, &QAction::triggered, this, &LipidSpaceGUI::set6ColumnLayout);
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
    connect(ui->featureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setFeature(int)));
    connect(this, SIGNAL(featureChanged(string)), ui->dendrogramView, SLOT(setFeature(string)));
    connect(ui->speciesList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->classList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->categoryList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &LipidSpaceGUI::featureItemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &LipidSpaceGUI::featureItemDoubleClicked);
    connect(ui->sampleList, &QListWidget::itemChanged, this, &LipidSpaceGUI::itemChanged);
    connect(ui->applyChangesPushButton, &QPushButton::clicked, this, &LipidSpaceGUI::runAnalysis);
    connect(ui->pieTreeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setPieTree(int)));
    connect(ui->dendrogramHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setDendrogramHeight(int)));
    connect(ui->normalizationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setNormalization(int)));
    
    ui->speciesList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->classList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->categoryList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->sampleList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->speciesList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->classList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->categoryList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    connect(ui->sampleList, &QListWidget::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenu);
    
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
    connect(ui->dendrogramView, &QGraphicsView::customContextMenuRequested, this, &LipidSpaceGUI::ShowContextMenuDendrogram);
    
    updateGUI();
}



LipidSpaceGUI::~LipidSpaceGUI(){
    delete ui;
    delete progress;
    delete progressbar;
    delete dragLayer;
}


void LipidSpaceGUI::setFeature(int){
    featureChanged(ui->featureComboBox->currentText().toStdString());
}



void LipidSpaceGUI::quitProgram(){
    QApplication::quit();
}



void LipidSpaceGUI::openTable(){
    ImportTable it;
    connect(&it, SIGNAL(importTable(string, vector<TableColumnType>*, TableType)), this, SLOT(loadTable(string, vector<TableColumnType>*, TableType)));
    it.setModal(true);
    it.exec();
}


bool compare_string_asc(string a, string b){
    return a.compare(b) < 0;
}

bool compare_string_desc(string a, string b){
    return a.compare(b) > 0;
}

void LipidSpaceGUI::updateSelectionView(){
    
    // compute sort order for the groups
    disconnect(ui->speciesComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->classComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->categoryComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    disconnect(ui->sampleComboBox, SIGNAL(currentIndexChanged(int)), 0, 0);
    
    
    for (int i = 0; i < 4; ++i){
        
        sortings[i].clear();
        sortings[i].insert({ALPHABETICALLY_ASC, vector<string>()});
        sortings[i].insert({ALPHABETICALLY_DESC, vector<string>()});
    
        vector<string> &sorting = sortings[i][ALPHABETICALLY_ASC];
        sorting.reserve(lipid_space->selection[i].size());
        for (auto &kv : lipid_space->selection[i]) sorting.push_back(kv.first);
        sort(sorting.begin(), sorting.end(), compare_string_asc);
        
        
        vector<string> &sorting_desc = sortings[i][ALPHABETICALLY_DESC];
        sorting_desc.reserve(lipid_space->selection[i].size());
        for (auto &kv : lipid_space->selection[i]) sorting_desc.push_back(kv.first);
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
    map<string, int> sort_species;
    vector<string> &sort_species_labels = sortings[0][sorting_boxes[0]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_species_labels.size(); ++i){
        sort_species.insert({sort_species_labels[i], i});
        ListItem *item = new ListItem("", SPECIES_ITEM, ui->speciesList);
        ui->speciesList->addItem(item);
    }
    for (auto species : lipid_space->selection[0]){
        QListWidgetItem *item = ui->speciesList->item(sort_species[species.first]);
        item->setText(species.first.c_str());
        item->setCheckState(species.second ? Qt::Checked : Qt::Unchecked);
    }
    
    
    map<string, int> sort_class;
    vector<string> &sort_class_labels = sortings[1][sorting_boxes[1]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_class_labels.size(); ++i){
        sort_class.insert({sort_class_labels[i], i});
        ListItem *item = new ListItem("", CLASS_ITEM, ui->classList);
        ui->classList->addItem(item);
    }
    for (auto lipid_class : lipid_space->selection[1]){
        QListWidgetItem *item = ui->classList->item(sort_class[lipid_class.first]);
        item->setText(lipid_class.first.c_str());
        item->setCheckState(lipid_class.second ? Qt::Checked : Qt::Unchecked);
    }
    
    map<string, int> sort_category;
    vector<string> &sort_category_labels = sortings[2][sorting_boxes[2]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_category_labels.size(); ++i){
        sort_category.insert({sort_category_labels[i], i});
        ListItem *item = new ListItem("", CATEGORY_ITEM, ui->categoryList);
        ui->categoryList->addItem(item);
    }
    for (auto category : lipid_space->selection[2]){
        QListWidgetItem *item = ui->categoryList->item(sort_category[category.first]);
        item->setText(category.first.c_str());
        item->setCheckState(category.second ? Qt::Checked : Qt::Unchecked);
    }
    
    map<string, int> sort_sample;
    vector<string> &sort_sample_labels = sortings[3][sorting_boxes[3]->currentText().toStdString()];
    for (int i = 0; i < (int)sort_sample_labels.size(); ++i){
        sort_sample.insert({sort_sample_labels[i], i});
        ListItem *item = new ListItem("", SAMPLE_ITEM, ui->sampleList);
        ui->sampleList->addItem(item);
    }
    for (auto sample : lipid_space->selection[3]){
        QListWidgetItem *item = ui->sampleList->item(sort_sample[sample.first]);
        item->setText(sample.first.c_str());
        item->setCheckState(sample.second ? Qt::Checked : Qt::Unchecked);
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
}


    
void LipidSpaceGUI::loadTable(string file_name, vector<TableColumnType>* column_types, TableType table_type){
    bool repeat_loading = true;
    ui->normalizationComboBox->clear();
    ui->normalizationComboBox->addItem("Absolute normalization", "absolute");
    GlobalData::normalization = "absolute";
    ui->normalizationComboBox->addItem("Relative normalization", "relative");
    while (repeat_loading){
        try {
            switch(table_type){
                case ROW_TABLE:
                    lipid_space->load_row_table(file_name, column_types);
                    break;
                    
                case COLUMN_TABLE:
                    lipid_space->load_column_table(file_name, column_types);
                    break;
                    
                case PIVOT_TABLE:
                    lipid_space->load_pivot_table(file_name, column_types);
                    break;
            }
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
                            //resetAnalysis();
                        }
                        else {
                            //resetAnalysis();
                            repeat_loading = false;
                        }
                    }
                    break;
                
                case FileUnreadable:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        //resetAnalysis();
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
                            //resetAnalysis();
                        }
                        else {
                            //resetAnalysis();
                            repeat_loading = false;
                        }
                    }
                    break;
                    
                case NoColumnFound:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        //resetAnalysis();
                        repeat_loading = false;
                    }
                    break;
                    
                case ColumnNumMismatch:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        //resetAnalysis();
                        repeat_loading = false;
                    }
                    break;
                    
                default:
                    {
                        msgBox.setInformativeText("Please check the log message. In case, please contact the developers.");
                        msgBox.exec();
                        //resetAnalysis();
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
}



void LipidSpaceGUI::runAnalysis(){
    lipid_space->analysis_finished = false;
    disconnect(this, SIGNAL(transforming(QRectF)), 0, 0);
    disconnect(this, SIGNAL(updateCanvas()), 0, 0);
    
    progress->reset();
    lipid_space->start();
    progressbar->exec();
    
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
    
    if (!lipid_space->analysis_finished){
        ui->dendrogramView->clear();
        return;
    }
    
    ui->dendrogramHeightSpinBox->setValue(100);
    GlobalData::color_counter = 0;
    GlobalData::feature_counter = 0;
    GlobalData::colorMap.clear();
    GlobalData::colorMapFeatures.clear();
    
    for (auto lipid_class : lipid_space->global_lipidome->classes){
        if (uncontains_val(GlobalData::colorMap, lipid_class)){
            GlobalData::colorMap.insert({lipid_class, GlobalData::COLORS[GlobalData::color_counter++ % GlobalData::COLORS.size()]});
        }
    }
    int numTiles = 2 * (lipid_space->selected_lipidomes.size() > 1) + lipid_space->selected_lipidomes.size();
    ui->dendrogramView->resetDendrogram();
    
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
            connect(this, SIGNAL(initialized()), canvas, SLOT(setInitialized()));
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
        canvases.push_back(canvas);
    }
    
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
    
    fill_Table();
    ui->frame->setVisible(true);
    updateSelectionView();
    updateGUI();
    
    // dirty hack to overcome the annoying resizing calls of the canvases when
    // putting them into the grid layout. Unfortunately, I cannot figure out, when
    // it is over. So I use a timer and hope that after 200ms all rearranging is over
    QTimer::singleShot(200, this, SLOT(setInitialized()));
}


void LipidSpaceGUI::reassembleSelection(){
    lipid_space->reassembleSelection();
}


void LipidSpaceGUI::itemChanged(QListWidgetItem *item){
    if (item == 0) return;
    ListItem *list_item = (ListItem*)item;
    ListItemType lit = list_item->type;
    string entity = list_item->text().toStdString();
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





void LipidSpaceGUI::setInitialized(){
    initialized();
}


void LipidSpaceGUI::setExport(){
    QString outputFolder = QFileDialog::getExistingDirectory(0, ("Select Export Folder"), QDir::currentPath());
    
    if (outputFolder.length()) exporting(outputFolder.toStdString());
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
    fill_Table();
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

void LipidSpaceGUI::showHideGlobalLipidome(){
    showGlobalLipidome = ui->actionShow_global_lipidome->isChecked();
    updateGUI();
}

void LipidSpaceGUI::showHideQuant(){
    GlobalData::showQuant = ui->actionShow_quantitative_information->isChecked();
    updateCanvas();
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
    GlobalData::pie_tree_depth = depth;
    ui->dendrogramView->setFeature(ui->featureComboBox->currentText().toStdString());
}


void LipidSpaceGUI::setNormalization(int){
    GlobalData::normalization = ui->normalizationComboBox->currentData().toString().toStdString();
}


void LipidSpaceGUI::setDendrogramHeight(int height){
    GlobalData::dendrogram_height = height;
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




void LipidSpaceGUI::resizeEvent(QResizeEvent *event){
    event->ignore();
}


void LipidSpaceGUI::updateGUI(){
    updating = true;
    
    while (ui->featureComboBox->count()) ui->featureComboBox->removeItem(0);
    
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
        for (auto canvas : canvases) canvas->setVisible(true);
        int numTiles = 2 * (lipid_space->selected_lipidomes.size() > 1) + lipid_space->selected_lipidomes.size();
        int tileColumns = tileLayout == AUTOMATIC ? ceil(sqrt((double)numTiles)) : (int)tileLayout;
        
        
        int c = 0, r = 0;
        // show dendrogram if enabled
        if (lipid_space->selected_lipidomes.size() > 1){
            if (showDendrogram){
                ui->gridLayout->addWidget(canvases[0], r, c);
                if (++c == tileColumns){
                    c = 0;
                    ++r;
                }
            }
            else {
                canvases[0]->setVisible(false);
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
            }
            else {
                canvases[1]->setVisible(false);
            }
        }
        
        // show all remaining lipidomes
        for(int n = 2 * (lipid_space->selected_lipidomes.size() > 1); n < numTiles; ++n){
            ui->gridLayout->addWidget(canvases[n], r, c);
            if (++c == tileColumns){
                c = 0;
                ++r;
            }
        }
    }
    else {
        for (auto canvas : canvases) canvas->setVisible(false);
        canvases[single_window]->setVisible(true);
        ui->gridLayout->addWidget(canvases[single_window], 0, 0);
    }
    
    
    for (auto kv : lipid_space->feature_values){
        ui->featureComboBox->addItem(kv.first.c_str());
    }
    
    
    updating = false;
    dragLayer->raise();
}


void LipidSpaceGUI::openLists(){
    if (lipid_space->feature_values.size() > 0){
        QMessageBox::warning(this, "List conflict", "Features have been loaded. Lists do not supported any feature import routine. Please reset LipidSpace or load a table with exactly the same features.");
        return;
    }
    
    
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more lipid lists", ".", "Lists *.csv *.tsv *.txt (*.csv *.tsv *.txt)");
    
    if (files.size()){
        bool start_analysis = true;
        for (QString file_name : files){
            if (!start_analysis) break;
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
                                resetAnalysis();
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
                                    resetAnalysis();
                                }
                                else {
                                    resetAnalysis();
                                    repeat_loading = false;
                                    start_analysis = false;
                                }
                            }
                            break;
                            
                        case NoColumnFound:
                            {
                                msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                                msgBox.exec();
                                repeat_loading = false;
                                start_analysis = false;
                            }
                            break;
                            
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
        else {
            resetAnalysis();
        }
    }
}



void LipidSpaceGUI::ShowContextMenuDendrogram(const QPoint pos){
    QMenu *menu = new QMenu(this);
    QAction *exportAsPdf = new QAction("Export as pdf", this);
    menu->addAction(exportAsPdf);
    menu->popup(ui->dendrogramView->viewport()->mapToGlobal(pos));
    connect(exportAsPdf, &QAction::triggered, ui->dendrogramView, &Canvas::exportAsPdf);
}



void LipidSpaceGUI::ShowContextMenuLipidome(Canvas *canvas, QPoint pos){
    QMenu *menu = new QMenu(this);
    QAction *exportAsPdf = new QAction("Export as pdf", this);
    menu->addAction(exportAsPdf);
    
    menu->popup(canvas->viewport()->mapToGlobal(pos));
    connect(exportAsPdf, &QAction::triggered, canvas, &Canvas::exportAsPdf);
}


void LipidSpaceGUI::ShowContextMenu(const QPoint pos){
    QMenu *menu = new QMenu(this);
    if (ui->tabWidget->currentIndex() != 3){
        QAction *actionSelectAll = new QAction("Select all", this);
        QAction *actionDeselectAll = new QAction("Deselect all", this);
        menu->addAction(actionSelectAll);
        menu->addAction(actionDeselectAll);
        
        QListWidget *widget = nullptr;
        switch(ui->tabWidget->currentIndex()){
            case 0: widget = ui->speciesList; break;
            case 1: widget = ui->classList; break;
            case 2: widget = ui->categoryList; break;
            case 3: return;
            case 4: widget = ui->sampleList; break;
            default: break;
        }
        menu->popup(widget->viewport()->mapToGlobal(pos));
        connect(actionSelectAll, &QAction::triggered, this, &LipidSpaceGUI::select_all_entities);
        connect(actionDeselectAll, &QAction::triggered, this, &LipidSpaceGUI::deselect_all_entities);
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



void LipidSpaceGUI::select_all_entities(){
    QListWidget *widget = nullptr;
    switch(ui->tabWidget->currentIndex()){
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



void LipidSpaceGUI::deselect_all_entities(){
    QListWidget *widget = nullptr;
    switch(ui->tabWidget->currentIndex()){
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
    fill_Table();
}


void LipidSpaceGUI::fill_Table(){
    QTableWidget *t = ui->tableWidget;
    QTableWidgetItem *item = 0;
    
    t->setRowCount(0);
    t->setColumnCount(0);
    
    if ((int)lipid_space->selected_lipidomes.size() == 0 || (int)lipid_space->global_lipidome->lipids.size() == 0) return;
    
    int num_features = lipid_space->feature_values.size();
    map<string, int> lipid_index;
    map<string, int> feature_index;
        
    // achieve sorted list of lipid species
    set< string > sorted_lipid_species;
    set< string > selected_species;
    for (auto lipidome : lipid_space->lipidomes){
        for (auto lipid_species : lipidome->species) sorted_lipid_species.insert(lipid_species);
    }
    for (auto lipid_species : lipid_space->global_lipidome->species) selected_species.insert(lipid_species);
    
    if (table_transposed){
        t->setColumnCount(lipid_space->lipidomes.size());
        t->setRowCount(sorted_lipid_species.size() + num_features);
        
        for (int c = 0; c < (int)lipid_space->lipidomes.size(); c++){
            item = new QTableWidgetItem(QString(lipid_space->lipidomes[c]->cleaned_name.c_str()));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setHorizontalHeaderItem(c, item);
        }
        
        int f = 0;
        for (auto kv : lipid_space->feature_values){
            item = new QTableWidgetItem(kv.first.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            feature_index.insert({kv.first, f});
            t->setVerticalHeaderItem(f++, item);
        }
        
        int rrr = 0;
        for (auto header_std : sorted_lipid_species){
            QString header_name = header_std.c_str();
            
            // dirty way to make the transpose button completely visible
            if (f == 0 && header_name.length() < 10) {
                while (header_name.length() < 14) header_name += " ";
            }
            item = new QTableWidgetItem(header_name);
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
                }
                else {
                    item = new QTableWidgetItem(QString::number(kv.second.numerical_value));
                }
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                int r = feature_index[kv.first];
                t->setItem(r, c, item);
                free_cells.remove(r * C + c);
            }
            // add lipid quant data
            for (int r = 0; r < (int)lipid_space->lipidomes[c]->species.size(); r++){
                QString qn = QString::number(lipid_space->lipidomes[c]->original_intensities[r]);
                item = new QTableWidgetItem(qn);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                
                if (uncontains_val(selected_species, lipid_space->lipidomes[c]->species[r])){
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
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setVerticalHeaderItem(r, item);
        }
        
        int f = 0;
        for (auto kv : lipid_space->feature_values){
            item = new QTableWidgetItem(kv.first.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            feature_index.insert({kv.first, f});
            t->setHorizontalHeaderItem(f++, item);
        }
        
        int ccc = 0;
        for (auto header_std : sorted_lipid_species){
        //for (int c = 0; c < (int)lipid_space->global_lipidome->species.size(); c++){
            item = new QTableWidgetItem(header_std.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
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
                }
                else {
                    item = new QTableWidgetItem(QString::number(kv.second.numerical_value));
                }
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                int c = feature_index[kv.first];
                t->setItem(r, c, item);
                free_cells.remove(r * C + c);
            }
            
            // add lipid quant data
            for (int c = 0; c < (int)lipid_space->lipidomes[r]->species.size(); c++){
                item = new QTableWidgetItem(QString::number(lipid_space->lipidomes[r]->original_intensities[c]));
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (uncontains_val(selected_species, lipid_space->lipidomes[r]->species[c])){
                    item->setData(Qt::ForegroundRole, QBrush(QColor(180, 180, 180)));
                }
                int cc = lipid_index[lipid_space->lipidomes[r]->species[c]];
                t->setItem(r, cc, item);
                free_cells.remove(r * C + cc);
            }
        }
        // fill the empty fields with disabled cells
        for (auto pos : free_cells){
            item = new QTableWidgetItem("");
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            int r = pos / C;
            int c = pos % C;
            t->setItem(r, c, item);
        }
    }
}
