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
    ui->withinLabel1->move(ui->withinLabel1->x(), 92);
    ui->withinLabel2->move(ui->withinLabel2->x(), 92);
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
            
        case EqualFilter:
            if (!filter.second.empty()){
                QString line = QString::number(filter.second[0]);
                for (int i = 1; i < (int)filter.second.size(); ++i){
                    line += ", " + QString::number(filter.second[i]);
                }
                ui->equalEdit->setText(line);
            }
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
    ui->equalLabel->setVisible(false);
    ui->withinLabel1->setVisible(false);
    ui->withinLabel2->setVisible(false);
    ui->outsideLabel1->setVisible(false);
    ui->outsideLabel2->setVisible(false);
    ui->outsideLabel3->setVisible(false);
    
    ui->lessSpinBox->setVisible(false);
    ui->greaterSpinBox->setVisible(false);
    ui->equalEdit->setVisible(false);
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
            
        case EqualFilter:
            ui->equalLabel->setVisible(true);
            ui->equalEdit->setVisible(true);
            break;
            
        case WithinRange:
            ui->withinLabel1->setVisible(true);
            ui->withinLabel2->setVisible(true);
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
    vector<double> equal_values;
    if ((FeatureFilter)ui->comboBox->currentIndex() == EqualFilter){
        if (ui->equalEdit->text().size() == 0) {
            QMessageBox::warning(this, "Empty field", "An empty field is not permitted.");
            return;
        }
        
        for (int i = 0; i < (int)ui->equalEdit->text().size(); ++i){
            QChar c = ui->equalEdit->text().at(i);
            if (!(('0' <= c && c <= '9') || (c == ',') || (c == ' ') || (c == '.') || (c == '-'))){
                QString warning_message = QString("The character '") + c + QString("' is invalid. Only comma separated notation is valid, example:\n2, 1909.12, -4711.0815, 12345.432");
                QMessageBox::warning(this, "Invalid character", warning_message);
                return;
            }
        }
        vector<string> *tokens = split_string(ui->equalEdit->text().toStdString(), ',', '\'');
        for (string token : *tokens){
            string stripped_token = strip(token, ' ');
            bool valid_number;
            equal_values.push_back(QString(stripped_token.c_str()).toDouble(&valid_number));
            if (!valid_number){
                QMessageBox::warning(this, "Invalid number", "The string '" + QString(stripped_token.c_str()) + "' could not be converted into a number.");
                delete tokens;
                return;
            }
        }
        delete tokens;
    }
    
    
    filter.first = (FeatureFilter)ui->comboBox->currentIndex();
    filter.second.clear();
    
    switch(filter.first){
        case LessFilter:
            filter.second.push_back(ui->lessSpinBox->value());
            break;
            
        case GreaterFilter:
            filter.second.push_back(ui->greaterSpinBox->value());
            break;
            
        case EqualFilter:
            for (double val : equal_values) filter.second.push_back(val);
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
