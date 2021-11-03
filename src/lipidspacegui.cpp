#include "lipidspace/lipidspacegui.h"


LipidSpaceGUI::LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LipidSpaceGUI),
    lipid_space(_lipid_space)
{
    ui->setupUi(this);
    
    connect(ui->actionLoad_list_s, SIGNAL(triggered()), this, SLOT(openLists()));
    connect(ui->actionLoad_table, SIGNAL(triggered()), this, SLOT(openTable()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quitProgram()));
}

LipidSpaceGUI::~LipidSpaceGUI()
{
    delete ui;
}


void LipidSpaceGUI::quitProgram(){
    QApplication::quit();
}


void LipidSpaceGUI::openTable(){
    
}


void LipidSpaceGUI::openLists(){
    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more lipid lists",
                            ".",
                            "Lists (*.csv *.tsv *.txt)");
    if (files.size()){
        for (QString file_name : files){
            lipid_space->lipidomes.push_back(lipid_space->load_list(file_name.toUtf8().constData()));
        }
        lipid_space->run_analysis();
    }
}
