#include "lipidspace/managelipidomes.h"


ManageLipidomes::ManageLipidomes(LipidSpace *_lipid_space, QWidget *parent) : QDialog(parent), ui(new Ui::ManageLipidomes){
    ui->setupUi(this);
    lipid_space = _lipid_space;
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    
    connect(ui->removeSelectedButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
    connect(ui->removeAllButton, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    
    int i = 0;
    for (auto lipidome : lipid_space->lipidomes){
        QString title = lipidome->cleaned_name.c_str();
        ui->listView->addItem(title);
        itemIndex.insert({title, i++});
        
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
    if (removeItems.size()){
        if (removeItems.size() == lipid_space->lipidomes.size()){
            emit resetAnalysis();
        }
        else {
            sort(removeItems.begin(), removeItems.end());
            for (int i = removeItems.size() - 1; i >= 0; i--){
                lipid_space->lipidomes.erase(lipid_space->lipidomes.begin() + removeItems[i]);
            }
            reassembleSelection();
            runAnalysis();
        }
    }
    close();
}

void ManageLipidomes::removeSelected(){
    for (auto qitem : ui->listView->selectedItems()){
        QString title = qitem->text();
        removeItems.push_back(itemIndex[title]);
        ui->listView->takeItem(ui->listView->row(qitem));
    }
}


void ManageLipidomes::removeAll(){
    while (ui->listView->count()){
        QString title = ui->listView->takeItem(0)->text();
        removeItems.push_back(itemIndex[title]);
    }
}
