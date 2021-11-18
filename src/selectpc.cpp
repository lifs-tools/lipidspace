#include "lipidspace/selectpc.h"
#include "ui_selectpc.h"

SelectPC::SelectPC(QWidget *parent) : QDialog(parent), ui(new Ui::SelectPC) {
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(width(), height());
    
    
    int max_pc = LipidSpace::cols_for_pca;
    for (int i = 1; i <= max_pc; ++i){
        ui->comboBox->addItem(QStringLiteral("Principal component %1").arg(i));
        ui->comboBox_2->addItem(QStringLiteral("Principal component %1").arg(i));
    }
    ui->comboBox->setCurrentIndex(LipidSpaceGUI::PC1);
    ui->comboBox_2->setCurrentIndex(LipidSpaceGUI::PC2);
    
    
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

SelectPC::~SelectPC(){
    delete ui;
}


void SelectPC::cancel(){
    close();
}

void SelectPC::ok(){
    LipidSpaceGUI::PC1 = (int)ui->comboBox->currentIndex();
    LipidSpaceGUI::PC2 = (int)ui->comboBox_2->currentIndex();
    reloadPoints();
    close();
}
