#include "lipidspace/lipidspacegui.h"


LipidSpaceGUI::LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LipidSpaceGUI),
    lipid_space(_lipid_space)
{
    ui->setupUi(this);
    ui->canvas->setInputClasses(lipid_space, this);
    connect(ui->actionLoad_list_s, SIGNAL(triggered()), this, SLOT(openLists()));
    connect(ui->actionLoad_table, SIGNAL(triggered()), this, SLOT(openTable()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quitProgram()));
    connect(ui->actionRemove_all_lipidomes, SIGNAL(triggered()), this, SLOT(resetAnalysis()));
    connect(ui->canvas, SIGNAL(showMessage(QString)), this, SLOT(showMessage(QString)));
    
    tileLayout = AUTOMATIC;
    showDendrogram = true;
    showGlobalLipidome = true;
}

LipidSpaceGUI::~LipidSpaceGUI()
{
    delete ui;
}




void LipidSpaceGUI::quitProgram(){
    QApplication::quit();
}


void LipidSpaceGUI::openTable(){
    QString file_name = QFileDialog::getOpenFileName(this, "Select a lipid table", ".", "Tables *.csv *.tsv *.xls (*.csv *.tsv *.xls)");
    if (file_name.length()){
        try {
            lipid_space->load_table(file_name.toUtf8().constData());
            lipid_space->run_analysis();
            ui->canvas->refreshCanvas();
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
                lipid_space->run_analysis();
                ui->canvas->refreshCanvas();
            }
        }
    }
}


void LipidSpaceGUI::resetAnalysis(){
    lipid_space->reset_analysis();
    ui->canvas->resetCanvas();
}


void LipidSpaceGUI::showMessage(QString message){
    statusBar()->showMessage(message);
}


void updateGUI(){
    
}


void LipidSpaceGUI::openLists(){
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more lipid lists", ".", "Lists *.csv *.tsv *.txt (*.csv *.tsv *.txt)");
    if (files.size()){
        try {
            for (QString file_name : files){
                lipid_space->lipidomes.push_back(lipid_space->load_list(file_name.toUtf8().constData()));
            }
            lipid_space->run_analysis();
            ui->canvas->refreshCanvas();
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
                lipid_space->run_analysis();
                ui->canvas->refreshCanvas();
            }
            else {
                resetAnalysis();
            }
        }
    }
}
