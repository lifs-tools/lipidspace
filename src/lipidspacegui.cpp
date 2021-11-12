#include "lipidspace/lipidspacegui.h"


LipidSpaceGUI::LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent) : QMainWindow(parent) , ui(new Ui::LipidSpaceGUI), lipid_space(_lipid_space){
    ui->setupUi(this);
    
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
    connect(ui->actionIgnore_quantitative_information, SIGNAL(triggered()), this, SLOT(toggleQuant()));
    connect(ui->actionUnbound_lipid_distance_metric, SIGNAL(triggered()), this, SLOT(toggleBoundMetric()));
            
    
    tileLayout = AUTOMATIC;
    showQuant = true;
    showDendrogram = true;
    showGlobalLipidome = true;
    updating = false;
    color_counter = 0;
    
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
}

bool LipidSpaceGUI::showQuant = true;

int LipidSpaceGUI::color_counter = 0;

map<string, QColor> LipidSpaceGUI::colorMap;

const vector<QColor> LipidSpaceGUI::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"), QColor("#17becf")};



void LipidSpaceGUI::quitProgram(){
    QApplication::quit();
}


void LipidSpaceGUI::openTable(){
    QString file_name = QFileDialog::getOpenFileName(this, "Select a lipid table", ".", "Tables *.csv *.tsv *.xls (*.csv *.tsv *.xls)");
    if (file_name.length()){
        try {
            lipid_space->load_table(file_name.toUtf8().constData());
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
                
                lipid_space->load_table(file_name.toUtf8().constData());
                runAnalysis();
            }
            else {
                resetAnalysis();
            }
        }
    }
}



void LipidSpaceGUI::runAnalysis(){
    for (auto canvas : canvases) delete canvas;
    canvases.clear();
    updateGUI();
    
    std::thread runAnalysisThread = lipid_space->run_analysis_thread(progress);
    progressbar->exec();
    runAnalysisThread.join();
    
    color_counter = 0;
    int numTiles = 2 * (lipid_space->lipidomes.size() > 1) + lipid_space->lipidomes.size();
    
    for (int n = 0; n < numTiles; ++n){
        int num = 0;
        if ((lipid_space->lipidomes.size() > 1) && (n == 0)) num = -2;
        else if ((lipid_space->lipidomes.size() > 1) && (n == 1)) num = -1;
        else num = max(0, n - 2 * (lipid_space->lipidomes.size() > 1));
        
        Canvas* canvas = new Canvas(lipid_space, this, num, ui->centralwidget);
        if (num != -2){
            connect(canvas, SIGNAL(scaling(QWheelEvent *, QRectF, int)), this, SLOT(setScale(QWheelEvent *, QRectF, int)));
            connect(this, SIGNAL(scaling(QWheelEvent *, QRectF, int)), canvas, SLOT(setScale(QWheelEvent *, QRectF, int)));
            connect(canvas, SIGNAL(moving(QRectF, int)), this, SLOT(setMove(QRectF, int)));
            connect(this, SIGNAL(moving(QRectF, int)), canvas, SLOT(setMove(QRectF, int)));
            connect(canvas, SIGNAL(showMessage(QString)), this, SLOT(showMessage(QString)));
            connect(this, SIGNAL(updateCanvas()), canvas, SLOT(setUpdate()));
        }
        canvases.push_back(canvas);
    }
    
    updateGUI();
}


void LipidSpaceGUI::setScale(QWheelEvent *event, QRectF f, int _num){
    scaling(event, f, _num);
}


void LipidSpaceGUI::setMove(QRectF f, int _num){
    moving(f, _num);
}



void LipidSpaceGUI::resetAnalysis(){
    QLayoutItem *wItem;
    while ((wItem = ui->gridLayout->layout()->takeAt(0)) != 0){
        delete wItem;
    }
    for (auto canvas : canvases) delete canvas;
    canvases.clear();
    
    lipid_space->reset_analysis();
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
    //ui->canvas->showHideQuant(showQuant);
    //ui->canvas->update();
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
    lipid_space->run_analysis();
    updateGUI();
}


void LipidSpaceGUI::openManageLipidomesWindow(){
    ManageLipidomes manageLipidomes(lipid_space, this);
    connect(&manageLipidomes, SIGNAL(runAnalysis()), this, SLOT(runAnalysis()));
    connect(&manageLipidomes, SIGNAL(resetAnalysis()), this, SLOT(resetAnalysis()));
    manageLipidomes.setModal(true);
    manageLipidomes.exec();
}


void LipidSpaceGUI::updateGUI(){
    updating = true;
    
    ui->menuAnalysis->setEnabled(lipid_space->lipidomes.size());
    ui->menuView->setEnabled(lipid_space->lipidomes.size());
    
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
    while ((wItem = ui->gridLayout->layout()->takeAt(0)) != 0);
    
    if (!lipid_space->analysis_finished) return;
    
    int numTiles = 2 * (lipid_space->lipidomes.size() > 1) + lipid_space->lipidomes.size();
    int tileColumns = tileLayout == AUTOMATIC ? ceil(sqrt((double)numTiles)) : (int)tileLayout;
    
    int c = 0, r = 0;
    // show dendrogram if enabled
    if (showDendrogram && lipid_space->lipidomes.size() > 1){
        ui->gridLayout->addWidget(canvases[0], r, c);
        if (++c == tileColumns){
            c = 0;
            ++r;
        }
    }
    
    // show global lipidome if enabled
    if (showGlobalLipidome && lipid_space->lipidomes.size() > 1){
        ui->gridLayout->addWidget(canvases[1], r, c);
        if (++c == tileColumns){
            c = 0;
            ++r;
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
    updating = false;
    /*
    ui->canvas->setLayout((int)tileLayout);
    ui->canvas->showHideQuant(showQuant);
    ui->canvas->showHideDendrogram(showDendrogram);
    ui->canvas->showHideGlobalLipidome(showGlobalLipidome);
    ui->canvas->refreshCanvas();
    */
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
