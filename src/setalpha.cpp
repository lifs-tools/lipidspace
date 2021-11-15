#include "lipidspace/setalpha.h"

SetAlpha::SetAlpha(QWidget *parent) : QDialog(parent), ui(new Ui::SetAlpha) {
    ui->setupUi(this);
    setWindowTitle("Set transparency");
    
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(width(), height());
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(255);
    ui->horizontalSlider->setTickInterval(256);
    ui->horizontalSlider->setValue(LipidSpaceGUI::alpha);
    
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

SetAlpha::~SetAlpha() {
    delete ui;
}


void SetAlpha::cancel(){
    close();
}

void SetAlpha::ok(){
    LipidSpaceGUI::alpha = ui->horizontalSlider->value();
    close();
}
