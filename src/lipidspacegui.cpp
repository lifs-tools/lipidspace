#include "lipidspace/lipidspacegui.h"
#include "ui_lipidspacegui.h"

LipidSpaceGUI::LipidSpaceGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LipidSpaceGUI)
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
}
