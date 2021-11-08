#include "lipidspace/managelipidomes.h"


ManageLipidomes::ManageLipidomes(LipidSpace *_lipidSpace, QWidget *parent) : QDialog(parent), ui(new Ui::ManageLipidomes){
    ui->setupUi(this);
    lipidSpace = _lipidSpace;
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    
    connect(ui->removeSelectedButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
    connect(ui->removeAllButton, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    
    for (auto lipidome : lipidSpace->lipidomes){
        QFileInfo qFileInfo(lipidome->file_name.c_str());
        ui->listView->addItem(qFileInfo.baseName());
    }
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

ManageLipidomes::~ManageLipidomes(){
    delete ui;
}

void ManageLipidomes::cancel(){
    close();
}

void ManageLipidomes::ok(){
    close();
}

void ManageLipidomes::removeSelected(){
}

void ManageLipidomes::removeAll(){
}
