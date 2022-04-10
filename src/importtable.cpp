#include "lipidspace/importtable.h"

ImportTable::ImportTable(QWidget *parent) : QDialog(parent), ui(new Ui::ImportTable) {
    ui->setupUi(this);
    setWindowTitle("Data table import");
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    
    // set sample lists
    ui->sampleListWidgetRow->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidgetRow->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidgetRow->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->sampleListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    ui->sampleListWidgetCol->addFieldName("sample");
    
    ui->sampleListWidgetPivot->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidgetPivot->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidgetPivot->setSelectionMode(QListWidget::ExtendedSelection);
    ui->sampleListWidgetPivot->addFieldName("sample");
    ui->sampleListWidgetPivot->setNum(3);
    
    
    // set ignore lists
    ui->ignoreListWidgetRow->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidgetRow->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidgetRow->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->ignoreListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->ignoreListWidgetPivot->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidgetPivot->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidgetPivot->setSelectionMode(QListWidget::ExtendedSelection);
    
    
    // set lipid lists
    ui->lipidListWidgetRow->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidgetRow->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidgetRow->setSelectionMode(QListWidget::ExtendedSelection);
    ui->lipidListWidgetRow->addFieldName("lipid");
    
    ui->lipidListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->lipidListWidgetPivot->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidgetPivot->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidgetPivot->setSelectionMode(QListWidget::ExtendedSelection);
    ui->lipidListWidgetPivot->addFieldName("lipid");
    
    
    // set feature lists
    ui->numericalFeatureListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->numericalFeatureListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->numericalFeatureListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->nominalFeatureListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->nominalFeatureListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->nominalFeatureListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->numericalFeatureListWidgetPivot->setDragDropMode(QAbstractItemView::DragDrop);
    ui->numericalFeatureListWidgetPivot->setDefaultDropAction(Qt::MoveAction);
    ui->numericalFeatureListWidgetPivot->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->nominalFeatureListWidgetPivot->setDragDropMode(QAbstractItemView::DragDrop);
    ui->nominalFeatureListWidgetPivot->setDefaultDropAction(Qt::MoveAction);
    ui->nominalFeatureListWidgetPivot->setSelectionMode(QListWidget::ExtendedSelection);
    
    
    // set quant list
    ui->quantListWidgetPivot->setDragDropMode(QAbstractItemView::DragDrop);
    ui->quantListWidgetPivot->setDefaultDropAction(Qt::MoveAction);
    ui->quantListWidgetPivot->setSelectionMode(QListWidget::ExtendedSelection);
    ui->quantListWidgetPivot->addFieldName("quant");


    
    connect(ui->okButtonRow, SIGNAL(clicked()), this, SLOT(okRow()));
    connect(ui->okButtonCol, SIGNAL(clicked()), this, SLOT(okCol()));
    connect(ui->okButtonPivot, SIGNAL(clicked()), this, SLOT(okPivot()));
    
    connect(ui->cancelButtonRow, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->cancelButtonCol, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->cancelButtonPivot, SIGNAL(clicked()), this, SLOT(cancel()));
    
    connect(ui->sampleListWidgetCol, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->sampleListWidgetPivot, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->lipidListWidgetRow, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->lipidListWidgetPivot, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->quantListWidgetPivot, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));

    
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
    string file_line;
    
    QTableWidget *t = ui->tableWidget;
    int line_count = 0;
    t->setRowCount(0);
    int num_columns = 0;
    while(getline(infile, file_line)){
        // read the headers
        if (line_count == 0){
            if (file_line.length() == 0){
                Logging::write_log("Error: first line in file '" + file_name.toStdString() + "' contains no data.");
                QMessageBox::critical(this, "Error", "Error: first line in file '" + file_name + "' contains no data.");
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
                return;
            }
            
            vector<string>* tokens = split_string(file_line, ',', '"', true);
            num_columns = tokens->size();
            t->setColumnCount(num_columns);
            int c = 0;
            map<QString, int> doublettes;
            for (string header : *tokens){
                if (header[0] == '"' && header[header.length() - 1] == '"') header = strip(header, '"');
                QString qheader = header.length() ? header.c_str() : "empty_field";
                
                if (uncontains_val(doublettes, qheader)){
                    doublettes.insert({qheader, 1});
                }
                else {
                    qheader += "." + QString::number(++doublettes[qheader]);
                }
                
                original_column_index.insert({qheader, c});
                
                QTableWidgetItem *item = new QTableWidgetItem(qheader);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                t->setHorizontalHeaderItem(c++, item);
                
                QListWidgetItem* item_row = new QListWidgetItem();
                item_row->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
                item_row->setText(qheader);
                ui->ignoreListWidgetRow->addItem(item_row);
                
                QListWidgetItem* item_col = new QListWidgetItem();
                item_col->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
                item_col->setText(qheader);
                ui->ignoreListWidgetCol->addItem(item_col);
                
                QListWidgetItem* item_pivot = new QListWidgetItem();
                item_pivot->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
                item_pivot->setText(qheader);
                ui->ignoreListWidgetPivot->addItem(item_pivot);
            }
            delete tokens;
        }
        
        // put all the other rows into the table
        else {
            
            t->setRowCount(line_count);
            vector<string>* tokens = split_string(file_line, ',', '"', true);
            
            if ((int)tokens->size() > num_columns){
                Logging::write_log("Error: line '" + std::to_string(line_count + 1) + "' has more cells than first line in file '" + file_name.toStdString() + "'.");
                QMessageBox::critical(this, "Error", QStringLiteral("Error: line '%1' has more cells than first line in file '%2'.").arg(line_count + 1).arg(file_name));
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
                delete t;
                return;
            }
            int c = 0;
            for (string header : *tokens){
                if (header[0] == '"' && header[header.length() - 1] == '"') header = strip(header, '"');
                QString qcell = header.length() ? header.c_str() : "";
                
                QTableWidgetItem *item = new QTableWidgetItem(qcell);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                t->setItem(line_count - 1, c++, item);
            }
            for (; c < num_columns; c++){
                QTableWidgetItem *item = new QTableWidgetItem("");
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                t->setItem(line_count - 1, c++, item);
            }
            
            delete tokens;
        }
        
        if (++line_count == 11) break;
    }
}

ImportTable::~ImportTable() {
    delete ui;
}



void ImportTable::oneItemViolated(string field_name, int num){
    QMessageBox::warning(this, "Only one column", "Only " + QString::number(num) + " column(s) can be assigned to the " + QString(field_name.c_str()) + " column");
}



void ImportTable::cancel(){
    close();
}





void ImportTable::okRow(){
    if (ui->lipidListWidgetRow->count() == 1 && ui->sampleListWidgetRow->count() >= 1) {
        vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);
        
        for (int i = 0; i < (int)ui->sampleListWidgetRow->count(); ++i){
            column_types->at(original_column_index[ui->sampleListWidgetRow->item(i)->text()]) = SampleColumn;
        }
        
        column_types->at(original_column_index[ui->lipidListWidgetRow->item(0)->text()]) = LipidColumn;
        
        importTable(data_table_file, column_types, ROW_TABLE);
        close();
    }
    else if (!ui->sampleListWidgetRow->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select at least one column as sample column.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select one column as lipid column.");
    }
}



void ImportTable::okCol(){
    if (ui->lipidListWidgetCol->count() >= 2 && ui->sampleListWidgetCol->count() == 1) {
        vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);
        
        column_types->at(original_column_index[ui->sampleListWidgetCol->item(0)->text()]) = SampleColumn;
        
        for (int i = 0; i < (int)ui->lipidListWidgetCol->count(); ++i){
            column_types->at(original_column_index[ui->lipidListWidgetCol->item(i)->text()]) = LipidColumn;
        }
        
        for (int i = 0; i < (int)ui->numericalFeatureListWidgetCol->count(); ++i){
            column_types->at(original_column_index[ui->numericalFeatureListWidgetCol->item(i)->text()]) = FeatureColumnNumerical;
        }
        
        for (int i = 0; i < (int)ui->nominalFeatureListWidgetCol->count(); ++i){
            column_types->at(original_column_index[ui->nominalFeatureListWidgetCol->item(i)->text()]) = FeatureColumnNominal;
        }
        
        importTable(data_table_file, column_types, COLUMN_TABLE);
        close();
    }
    else if (!ui->sampleListWidgetCol->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select one column as sample column.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select at least two columns as lipid columns.");
    }
}




void ImportTable::okPivot(){
    if (ui->lipidListWidgetPivot->count() == 1 && ui->sampleListWidgetPivot->count() >= 1 && ui->quantListWidgetPivot->count() == 1) {
        vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);
        
        for (int i = 0; i < (int)ui->sampleListWidgetPivot->count(); ++i){
            column_types->at(original_column_index[ui->sampleListWidgetPivot->item(i)->text()]) = SampleColumn;
        }
        
        column_types->at(original_column_index[ui->lipidListWidgetPivot->item(0)->text()]) = LipidColumn;
        
        column_types->at(original_column_index[ui->quantListWidgetPivot->item(0)->text()]) = QuantColumn;
        
        for (int i = 0; i < (int)ui->numericalFeatureListWidgetPivot->count(); ++i){
            column_types->at(original_column_index[ui->numericalFeatureListWidgetPivot->item(i)->text()]) = FeatureColumnNumerical;
        }
        
        for (int i = 0; i < (int)ui->nominalFeatureListWidgetPivot->count(); ++i){
            column_types->at(original_column_index[ui->nominalFeatureListWidgetPivot->item(i)->text()]) = FeatureColumnNominal;
        }
        
        importTable(data_table_file, column_types, PIVOT_TABLE);
        close();
    }
    else if (!ui->sampleListWidgetPivot->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select one column as sample column.");
    }
    else if (!ui->quantListWidgetPivot->count()) {
        QMessageBox::warning(this, "No quant column selected", "Please select one column as quant column.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select one column as lipid column.");
    }
}
