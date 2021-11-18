#include "lipidspace/importpivottable.h"
#include "ui_importpivottable.h"

ImportPivotTable::ImportPivotTable(QWidget *parent) : QDialog(parent), ui(new Ui::ImportPivotTable) {
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
    ui->lipidListWidget->addFieldName("lipid");
    
    ui->featureListWidget->setDragDropMode(QAbstractItemView::DragDrop);
    ui->featureListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->featureListWidget->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->quantListWidget->setDragDropMode(QAbstractItemView::DragDrop);
    ui->quantListWidget->setDefaultDropAction(Qt::MoveAction);
    ui->quantListWidget->setSelectionMode(QListWidget::ExtendedSelection);
    ui->quantListWidget->addFieldName("quant");
    
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->sampleListWidget, SIGNAL(oneItemViolation(string)), this, SLOT(oneItemViolated(string)));
    
    QString file_name = QFileDialog::getOpenFileName(this, "Select a lipid data table", ".", "Data Tables *.csv *.tsv *.xls (*.csv *.tsv *.xls)");
    if (!file_name.length()) {
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }
        
    
    data_table_file = file_name.toStdString();
    ifstream infile(data_table_file);
    if (!infile.good()){
        Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
        QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }
    
    
    // load all column headers and split them
    string column_headers;
    getline(infile, column_headers);
    
    if (column_headers.length() == 0){
        Logging::write_log("Error: first line in file '" + file_name.toStdString() + "' contains no data.");
        QMessageBox::critical(this, "Error", "Error: first line in file '" + file_name + "' contains no data.");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }
    
    vector<string>* tokens = split_string(column_headers, ',', '"', true);
    
    
    int i = 0;
    int empty = 1;
    for (string header : *tokens){
        QString qheader = header.length() ? header.c_str() : QStringLiteral("empty_field.%1").arg(empty++);
        original_column_index.insert({qheader, i++});
        QListWidgetItem* item = new QListWidgetItem();
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        item->setText(qheader);
        ui->ignoreListWidget->addItem(item);
    }
    
    delete tokens;
}

ImportPivotTable::~ImportPivotTable() {
    delete ui;
}



void ImportPivotTable::oneItemViolated(string field_name){
    QMessageBox::warning(this, "Only one column", "Only one column can be assigned to the " + QString(field_name.c_str()) + " column");
}



void ImportPivotTable::cancel(){
    close();
}

void ImportPivotTable::ok(){
    if (ui->lipidListWidget->count() == 1 && ui->sampleListWidget->count() == 1 && ui->quantListWidget->count() == 1) {
        vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);
        
        column_types->at(original_column_index[ui->sampleListWidget->item(0)->text()]) = SampleColumn;
        
        column_types->at(original_column_index[ui->lipidListWidget->item(0)->text()]) = LipidColumn;
        
        column_types->at(original_column_index[ui->quantListWidget->item(0)->text()]) = QuantColumn;
        
        for (int i = 0; i < (int)ui->featureListWidget->count(); ++i){
            column_types->at(original_column_index[ui->featureListWidget->item(i)->text()]) = FeatureColumn;
        }
        
        importTable(data_table_file, column_types);
        close();
    }
    else if (!ui->sampleListWidget->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select one column as sample column.");
    }
    else if (!ui->quantListWidget->count()) {
        QMessageBox::warning(this, "No quant column selected", "Please select one column as quant column.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select one column as lipid column.");
    }
}
