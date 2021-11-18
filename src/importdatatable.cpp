#include "lipidspace/importdatatable.h"




ImportDataTable::ImportDataTable(QWidget *parent) : QDialog(parent), ui(new Ui::ImportDataTable) {
    ui->setupUi(this);
    setWindowTitle("Data table import");
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    
    ui->sampleListWidget->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidget->setSelectionMode(QListWidget::ExtendedSelection);
    ui->sampleListWidget->addFieldName("sample");
    
    ui->ignoreListWidget->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidget->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->lipidListWidget->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidget->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->featureListWidget->setDragDropMode(QAbstractItemView::DragDrop);
    ui->featureListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->featureListWidget->setSelectionMode(QListWidget::ExtendedSelection);
    
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->sampleListWidget, SIGNAL(oneItemViolation(string)), this, SLOT(oneItemViolated(string)));
    
    QString file_name = QFileDialog::getOpenFileName(this, "Select a lipid data table", ".", "Data Tables *.csv *.tsv *.xls (*.csv *.tsv *.xls)");
    if (!file_name.length()) close();
    
    data_table_file = file_name.toStdString();
    ifstream infile(data_table_file);
    if (!infile.good()){
        Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
        QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
        close();
    }
    
    
    // load all column headers and split them
    string column_headers;
    getline(infile, column_headers);
    
    if (column_headers.length() == 0){
        Logging::write_log("Error: first line in file '" + file_name.toStdString() + "' contains no data.");
        QMessageBox::critical(this, "Error", "Error: first line in file '" + file_name + "' contains no data.");
        close();
    }
    
    vector<string>* tokens = split_string(column_headers, ',', '"', true);
    
    
    int i = 0;
    for (string header : *tokens){
        QString qheader = header.c_str();
        original_column_index.insert({qheader, i++});
        QListWidgetItem* item = new QListWidgetItem();
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        item->setText(qheader);
        ui->ignoreListWidget->addItem(item);
    }
    
    delete tokens;
}



void ImportDataTable::oneItemViolated(string field_name){
    QMessageBox::warning(this, "Only one column", "Only one column can be assigned to the " + QString(field_name.c_str()) + " column");
}


ImportDataTable::~ImportDataTable() {
    delete ui;
}


void ImportDataTable::cancel(){
    close();
}

void ImportDataTable::ok(){
    if (ui->lipidListWidget->count() >= 2 && ui->sampleListWidget->count() == 1) {
        vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);
        
        column_types->at(original_column_index[ui->sampleListWidget->item(0)->text()]) = SampleColumn;
        
        for (int i = 0; i < (int)ui->lipidListWidget->count(); ++i){
            column_types->at(original_column_index[ui->lipidListWidget->item(i)->text()]) = LipidColumn;
        }
        
        for (int i = 0; i < (int)ui->featureListWidget->count(); ++i){
            column_types->at(original_column_index[ui->featureListWidget->item(i)->text()]) = FeatureColumn;
        }
        
        importTable(data_table_file, column_types);
        close();
    }
    else if (!ui->sampleListWidget->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select one column as sample columns.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select at least two columns as lipid columns.");
    }
}

