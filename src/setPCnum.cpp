#include "lipidspace/setPCnum.h"

SetPCnum::SetPCnum(QWidget *parent) : QDialog(parent),  ui(new Ui::SetPCnum) {
    ui->setupUi(this);
    setWindowTitle("Set number of Principal components");
    
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(width(), height());
    ui->spinBox->setMinimum(2);
    ui->spinBox->setMaximum(((LipidSpaceGUI*)parent)->lipid_space->global_lipidome->lipids.size() - 1);
    ui->spinBox->setValue(LipidSpace::cols_for_pca);
    
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

SetPCnum::~SetPCnum() {
    delete ui;
}


void SetPCnum::cancel(){
    close();
}

void SetPCnum::ok(){
    LipidSpace::cols_for_pca = (int)ui->spinBox->value();
    close();
}
