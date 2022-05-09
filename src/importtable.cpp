#include "lipidspace/importtable.h"

ImportTable::ImportTable(QWidget *parent) : QDialog(parent), ui(new Ui::ImportTable) {
    ui->setupUi(this);
    setWindowTitle("Data table import");
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    sheet = "";
    
    // set sample lists
    ui->sampleListWidgetRow->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidgetRow->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidgetRow->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->sampleListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    ui->sampleListWidgetCol->addFieldName("sample");
    
    ui->sampleListWidgetFlat->setDragDropMode(QAbstractItemView::DragDrop);
    ui->sampleListWidgetFlat->setDefaultDropAction(Qt::MoveAction);
    ui->sampleListWidgetFlat->setSelectionMode(QListWidget::ExtendedSelection);
    ui->sampleListWidgetFlat->addFieldName("sample");
    ui->sampleListWidgetFlat->setNum(3);
    
    
    // set ignore lists
    ui->ignoreListWidgetRow->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidgetRow->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidgetRow->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->ignoreListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->ignoreListWidgetFlat->setDragDropMode(QAbstractItemView::DragDrop);
    ui->ignoreListWidgetFlat->setDefaultDropAction(Qt::MoveAction);
    ui->ignoreListWidgetFlat->setSelectionMode(QListWidget::ExtendedSelection);
    
    
    // set lipid lists
    ui->lipidListWidgetRow->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidgetRow->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidgetRow->setSelectionMode(QListWidget::ExtendedSelection);
    ui->lipidListWidgetRow->addFieldName("lipid");
    
    ui->lipidListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->lipidListWidgetFlat->setDragDropMode(QAbstractItemView::DragDrop);
    ui->lipidListWidgetFlat->setDefaultDropAction(Qt::MoveAction);
    ui->lipidListWidgetFlat->setSelectionMode(QListWidget::ExtendedSelection);
    ui->lipidListWidgetFlat->addFieldName("lipid");
    
    
    // set feature lists
    ui->numericalFeatureListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->numericalFeatureListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->numericalFeatureListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->nominalFeatureListWidgetCol->setDragDropMode(QAbstractItemView::DragDrop);
    ui->nominalFeatureListWidgetCol->setDefaultDropAction(Qt::MoveAction);
    ui->nominalFeatureListWidgetCol->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->numericalFeatureListWidgetFlat->setDragDropMode(QAbstractItemView::DragDrop);
    ui->numericalFeatureListWidgetFlat->setDefaultDropAction(Qt::MoveAction);
    ui->numericalFeatureListWidgetFlat->setSelectionMode(QListWidget::ExtendedSelection);
    
    ui->nominalFeatureListWidgetFlat->setDragDropMode(QAbstractItemView::DragDrop);
    ui->nominalFeatureListWidgetFlat->setDefaultDropAction(Qt::MoveAction);
    ui->nominalFeatureListWidgetFlat->setSelectionMode(QListWidget::ExtendedSelection);
    
    
    // set quant list
    ui->quantListWidgetFlat->setDragDropMode(QAbstractItemView::DragDrop);
    ui->quantListWidgetFlat->setDefaultDropAction(Qt::MoveAction);
    ui->quantListWidgetFlat->setSelectionMode(QListWidget::ExtendedSelection);
    ui->quantListWidgetFlat->addFieldName("quant");


    
    connect(ui->okButtonRow, SIGNAL(clicked()), this, SLOT(okRow()));
    connect(ui->okButtonCol, SIGNAL(clicked()), this, SLOT(okCol()));
    connect(ui->okButtonFlat, SIGNAL(clicked()), this, SLOT(okFlat()));
    
    connect(ui->cancelButtonRow, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->cancelButtonCol, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->cancelButtonFlat, SIGNAL(clicked()), this, SLOT(cancel()));
    
    connect(ui->sampleListWidgetCol, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->sampleListWidgetFlat, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->lipidListWidgetRow, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->lipidListWidgetFlat, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));
    connect(ui->quantListWidgetFlat, SIGNAL(oneItemViolation(string, int)), this, SLOT(oneItemViolated(string, int)));

    
    QString file_name = QFileDialog::getOpenFileName(this, "Select a lipid data table", GlobalData::last_folder, "Worksheets *.xlsx (*.xlsx);;Data Tables *.csv *.tsv *.xls (*.csv *.tsv *.xls)");
    if (!file_name.length()) {
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }
    
    
    QFileInfo fi(file_name);
    data_table_file = file_name.toStdString();
    string ext = fi.suffix().toLower().toStdString();
	
    
    if (ext == "xlsx"){
        try {
            SelectWorksheet select_worksheet(data_table_file, sheet, this);
            select_worksheet.setModal(true);
            select_worksheet.exec();
            if (sheet == ""){
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
                return;
            }
        }
        catch (exception &e){
            Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
            QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
            QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            return;
        }
    }
    
    
    GlobalData::last_folder = fi.absoluteDir().absolutePath();
    
    ifstream infile(data_table_file);
    if (!infile.good()){
        Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
        QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }
    
    
    
    QTableWidget *t = ui->tableWidget;
    int line_count = 1;
    t->setRowCount(0);
    int num_columns = 0;
    
    FileTableHandler *fth = 0;
    try{
        fth = new FileTableHandler(data_table_file, sheet);
    }
    catch (exception &e){
        if (fth) delete fth;
        cout << "Error: " << e.what() << endl;
        Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
        QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }
    
    // read the headers
    num_columns = fth->headers.size();
    t->setColumnCount(num_columns);
            
    int c = 0;
    map<QString, int> doublettes;
    for (string header : fth->headers){
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
        
        QListWidgetItem* item_flat = new QListWidgetItem();
        item_flat->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        item_flat->setText(qheader);
        ui->ignoreListWidgetFlat->addItem(item_flat);
    }
    
    // put all the other rows into the table
    for (auto tokens : fth->rows){
        t->setRowCount(line_count);
        
        c = 0;
        for (string value : tokens){
            QString qcell = value.length() ? value.c_str() : "";
            
            QTableWidgetItem *item = new QTableWidgetItem(qcell);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setItem(line_count - 1, c++, item);
        }
        for (; c < num_columns; c++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setItem(line_count - 1, c++, item);
        }
        if (++line_count == 11) break;
    }
    delete fth;
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
        
        importTable(data_table_file, column_types, ROW_PIVOT_TABLE, sheet);
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
        
        importTable(data_table_file, column_types, COLUMN_PIVOT_TABLE, sheet);
        close();
    }
    else if (!ui->sampleListWidgetCol->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select one column as sample column.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select at least two columns as lipid columns.");
    }
}




void ImportTable::okFlat(){
    if (ui->lipidListWidgetFlat->count() == 1 && ui->sampleListWidgetFlat->count() >= 1 && ui->quantListWidgetFlat->count() == 1) {
        vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);
        
        for (int i = 0; i < (int)ui->sampleListWidgetFlat->count(); ++i){
            column_types->at(original_column_index[ui->sampleListWidgetFlat->item(i)->text()]) = SampleColumn;
        }
        
        column_types->at(original_column_index[ui->lipidListWidgetFlat->item(0)->text()]) = LipidColumn;
        
        column_types->at(original_column_index[ui->quantListWidgetFlat->item(0)->text()]) = QuantColumn;
        
        for (int i = 0; i < (int)ui->numericalFeatureListWidgetFlat->count(); ++i){
            column_types->at(original_column_index[ui->numericalFeatureListWidgetFlat->item(i)->text()]) = FeatureColumnNumerical;
        }
        
        for (int i = 0; i < (int)ui->nominalFeatureListWidgetFlat->count(); ++i){
            column_types->at(original_column_index[ui->nominalFeatureListWidgetFlat->item(i)->text()]) = FeatureColumnNominal;
        }
        
        importTable(data_table_file, column_types, FLAT_TABLE, sheet);
        close();
    }
    else if (!ui->sampleListWidgetFlat->count()) {
        QMessageBox::warning(this, "No sample column selected", "Please select one column as sample column.");
    }
    else if (!ui->quantListWidgetFlat->count()) {
        QMessageBox::warning(this, "No quant column selected", "Please select one column as quant column.");
    }
    else {
        QMessageBox::warning(this, "No lipid column selected", "Please select one column as lipid column.");
    }
}
