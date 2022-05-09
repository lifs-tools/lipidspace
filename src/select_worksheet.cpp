#include "lipidspace/select_worksheet.h"

SelectWorksheet::SelectWorksheet(string file_name, string &_sheet, QWidget *parent) : QDialog(parent), sheet(_sheet), ui(new Ui::SelectWorksheet) {
    ui->setupUi(this);
    sheet = "";
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(width(), height());
    
    XLDocument doc(file_name.c_str());
   
    auto wb = doc.workbook();
    for (const auto& name : wb.worksheetNames()) ui->comboBox->addItem(name.c_str());
    
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

SelectWorksheet::~SelectWorksheet(){
    delete ui;
}


void SelectWorksheet::cancel(){
    close();
}

void SelectWorksheet::ok(){
    sheet = ui->comboBox->currentText().toStdString();
    close();
}
