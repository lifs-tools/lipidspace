#include "lipidspace/filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(pair<FeatureFilter, vector<double>> &_filter, QWidget *parent) : QDialog(parent), ui(new Ui::FilterDialog), filter(_filter) {
    ui->setupUi(this);
    setWindowTitle("Set numerical filter");
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    
    ui->comboBox->addItem("no filter");
    ui->comboBox->addItem("less filter");
    ui->comboBox->addItem("greater filter");
    ui->comboBox->addItem("equal filter");
    ui->comboBox->addItem("within range filter");
    ui->comboBox->addItem("outside range filter");

    ui->lessLabel->move(ui->lessLabel->x(), 92);
    ui->greaterLabel->move(ui->greaterLabel->x(), 92);
    ui->withinLabel->move(ui->withinLabel->x(), 92);
    ui->outsideLabel1->move(ui->outsideLabel1->x(), 92);
    ui->outsideLabel2->move(ui->outsideLabel2->x(), 92);
    ui->outsideLabel3->move(ui->outsideLabel3->x(), 92);
    
    ui->lessSpinBox->move(ui->lessSpinBox->x(), 90);
    ui->greaterSpinBox->move(ui->greaterSpinBox->x(), 90);
    ui->withinSpinBox1->move(ui->withinSpinBox1->x(), 90);
    ui->withinSpinBox2->move(ui->withinSpinBox2->x(), 90);
    ui->outsideSpinBox1->move(ui->outsideSpinBox1->x(), 90);
    ui->outsideSpinBox2->move(ui->outsideSpinBox2->x(), 90);
    
    switch(filter.first){
        case LessFilter:
            if (filter.second.size() >= 1) ui->lessSpinBox->setValue(filter.second[0]);
            break;
            
        case GreaterFilter:
            if (filter.second.size() >= 1) ui->greaterSpinBox->setValue(filter.second[0]);
            break;
            
        case WithinRange:
            if (filter.second.size() >= 2){
                ui->withinSpinBox1->setValue(filter.second[0]);
                ui->withinSpinBox2->setValue(filter.second[1]);
            }
            break;
            
        case OutsideRange:
            if (filter.second.size() >= 2){
                ui->outsideSpinBox1->setValue(filter.second[0]);
                ui->outsideSpinBox2->setValue(filter.second[1]);
            }
            break;
        
        default:
            break;
    }
    
    
    
    ui->comboBox->setCurrentIndex((int)filter.first);
    changeUI((int)filter.first);
    
    connect(ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &FilterDialog::changeUI);
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

FilterDialog::~FilterDialog() {
    delete ui;
}


void FilterDialog::changeUI(int i){
    ui->lessLabel->setVisible(false);
    ui->greaterLabel->setVisible(false);
    ui->withinLabel->setVisible(false);
    ui->outsideLabel1->setVisible(false);
    ui->outsideLabel2->setVisible(false);
    ui->outsideLabel3->setVisible(false);
    
    
    ui->lessSpinBox->setVisible(false);
    ui->greaterSpinBox->setVisible(false);
    ui->withinSpinBox1->setVisible(false);
    ui->withinSpinBox2->setVisible(false);
    ui->outsideSpinBox1->setVisible(false);
    ui->outsideSpinBox2->setVisible(false);
    
    switch ((FeatureFilter)i){
        case LessFilter:
            ui->lessLabel->setVisible(true);
            ui->lessSpinBox->setVisible(true);
            break;
            
        case GreaterFilter:
            ui->greaterLabel->setVisible(true);
            ui->greaterSpinBox->setVisible(true);
            break;
            
        case WithinRange:
            ui->withinLabel->setVisible(true);
            ui->withinSpinBox1->setVisible(true);
            ui->withinSpinBox2->setVisible(true);
            break;
            
        case OutsideRange:
            ui->outsideLabel1->setVisible(true);
            ui->outsideLabel2->setVisible(true);
            ui->outsideLabel3->setVisible(true);
            ui->outsideSpinBox1->setVisible(true);
            ui->outsideSpinBox2->setVisible(true);
            break;
            
        default:
            break;
    }
}


void FilterDialog::ok(){
    filter.first = (FeatureFilter)ui->comboBox->currentIndex();
    filter.second.clear();
    switch(filter.first){
        case LessFilter:
            filter.second.push_back(ui->lessSpinBox->value());
            break;
            
        case GreaterFilter:
            filter.second.push_back(ui->greaterSpinBox->value());
            break;
            
        case WithinRange:
            filter.second.push_back(ui->withinSpinBox1->value());
            filter.second.push_back(ui->withinSpinBox2->value());
            break;
        
        case OutsideRange:
            filter.second.push_back(ui->outsideSpinBox1->value());
            filter.second.push_back(ui->outsideSpinBox2->value());
            break;
        
        default:
            break;
    }
    close();
}


void FilterDialog::cancel(){
    close();
}
