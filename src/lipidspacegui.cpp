#include "lipidspace/lipidspacegui.h"


GlobalLipidomeModel::GlobalLipidomeModel(LipidSpace *lipid_space, QObject * parent) : QAbstractTableModel{parent} {
    columns = 0;
    int C = (int)lipid_space->lipidomes.size();
    
    Matrix global_data(lipid_space->global_lipidome->m.rows, lipid_space->lipidomes.size());
    map<string, int> species_to_index;
    for (int i = 0; i < (int)lipid_space->global_lipidome->species.size(); ++i){
        species_to_index.insert({lipid_space->global_lipidome->species[i], i});
    }
    
    for (int c = 0; c < (int)lipid_space->lipidomes.size(); c++){
        auto lipidome = lipid_space->lipidomes[c];
        for (int r = 0; r < (int)lipidome->species.size(); ++r){
            global_data(species_to_index[lipidome->species[r]], c) = lipidome->intensities(r);
        }
    }
    
    headers.push_back("Species");
    map<string, int> mapping;
    for (int c = 0; c < C; c++){
        QFileInfo qFileInfo(lipid_space->lipidomes[c]->file_name.c_str());
        headers.push_back(qFileInfo.baseName());
        
    }
    
    for (int r = 0; r < (int)lipid_space->global_lipidome->species.size(); ++r){
        rows.append(QList<QString>());
        QList<QString> &current_row = rows.back();
        current_row.reserve(C + 1);
        current_row.push_back(lipid_space->global_lipidome->species[r].c_str());
        for (int c = 0; c < C; c++){
            current_row.push_back("");
        }
    }
}


int GlobalLipidomeModel::rowCount(const QModelIndex &) const {
    return headers.size();
}


int GlobalLipidomeModel::columnCount(const QModelIndex &) const {
    return columns;
}


QVariant GlobalLipidomeModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
    return rows[index.row()][index.column()];
}


QVariant GlobalLipidomeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    return headers[section];
}


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




LipidSpaceGUI::LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent) : QMainWindow(parent), timer(this) {
    lipid_space = _lipid_space;
    ui = new Ui::LipidSpaceGUI();
    ui->setupUi(this);
    
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
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowContextMenu(const QPoint)));
            
    
    tileLayout = AUTOMATIC;
    showQuant = true;
    showDendrogram = true;
    showGlobalLipidome = true;
    updating = false;
    color_counter = 0;
    single_window = -1;
    ui->tabWidget->setVisible(false);
    global_lipidome_model = 0;
    table_transposed = false;
    
    progressbar = new Progressbar(this);
    progress = new Progress();
    connect(progress, SIGNAL(finish()), progressbar, SLOT(finish()));
    connect(progress, SIGNAL(set_current(int)), progressbar, SLOT(set_current(int)));
    connect(progress, SIGNAL(set_max(int)), progressbar, SLOT(set_max(int)));
    connect(progressbar, SIGNAL(interrupt()), progress, SLOT(interrupt()));
    connect(progressbar, SIGNAL(resetAnalysis()), this, SLOT(resetAnalysis()));
    progressbar->setModal(true);
    
    
    updateGUI();
}



LipidSpaceGUI::~LipidSpaceGUI(){
    delete ui;
    delete progress;
    delete progressbar;
    delete dragLayer;
}

int LipidSpaceGUI::alpha = DEFAULT_ALPHA;
bool LipidSpaceGUI::showQuant = true;
int LipidSpaceGUI::color_counter = 0;
map<string, QColor> LipidSpaceGUI::colorMap;
int LipidSpaceGUI::PC1 = 0;
int LipidSpaceGUI::PC2 = 1;

const vector<QColor> LipidSpaceGUI::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"), QColor("#17becf")};



void LipidSpaceGUI::quitProgram(){
    QApplication::quit();
}



void LipidSpaceGUI::openTable(){
    ImportTable it;
    connect(&it, SIGNAL(importTable(string, vector<TableColumnType>*, TableType)), this, SLOT(loadTable(string, vector<TableColumnType>*, TableType)));
    it.setModal(true);
    it.exec();
}


    
void LipidSpaceGUI::loadTable(string file_name, vector<TableColumnType>* column_types, TableType table_type){
    try {
        if (table_type == ROW_TABLE){
            lipid_space->load_row_table(file_name, column_types);
        }
        else if (table_type == COLUMN_TABLE){
            lipid_space->load_column_table(file_name, column_types);
        }
        else {
            lipid_space->load_pivot_table(file_name, column_types);
        }
        runAnalysis();
    }
    catch (exception &e) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Error during table import");
        msgBox.setText(e.what());
        msgBox.setInformativeText("Do you want to continue by ignoring unknown lipid species?");
        
        QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
        
        msgBox.setDefaultButton(msgBox.addButton(tr("Abort"), QMessageBox::NoRole));
        msgBox.exec();
        if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
            lipid_space->ignore_unknown_lipids = true;
            resetAnalysis();
            if (table_type == ROW_TABLE){
                lipid_space->load_row_table(file_name, column_types);
            }
            else if (table_type == COLUMN_TABLE){
                lipid_space->load_column_table(file_name, column_types);
            }
            else {
                lipid_space->load_pivot_table(file_name, column_types);
            }
            runAnalysis();
        }
        else {
            resetAnalysis();
        }
    }
}



void LipidSpaceGUI::runAnalysis(){
    
    lipid_space->analysis_finished = false;
    disconnect(this, SIGNAL(transforming(QRectF, int)), 0, 0);
    disconnect(this, SIGNAL(updateCanvas()), 0, 0);
    
    std::thread runAnalysisThread = lipid_space->run_analysis_thread(progress);
    progressbar->exec();
    runAnalysisThread.join();
    
    for (auto canvas : canvases){
        disconnect(canvas, SIGNAL(transforming(QRectF, int)), 0, 0);
        disconnect(canvas, SIGNAL(showMessage(QString)), 0, 0);
        disconnect(canvas, SIGNAL(doubleClicked(int)), 0, 0);
        disconnect(canvas, SIGNAL(mouse(QMouseEvent*, Canvas*)), 0, 0);
        disconnect(canvas, SIGNAL(swappingLipidomes(int, int)), 0, 0);
        delete canvas;
    }
    canvases.clear();
    updateGUI();
    
    color_counter = 0;
    int numTiles = 2 * (lipid_space->lipidomes.size() > 1) + lipid_space->lipidomes.size();
    
    for (int n = 0; n < numTiles; ++n){
        int num = 0;
        if ((lipid_space->lipidomes.size() > 1) && (n == 0)) num = -2;
        else if ((lipid_space->lipidomes.size() > 1) && (n == 1)) num = -1;
        else num = max(0, n - 2 * (lipid_space->lipidomes.size() > 1));
        
        Canvas* canvas = new Canvas(lipid_space, this, num, ui->centralwidget);
        connect(canvas, SIGNAL(doubleClicked(int)), this, SLOT(setDoubleClick(int)));
        if (num != -2){
            connect(canvas, SIGNAL(transforming(QRectF, int)), this, SLOT(setTransforming(QRectF, int)));
            connect(this, SIGNAL(transforming(QRectF, int)), canvas, SLOT(setTransforming(QRectF, int)));
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
    
    fill_Table();
    ui->tabWidget->setVisible(true);
    updateGUI();
    
    // dirty hack to overcome the annoying resizing calls of the canvases when
    // putting them into the grid layout. Unfortunately, I cannot figure out, when
    // it is over. So I use a timer and hope that after 200ms all rearranging is over
    QTimer::singleShot(200, this, SLOT(setInitialized()));
}


void LipidSpaceGUI::setInitialized(){
    initialized();
}


void LipidSpaceGUI::setExport(){
    QString outputFolder = QFileDialog::getExistingDirectory(0, ("Select Export Folder"), QDir::currentPath());
    
    //if (outputFolder.length()) exporting(outputFolder);
}


void LipidSpaceGUI::setTransforming(QRectF f, int _num){
    transforming(f, _num);
}


void LipidSpaceGUI::setDoubleClick(int _num){
    _num += 2 * (lipid_space->lipidomes.size() > 1);
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
    PC1 = 0;
    PC2 = 1;
    LipidSpace::cols_for_pca = LipidSpace::cols_for_pca_init;
    
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
    showQuant = ui->actionShow_quantitative_information->isChecked();
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
    for (int i = (lipid_space->lipidomes.size() > 1); i < (int)canvases.size(); ++i){
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
        int numTiles = 2 * (lipid_space->lipidomes.size() > 1) + lipid_space->lipidomes.size();
        int tileColumns = tileLayout == AUTOMATIC ? ceil(sqrt((double)numTiles)) : (int)tileLayout;
        
        
        int c = 0, r = 0;
        // show dendrogram if enabled
        if (lipid_space->lipidomes.size() > 1){
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
        if (lipid_space->lipidomes.size() > 1){
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
        for(int n = 2 * (lipid_space->lipidomes.size() > 1); n < numTiles; ++n){
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
    
    updating = false;
    dragLayer->raise();
}


void LipidSpaceGUI::openLists(){
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more lipid lists", ".", "Lists *.csv *.tsv *.txt (*.csv *.tsv *.txt)");
    if (files.size()){
        try {
            for (QString file_name : files){
                lipid_space->lipidomes.push_back(lipid_space->load_list(file_name.toUtf8().constData()));
            }
            runAnalysis();
        }
        catch (exception &e) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Error during list import");
            msgBox.setText(e.what());
            msgBox.setInformativeText("Do you want to continue by ignoring unknown lipid species?");
            
            QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
            
            msgBox.setDefaultButton(msgBox.addButton(tr("Abort"), QMessageBox::NoRole));
            msgBox.exec();
            if (msgBox.clickedButton() == (QAbstractButton*)continueButton){
                lipid_space->ignore_unknown_lipids = true;
                resetAnalysis();
                for (QString file_name : files){
                    lipid_space->lipidomes.push_back(lipid_space->load_list(file_name.toUtf8().constData()));
                }
                runAnalysis();
            }
            else {
                resetAnalysis();
            }
        }
    }
}


void LipidSpaceGUI::ShowContextMenu(const QPoint pos) {
    //QModelIndex index = ui->tableWidget->indexAt(pos);

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
    
    
    int num_features = lipid_space->lipidomes[0]->features.size();
    map<LipidAdduct*, int> lipid_index;
    map<string, int> feature_index;
    
    if (table_transposed){
        t->setColumnCount(lipid_space->lipidomes.size());
        t->setRowCount(lipid_space->global_lipidome->lipids.size() + num_features);
        
        for (int c = 0; c < (int)lipid_space->lipidomes.size(); c++){
            QFileInfo qFileInfo(lipid_space->lipidomes[c]->file_name.c_str());
            item = new QTableWidgetItem(qFileInfo.baseName());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setHorizontalHeaderItem(c, item);
        }
        
        int f = 0;
        for (auto kv : lipid_space->lipidomes[0]->features){
            item = new QTableWidgetItem(kv.first.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            feature_index.insert({kv.first, f});
            t->setVerticalHeaderItem(f++, item);
        }
        
        for (int r = 0; r < (int)lipid_space->global_lipidome->species.size(); ++r){
            item = new QTableWidgetItem(lipid_space->global_lipidome->species[r].c_str());
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
            QFileInfo qFileInfo(lipid_space->lipidomes[r]->file_name.c_str());
            item = new QTableWidgetItem(qFileInfo.baseName());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setVerticalHeaderItem(r, item);
        }
        
        int f = 0;
        for (auto kv : lipid_space->lipidomes[0]->features){
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
