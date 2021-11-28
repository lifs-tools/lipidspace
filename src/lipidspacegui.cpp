#include "lipidspace/lipidspacegui.h"


DragLayer::DragLayer(QWidget *parent) : QWidget(parent) {
    
}

void DragLayer::mousePressEvent(QMouseEvent*, Canvas *canvas){
    if (!isVisible() && canvas->num >= 0){
        source_tile = canvas->num;
        move(canvas->pos());
        resize(canvas->size());
        setVisible(true);
        
        delta = canvas->mapFromGlobal(QCursor::pos());
        grabMouse();
        setMouseTracking(true);
    }
}

void DragLayer::mouseMoveEvent(QMouseEvent*){
    QPoint mouse = parentWidget()->mapFromGlobal(QCursor::pos());
    move(mouse.x() - delta.x(), mouse.y() - delta.y());
    hover();
}


void DragLayer::mouseReleaseEvent(QMouseEvent*){
    releaseMouse();
    swapping(source_tile);
    setVisible(false);
}


void DragLayer::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QColor qc = QColor(0, 110, 255, 50);
    painter.fillRect(QRect(0, 0, width(), height()), qc);
}



void LipidSpaceGUI::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_X && !loadedDataSet){
        loadedDataSet = true;
        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 12; ++i) ct->push_back(IgnoreColumn);
        ct->at(1) = LipidColumn;
        ct->at(6) = SampleColumn;
        ct->at(7) = FeatureColumn;
        ct->at(8) = FeatureColumn;
        ct->at(11) = QuantColumn;
        loadTable("Anxa7_pivot.csv", ct, PIVOT_TABLE);
        
        /*
        for (int i = 0; i < 32; ++i) ct->push_back(SampleColumn);
        ct->at(0) = LipidColumn;
        loadTable("examples/Tablesets/Plasma-Liebisch.csv", ct, ROW_TABLE);
        */
        
        
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
    
    connect(ui->actionLoad_list_s, SIGNAL(triggered()), this, SLOT(openLists()));
    connect(ui->actionLoad_table, SIGNAL(triggered()), this, SLOT(openTable()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quitProgram()));
    
    connect(ui->actionAutomatically, SIGNAL(triggered()), this, SLOT(setAutomaticLayout()));
    connect(ui->actionShow_quantitative_information, SIGNAL(triggered()), this, SLOT(showHideQuant()));
    connect(ui->actionShow_global_lipidome, SIGNAL(triggered()), this, SLOT(showHideGlobalLipidome()));
    connect(ui->actionShow_dendrogram, SIGNAL(triggered()), this, SLOT(showHideDendrogram()));
    connect(ui->action1_column, SIGNAL(triggered()), this, SLOT(set1ColumnLayout()));
    connect(ui->action2_columns, SIGNAL(triggered()), this, SLOT(set2ColumnLayout()));
    connect(ui->action3_columns, SIGNAL(triggered()), this, SLOT(set3ColumnLayout()));
    connect(ui->action4_columns, SIGNAL(triggered()), this, SLOT(set4ColumnLayout()));
    connect(ui->action5_columns, SIGNAL(triggered()), this, SLOT(set5ColumnLayout()));
    connect(ui->action6_columns, SIGNAL(triggered()), this, SLOT(set6ColumnLayout()));
    connect(ui->actionIgnoring_lipid_sn_positions, SIGNAL(triggered()), this, SLOT(setSnPositions()));
    connect(ui->actionManage_lipidomes, SIGNAL(triggered()), this, SLOT(openManageLipidomesWindow()));
    connect(ui->actionSet_transparency, SIGNAL(triggered()), this, SLOT(openSetAlpha()));
    connect(ui->actionSet_number_of_principal_components, SIGNAL(triggered()), this, SLOT(openSetPCnum()));
    connect(ui->actionSelect_principal_components, SIGNAL(triggered()), this, SLOT(openSelectPC()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(ui->actionLog_messages, SIGNAL(triggered()), this, SLOT(openLog()));
    connect(ui->actionIgnore_quantitative_information, SIGNAL(triggered()), this, SLOT(toggleQuant()));
    connect(ui->actionUnbound_lipid_distance_metric, SIGNAL(triggered()), this, SLOT(toggleBoundMetric()));
    connect(ui->actionExport_Results, SIGNAL(triggered()), this, SLOT(setExport()));
    //connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowContextMenu(const QPoint)));
    connect(ui->tableWidget, SIGNAL(cornerButtonClick()), this, SLOT(transposeTable()));
    connect(ui->featureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setFeature(int)));
    connect(this, SIGNAL(featureChanged(string)), ui->dendrogramView, SLOT(setFeature(string)));
    connect(ui->speciesList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->classList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->categoryList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->sampleList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->speciesPushButton, SIGNAL(clicked()), this, SLOT(runAnalysis()));
    connect(ui->classPushButton, SIGNAL(clicked()), this, SLOT(runAnalysis()));
    connect(ui->categoryPushButton, SIGNAL(clicked()), this, SLOT(runAnalysis()));
    connect(ui->samplePushButton, SIGNAL(clicked()), this, SLOT(runAnalysis()));
    
    tileLayout = AUTOMATIC;
    GlobalData::showQuant = true;
    showDendrogram = true;
    showGlobalLipidome = true;
    updating = false;
    GlobalData::color_counter = 0;
    single_window = -1;
    ui->tabWidget->setVisible(false);
    table_transposed = false;
    
    progressbar = new Progressbar(this);
    progress = new Progress();
    connect(progress, SIGNAL(finish()), progressbar, SLOT(finish()));
    connect(progress, SIGNAL(set_current(int)), progressbar, SLOT(set_current(int)));
    connect(progress, SIGNAL(set_max(int)), progressbar, SLOT(set_max(int)));
    connect(progressbar, SIGNAL(interrupt()), progress, SLOT(interrupt()));
    connect(progressbar, SIGNAL(resetAnalysis()), this, SLOT(resetAnalysis()));
    progressbar->setModal(true);
    ui->dendrogramView->setDendrogramData(lipid_space);
    
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



void LipidSpaceGUI::updateSelectionView(){
    disconnect(ui->speciesList, SIGNAL(itemChanged(QListWidgetItem *)), 0, 0);
    disconnect(ui->classList, SIGNAL(itemChanged(QListWidgetItem *)), 0, 0);
    disconnect(ui->categoryList, SIGNAL(itemChanged(QListWidgetItem *)), 0, 0);
    disconnect(ui->sampleList, SIGNAL(itemChanged(QListWidgetItem *)), 0, 0);
    
    
    // remove all items from the lists
    ui->speciesList->clear();
    ui->classList->clear();
    ui->categoryList->clear();
    ui->sampleList->clear();
    
    
    // load new data
    for (auto species : lipid_space->selection[0]){
        ListItem *item = new ListItem(QString(species.first.c_str()), SPECIES_ITEM, ui->speciesList);
        item->setCheckState(species.second ? Qt::Checked : Qt::Unchecked);
        ui->speciesList->addItem(item);
    }
    
    
    for (auto lipid_class : lipid_space->selection[1]){
        ListItem *item = new ListItem(QString(lipid_class.first.c_str()), CLASS_ITEM, ui->classList);
        item->setCheckState(lipid_class.second ? Qt::Checked : Qt::Unchecked);
        ui->classList->addItem(item);
    }
    
    
    for (auto category : lipid_space->selection[2]){
        ListItem *item = new ListItem(QString(category.first.c_str()), CATEGORY_ITEM, ui->categoryList);
        item->setCheckState(category.second ? Qt::Checked : Qt::Unchecked);
        ui->categoryList->addItem(item);
    }
    
    
    for (auto sample : lipid_space->selection[3]){
        ListItem *item = new ListItem(QString(sample.first.c_str()), SAMPLE_ITEM, ui->sampleList);
        item->setCheckState(sample.second ? Qt::Checked : Qt::Unchecked);
        ui->sampleList->addItem(item);
    }
    connect(ui->speciesList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->classList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->categoryList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
    connect(ui->sampleList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
}


    
void LipidSpaceGUI::loadTable(string file_name, vector<TableColumnType>* column_types, TableType table_type){
    bool repeat_loading = true;
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
                            resetAnalysis();
                        }
                        else {
                            resetAnalysis();
                            repeat_loading = false;
                        }
                    }
                    break;
                
                case FileUnreadable:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        resetAnalysis();
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
                            resetAnalysis();
                        }
                        else {
                            resetAnalysis();
                            repeat_loading = false;
                        }
                    }
                    break;
                    
                case NoColumnFound:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        resetAnalysis();
                        repeat_loading = false;
                    }
                    break;
                    
                case ColumnNumMismatch:
                    {
                        msgBox.setInformativeText("Please check your input file and try again. In case, please contact the developers.");
                        msgBox.exec();
                        resetAnalysis();
                        repeat_loading = false;
                    }
                    break;
                    
                default:
                    {
                        msgBox.setInformativeText("Please check the log message. In case, please contact the developers.");
                        msgBox.exec();
                        resetAnalysis();
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
    
    std::thread runAnalysisThread = lipid_space->run_analysis_thread(progress);
    progressbar->exec();
    runAnalysisThread.join();
    
    for (auto canvas : canvases){
        disconnect(canvas, SIGNAL(transforming(QRectF)), 0, 0);
        disconnect(canvas, SIGNAL(showMessage(QString)), 0, 0);
        disconnect(canvas, SIGNAL(doubleClicked(int)), 0, 0);
        disconnect(canvas, SIGNAL(mouse(QMouseEvent*, Canvas*)), 0, 0);
        disconnect(canvas, SIGNAL(swappingLipidomes(int, int)), 0, 0);
        delete canvas;
    }
    canvases.clear();
    updateGUI();
    
    GlobalData::color_counter = 0;
    GlobalData::feature_counter = 0;
    GlobalData::colorMap.clear();
    GlobalData::colorMapFeatures.clear();
    
    int numTiles = 2 * (lipid_space->selected_lipidomes.size() > 1) + lipid_space->selected_lipidomes.size();
    ui->dendrogramView->resetDendrogram();
    
    for (int n = 0; n < numTiles; ++n){
        int num = 0;
        if ((lipid_space->selected_lipidomes.size() > 1) && (n == 0)) num = -2;
        else if ((lipid_space->selected_lipidomes.size() > 1) && (n == 1)) num = -1;
        else num = max(0, n - 2 * (lipid_space->selected_lipidomes.size() > 1));
        
        Canvas* canvas = new Canvas(lipid_space, num, ui->centralwidget);
        connect(canvas, SIGNAL(doubleClicked(int)), this, SLOT(setDoubleClick(int)));
        if (num != -2){
            connect(canvas, SIGNAL(transforming(QRectF)), this, SLOT(setTransforming(QRectF)));
            connect(this, SIGNAL(transforming(QRectF)), canvas, SLOT(setTransforming(QRectF)));
            connect(canvas, SIGNAL(showMessage(QString)), this, SLOT(showMessage(QString)));
            connect(this, SIGNAL(updateCanvas()), canvas, SLOT(setUpdate()));
            connect(this, SIGNAL(exporting(QString)), canvas, SLOT(exportPdf(QString)));
            connect(this, SIGNAL(initialized()), canvas, SLOT(setInitialized()));
            connect(canvas, SIGNAL(mouse(QMouseEvent*, Canvas*)), dragLayer, SLOT(mousePressEvent(QMouseEvent*, Canvas*)));
            connect(dragLayer, SIGNAL(hover()), canvas, SLOT(hoverOver()));
            connect(dragLayer, SIGNAL(swapping(int)), canvas, SLOT(setSwap(int)));
            connect(canvas, SIGNAL(swappingLipidomes(int, int)), this, SLOT(swapLipidomes(int, int)));
        }
        canvases.push_back(canvas);
    }
    
    // define colors of features
    for (auto kv : lipid_space->feature_values){
        string feature_prefix = kv.first + "_";
        for (string feature : kv.second){
            feature = feature_prefix + feature;
            GlobalData::colorMapFeatures.insert({feature, GlobalData::COLORS[GlobalData::feature_counter++ % GlobalData::COLORS.size()]});
        }
    }
    
    
    fill_Table();
    ui->tabWidget->setVisible(true);
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



void LipidSpaceGUI::setInitialized(){
    initialized();
}


void LipidSpaceGUI::setExport(){
    QString outputFolder = QFileDialog::getExistingDirectory(0, ("Select Export Folder"), QDir::currentPath());
    
    //if (outputFolder.length()) exporting(outputFolder);
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
    ui->tabWidget->setVisible(false);
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
                    cout << e.what() << endl;
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


void LipidSpaceGUI::ShowContextMenu(const QPoint pos){
    QMenu *menu = new QMenu(this);
    QAction *action = new QAction("Transpose table", this);
    menu->addAction(action);
    menu->popup(ui->tableWidget->viewport()->mapToGlobal(pos));
    connect(action, SIGNAL(triggered()), this, SLOT(transposeTable()));
}


void LipidSpaceGUI::transposeTable(){
    table_transposed = !table_transposed;
    fill_Table();
}


void LipidSpaceGUI::fill_Table(){
    QTableWidget *t = ui->tableWidget;
    QTableWidgetItem *item = 0;
    
    t->setColumnCount(0);
    t->setRowCount(0);
    // delete items before refilling
    for (int c = 0; c < (int)t->columnCount(); c++){
        item = t->takeHorizontalHeaderItem(c);
        if (item) delete item;
    }
    
    for (int r = 0; r < (int)t->rowCount(); ++r){
        item = t->takeVerticalHeaderItem(r);
        if (item) delete item;
        
        for (int c = 0; c < (int)t->columnCount(); c++){
            item = t->takeItem(r, c);
            if (item) delete item;
        }
    }
    
    if ((int)lipid_space->selected_lipidomes.size() == 0 || (int)lipid_space->global_lipidome->lipids.size() == 0) return;
    
    
    int num_features = lipid_space->feature_values.size();
    map<LipidAdduct*, int> lipid_index;
    map<string, int> feature_index;
    
    if (table_transposed){
        t->setColumnCount(lipid_space->lipidomes.size());
        t->setRowCount(lipid_space->global_lipidome->lipids.size() + num_features);
        
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
        
        for (int r = 0; r < (int)lipid_space->global_lipidome->species.size(); ++r){
            QString header_name = lipid_space->global_lipidome->species[r].c_str();
            // dirty way to make the transpose button completely visible
            if (f == 0 && header_name.length() < 10) {
                while (header_name.length() < 14) header_name += " ";
            }
            item = new QTableWidgetItem(header_name);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setVerticalHeaderItem(num_features + r, item);
            lipid_index.insert({lipid_space->global_lipidome->lipids[r], num_features + r});
        }
        
        // fill features and content
        int R = t->rowCount();
        int C = t->columnCount();
        int n = R * C;
        Bitfield free_cells(n, true);

        for (int c = 0; c < C; c++){
            
            // add features
            for(auto kv : lipid_space->lipidomes[c]->features){
                item = new QTableWidgetItem(kv.second.c_str());
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                int r = feature_index[kv.first];
                t->setItem(r, c, item);
                free_cells.remove(r * C + c);
            }
            // add lipid quant data
            for (int r = 0; r < (int)lipid_space->lipidomes[c]->species.size(); r++){
                item = new QTableWidgetItem(QString::number(lipid_space->lipidomes[c]->original_intensities[r]));
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                int rr = lipid_index[lipid_space->lipidomes[c]->lipids[r]];
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
        t->setColumnCount(lipid_space->global_lipidome->lipids.size() + num_features);
        
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
        
        for (int c = 0; c < (int)lipid_space->global_lipidome->species.size(); c++){
            item = new QTableWidgetItem(lipid_space->global_lipidome->species[c].c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setHorizontalHeaderItem(num_features + c, item);
            lipid_index.insert({lipid_space->global_lipidome->lipids[c], num_features + c});
        }
    
    
        // fill features and content
        int R = t->rowCount();
        int C = t->columnCount();
        int n = R * C;
        Bitfield free_cells(n, true);

        for (int r = 0; r < R; ++r){
            // add features
            for(auto kv : lipid_space->lipidomes[r]->features){
                item = new QTableWidgetItem(kv.second.c_str());
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                int c = feature_index[kv.first];
                t->setItem(r, c, item);
                free_cells.remove(r * C + c);
            }
            
            // add lipid quant data
            for (int c = 0; c < (int)lipid_space->lipidomes[r]->species.size(); c++){
                item = new QTableWidgetItem(QString::number(lipid_space->lipidomes[r]->original_intensities[c]));
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                int cc = lipid_index[lipid_space->lipidomes[r]->lipids[c]];
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
