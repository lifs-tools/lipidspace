#include "lipidspace/importtable.h"

ImportTable::ImportTable(QWidget *parent) : QDialog(parent), ui(new Ui::ImportTable) {
    lipid_space = 0;
    file_table_handler = 0;
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

    mapping_of_study_variables = false;

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

    connect(ui->checkBoxMappingCol, &QCheckBox::stateChanged, this, &ImportTable::checkBoxCol);
    connect(ui->checkBoxMappingFlat, &QCheckBox::stateChanged, this, &ImportTable::checkBoxFlat);

    file_name = "";
}


void ImportTable::checkBoxCol(int state){
    mapping_of_study_variables = (state == Qt::Checked);
    ui->checkBoxMappingFlat->setCheckState((Qt::CheckState)state);
}


void ImportTable::checkBoxFlat(int state){
    mapping_of_study_variables = (state == Qt::Checked);
    ui->checkBoxMappingCol->setCheckState((Qt::CheckState)state);
}


void ImportTable::show(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
    mapping_of_study_variables = false;
    ui->checkBoxMappingFlat->setCheckState(Qt::Unchecked);
    ui->checkBoxMappingCol->setCheckState(Qt::Unchecked);

    ui->sampleListWidgetRow->clear();
    ui->sampleListWidgetCol->clear();
    ui->sampleListWidgetFlat->clear();
    ui->ignoreListWidgetRow->clear();
    ui->ignoreListWidgetCol->clear();
    ui->ignoreListWidgetFlat->clear();
    ui->lipidListWidgetRow->clear();
    ui->lipidListWidgetCol->clear();
    ui->lipidListWidgetFlat->clear();
    ui->numericalFeatureListWidgetCol->clear();
    ui->nominalFeatureListWidgetCol->clear();
    ui->numericalFeatureListWidgetFlat->clear();
    ui->nominalFeatureListWidgetFlat->clear();
    ui->quantListWidgetFlat->clear();
    ui->tableWidget->setColumnCount(0);
    ui->tabWidget->setCurrentIndex(0);
    sheet = "";

    file_name = QFileDialog::getOpenFileName(this, "Select a lipid data table", GlobalData::last_folder, "Worksheets *.xlsx (*.xlsx);;Data Tables *.csv *.tsv (*.csv *.tsv)");
    if (!file_name.length()) {
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }


    QFileInfo fi(file_name);
    data_table_file = file_name.toStdString();

    ifstream infile(data_table_file);
    if (!infile.good()){
        Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
        QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }



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

    QTableWidget *t = ui->tableWidget;
    int line_count = 1;
    t->setRowCount(0);
    int num_columns = 0;

    try{
        if (file_table_handler != 0){
            delete file_table_handler;
            file_table_handler = 0;
        }
        file_table_handler = new FileTableHandler(data_table_file, sheet);
    }
    catch (exception &e){
        if (file_table_handler){
            delete file_table_handler;
            file_table_handler = 0;
        }
        Logging::write_log("Error: file '" + data_table_file + "' cannot be opened.");
        QMessageBox::critical(this, "Error", "Error: file '" + file_name + "' cannot be opened.");
        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return;
    }

    // read the headers
    num_columns = file_table_handler->headers.size();
    t->setColumnCount(num_columns);

    int c = 0;
    map<QString, int> doublettes;
    for (string header : file_table_handler->headers){
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
    for (auto tokens : file_table_handler->rows){
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

    QDialog::show();
    importOpened();
}

ImportTable::~ImportTable() {
    delete ui;
    delete file_table_handler;
}



void ImportTable::oneItemViolated(string field_name, int num){
    QMessageBox::warning(this, "Only one column", "Only " + QString::number(num) + " column(s) can be assigned to the " + QString(field_name.c_str()) + " column");
}



void ImportTable::cancel(){
    done(0);
}



bool ImportTable::checkStudyVariables(){
    if (lipid_space == 0 || lipid_space->lipidomes.size() == 0) return true;

    if (lipid_space->feature_values.size() > 0 && ui->tabWidget->currentIndex() == 0){
        QMessageBox::warning(this, "Mismatch with study variables", "Your table type does not support study varibles but you already have registered study variables. Therefore, an import is not possible.");
        return false;
    }

    QListWidget *numerical_list = 0;
    QListWidget *nominal_list = 0;

    if (ui->tabWidget->currentIndex() == 1){
        numerical_list = ui->numericalFeatureListWidgetCol;
        nominal_list = ui->nominalFeatureListWidgetCol;
    }
    else if (ui->tabWidget->currentIndex() == 2){
        numerical_list = ui->numericalFeatureListWidgetFlat;
        nominal_list = ui->nominalFeatureListWidgetFlat;
    }

    int num_numerical = 0;
    int num_nominal = 0;

    for (auto kv : lipid_space->feature_values){
        if (kv.second.feature_type == NominalFeature) num_nominal++;
        else num_numerical++;
    }
    num_nominal = max(0, num_nominal - 1); // -1 because of internal 'Origin' variable


    if (num_numerical != numerical_list->count()){
        QMessageBox::warning(this, "Mismatch with study variables", QString("Your number of numerical study varibles (%1) does not match with the number of numerical study variables already registered in LipidSpace (%2).").arg(numerical_list->count()).arg(num_numerical));
        return false;
    }

    if (num_nominal != nominal_list->count()){
        QMessageBox::warning(this, "Mismatch with study variables", QString("Your number of nominal study varibles (%1) does not match with the number of nominal study variables already registered in LipidSpace (%2).").arg(nominal_list->count()).arg(num_nominal));
        return false;
    }

    if (!mapping_of_study_variables){
        set<string> numerical_registered;
        set<string> nominal_registered;
        set<string> numerical_import;
        set<string> nominal_import;

        for (auto kv : lipid_space->feature_values){
            if (kv.second.feature_type == NominalFeature) nominal_registered.insert(kv.first);
            else numerical_registered.insert(kv.first);
        }
        if (contains_val(nominal_registered, FILE_FEATURE_NAME)) nominal_registered.erase(FILE_FEATURE_NAME);

        for (int i = 0; i < numerical_list->count(); ++i) numerical_import.insert(numerical_list->item(i)->text().toStdString());
        for (int i = 0; i < nominal_list->count(); ++i) nominal_import.insert(nominal_list->item(i)->text().toStdString());

        for (auto num_val : nominal_registered){
            if (uncontains_val(nominal_import, num_val)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("Registered nominal study variable \"%1\" is not selected. Maybe you might turn on the 'Mapping of study variables'!").arg(num_val.c_str()));
                return false;
            }
        }

        for (auto num_val : nominal_import){
            if (uncontains_val(nominal_registered, num_val)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("No nominal study variable \"%1\" is not registered already. Maybe you might turn on the 'Mapping of study variables'!").arg(num_val.c_str()));
                return false;
            }
        }

        for (auto num_val : numerical_registered){
            if (uncontains_val(numerical_import, num_val)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("Registered numerical study variable \"%1\" is not selected. Maybe you might turn on the 'Mapping of study variables'!").arg(num_val.c_str()));
                return false;
            }
        }

        for (auto num_val : numerical_import){
            if (uncontains_val(numerical_registered, num_val)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("No numerical study variable \"%1\" is not registered already. May you might turn on the 'Mapping of study variables'!").arg(num_val.c_str()));
                return false;
            }
        }

    }

    return true;
}





void ImportTable::okRow(){
    if (ui->lipidListWidgetRow->count() == 1 && ui->sampleListWidgetRow->count() >= 1) {
        if (checkStudyVariables()){
            vector<TableColumnType> *column_types = new vector<TableColumnType>(original_column_index.size(), IgnoreColumn);

            for (int i = 0; i < (int)ui->sampleListWidgetRow->count(); ++i){
                column_types->at(original_column_index[ui->sampleListWidgetRow->item(i)->text()]) = SampleColumn;
            }

            column_types->at(original_column_index[ui->lipidListWidgetRow->item(0)->text()]) = LipidColumn;

            accept();
            importTable(data_table_file, column_types, ROW_PIVOT_TABLE, sheet);
        }
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

        if (checkStudyVariables()){
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

            if (mapping_of_study_variables){
                MappingData *mapping_data = new MappingData();
                StudyVariableMapping study_variable_mapping(file_table_handler, mapping_data, column_types, lipid_space, this);
                study_variable_mapping.setModal(true);
                study_variable_mapping.exec();
                if (study_variable_mapping.result() == QDialog::Accepted){
                    accept();
                    importTable(data_table_file, column_types, COLUMN_PIVOT_TABLE, sheet, mapping_data);
                }
            }
            else {
                accept();
                importTable(data_table_file, column_types, COLUMN_PIVOT_TABLE, sheet);
            }
        }
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

        if (checkStudyVariables()){
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

            if (mapping_of_study_variables){
                MappingData *mapping_data = new MappingData();
                StudyVariableMapping study_variable_mapping(file_table_handler, mapping_data, column_types, lipid_space, this);
                study_variable_mapping.setModal(true);
                study_variable_mapping.exec();
                if (study_variable_mapping.result() == QDialog::Accepted){
                    accept();
                    importTable(data_table_file, column_types, FLAT_TABLE, sheet, mapping_data);
                }
            }
            else {
                accept();
                importTable(data_table_file, column_types, FLAT_TABLE, sheet);
            }
        }
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
