#include "lipidspace/studyvariablemapping.h"

StudyVariableMapping::StudyVariableMapping(FileTableHandler *fth, MappingData *md, vector<TableColumnType> *ct, LipidSpace *ls, QWidget *parent) : QDialog(parent), ui(new Ui::StudyVariableMapping) {
    ui->setupUi(this);
    file_table_handler = fth;
    lipid_space = ls;
    column_types = ct;
    mapping_data = md;
    registered_numerical_variables = 0;
    registered_nominal_variables = 0;
    original_import_variables = 0;
    inserted_variables = 0;

    setWindowTitle("Mapping of study variables");
    //setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    //setFixedSize(this->width(), this->height());

    connect(ui->continueButton, &QPushButton::clicked, this, &StudyVariableMapping::doContinue);
    connect(ui->cancelButton, &QPushButton::clicked, this, &StudyVariableMapping::cancel);
    connect(ui->insertButton, &QPushButton::clicked, this, &StudyVariableMapping::insertVariable);
    connect(ui->tableWidget, &KeyTableWidget::keyPressed, this, &StudyVariableMapping::tableKeyEvent);



    mapping_data->clear();
    mapping_data->insert({NumericalStudyVariable, map<string, Mapping>()});
    mapping_data->insert({NominalStudyVariable, map<string, Mapping>()});
    mapping_data->insert({NominalValue, map<string, Mapping>()});

    map<string, set<string>> nom_values;

    // TODO: check if ct size matches with header size

    // checking the study variables
    for (uint i = 0; i < ct->size(); ++i){
        if (ct->at(i) != StudyVariableColumnNumerical && ct->at(i) != StudyVariableColumnNominal) continue;

        string header = file_table_handler->headers[i];
        if (ct->at(i) == StudyVariableColumnNumerical){
            mapping_data->at(NumericalStudyVariable).insert({header, Mapping(header, NumericalStudyVariable)});
            continue;
        }


        // TODO: check if header is already registered in nom_values;
        nom_values.insert({header, set<string>()});
        set<string> &value_set = nom_values[header];

        for (auto row : file_table_handler->rows){
            value_set.insert(row[i]);
        }
    }

    for (auto kv : nom_values){
        mapping_data->at(NominalStudyVariable).insert({kv.first, Mapping(kv.first, NominalStudyVariable)});
        for (auto value : kv.second){
            string key = kv.first + "/" + value;
            mapping_data->at(NominalValue).insert({key, Mapping(value, NominalValue)});
            mapping_data->at(NominalValue)[key].parent = kv.first;
        }
    }

    // setting up the mapping table
    QTableWidget *t = ui->tableWidget;
    t->setRowCount(mapping_data->at(NumericalStudyVariable).size() + mapping_data->at(NominalStudyVariable).size() + mapping_data->at(NominalValue).size());
    t->setColumnCount(4);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->setSelectionBehavior( QAbstractItemView::SelectItems );
    t->setSelectionMode( QAbstractItemView::SingleSelection );

    QTableWidgetItem *item = new QTableWidgetItem("Study variable type");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(0, item);

    item = new QTableWidgetItem("Study variable name");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(1, item);

    item = new QTableWidgetItem("Mapping action");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(2, item);

    item = new QTableWidgetItem("Renaming");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(3, item);

    for (auto kv : lipid_space->study_variable_values){
        if (kv.second.variable_type == NumericalStudyVariable) registered_numerical_variables++;
        else registered_nominal_variables++;
    }

    // initial filling of the mapping table
    int row_cnt = 0;
    for (auto kv : mapping_data->at(NumericalStudyVariable)){
        item = new QTableWidgetItem("Numerical study_variable");
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 0, item);

        item = new QTableWidgetItem(kv.first.c_str());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 1, item);

        SignalCombobox *c = new SignalCombobox(t);
        c->addItem("No action");
        c->addItem("Rename");
        c->addItem("Register new with no default value");
        c->addItem("Register new with default value");
        c->setItemData(0, NumericalStudyVariable);
        c->setItemData(1, row_cnt);
        c->setItemData(2, 0);
        c->setItemData(3, 0);

        int var_cnt = 4;
        int match = -1;
        for (auto kv_f : lipid_space->study_variable_values){
            if (kv_f.second.variable_type != NumericalStudyVariable) continue;
            QString s = kv_f.second.name.c_str();
            if (QString(kv_f.second.name.c_str()).toLower() == QString(kv.first.c_str()).toLower()) match = var_cnt;

            c->addItem(QString("Map to %1").arg(s));
            c->setItemData(var_cnt++, s);
        }

        t->setCellWidget(row_cnt, 2, c);
        connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);

        SignalLineEdit *le = new SignalLineEdit(row_cnt, NumericalStudyVariable, t);
        le->setEnabled(false);
        connect(le, &SignalLineEdit::lineEditChanged, this, &StudyVariableMapping::lineEditChanged);
        t->setCellWidget(row_cnt++, 3, le);
        names_for_registration.push_back(kv.first);
        if (match > -1) c->setCurrentIndex(match);
    }
    // initial filling of the mapping table
    for (auto kv : mapping_data->at(NominalStudyVariable)){
        string type_name = "Nominal study_variable";

        item = new QTableWidgetItem(type_name.c_str());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 0, item);

        item = new QTableWidgetItem(kv.second.name.c_str());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 1, item);

        SignalCombobox *c = new SignalCombobox(t);
        c->addItem("No action");
        c->addItem("Rename to");
        c->addItem("Register new with no default value");
        c->addItem("Register new with default value");
        c->setItemData(0, NominalStudyVariable);
        c->setItemData(1, row_cnt);
        c->setItemData(2, 0);
        c->setItemData(3, 0);

        int var_cnt = 4;
        int match = -1;
        for (auto kv_f : lipid_space->study_variable_values){
            if (kv_f.second.variable_type != NominalStudyVariable || kv_f.first == FILE_STUDY_VARIABLE_NAME) continue;

            QString s = kv_f.second.name.c_str();
            if (QString(kv_f.second.name.c_str()).toLower() == QString(kv.first.c_str()).toLower()) match = var_cnt;
            c->addItem(QString("Map to %1").arg(s));
            c->setItemData(var_cnt++, s);
        }


        t->setCellWidget(row_cnt, 2, c);
        connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);

        SignalLineEdit *le = new SignalLineEdit(row_cnt, NominalStudyVariable, t);
        le->setEnabled(false);
        connect(le, &SignalLineEdit::lineEditChanged, this, &StudyVariableMapping::lineEditChanged);
        t->setCellWidget(row_cnt, 3, le);

        int variable_index = row_cnt++;
        variable_to_values.insert({variable_index, set<int>()});
        names_for_registration.push_back(kv.first);

        for (auto kv2 : mapping_data->at(NominalValue)){
            if (kv2.second.parent != kv.first) continue;
            if (contains_val(NA_VALUES, kv2.second.name)) continue;

            string type_name = " - Nominal value";
            item = new QTableWidgetItem(type_name.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setItem(row_cnt, 0, item);

            item = new QTableWidgetItem(kv2.second.name.c_str());
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            t->setItem(row_cnt, 1, item);

            SignalCombobox *c = new SignalCombobox(t);
            c->addItem("No action");
            c->addItem("Rename to");
            c->setItemData(0, NominalValue);
            c->setItemData(1, row_cnt);

            t->setCellWidget(row_cnt, 2, c);
            connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);

            SignalLineEdit *le = new SignalLineEdit(row_cnt, NominalValue, t);
            le->setEnabled(false);
            connect(le, &SignalLineEdit::lineEditChanged, this, &StudyVariableMapping::lineEditChanged);
            t->setCellWidget(row_cnt, 3, le);
            le->setText("");

            variable_to_values[variable_index].insert(row_cnt);
            value_to_variable.insert({row_cnt, variable_index});
            row_cnt++;
            names_for_registration.push_back(kv2.first);
        }

        if (match > -1) c->setCurrentIndex(match);
    }
    t->setRowCount(row_cnt);

    original_import_variables = row_cnt;
    ui->insertButton->setEnabled(registered_nominal_variables > 0 || registered_numerical_variables > 0);
}



void StudyVariableMapping::lineEditChanged(SignalLineEdit *edit, QString txt){
    int line_index = edit->row;
    string line_name = names_for_registration[line_index];
    StudyVariableType ft = edit->variable_type;
    mapping_data->at(ft)[line_name].mapping = txt.toStdString();
}




void StudyVariableMapping::tableKeyEvent(QKeyEvent *event){
    KeyTableWidget *t = ui->tableWidget;

    if (event->key() == Qt::Key_Delete){
        int current_row = t->currentRow();
        int current_col = t->currentColumn();

        if (current_row >= original_import_variables){
            inserted_variables--;
            t->removeRow(current_row);

            if (t->rowCount() > 0){
                current_row = min(current_row, t->rowCount() - 1);
                t->setCurrentCell(current_row, current_col);
            }

            if (inserted_variables > 0){
                for (int i = 0; i < inserted_variables; ++i){
                    int row = original_import_variables + i;
                    ((SignalCombobox*)t->cellWidget(row, 0))->row = row;
                    t->item(row, 1)->setText(QString("New study variable %1").arg(i + 1));
                    ((SignalCombobox*)t->cellWidget(row, 2))->row = row;
                    ((SignalLineEdit*)t->cellWidget(row, 3))->row = row;
                }
            }
        }
    }
}




void StudyVariableMapping::insertVariable(){
    KeyTableWidget *t = ui->tableWidget;
    int row = original_import_variables + inserted_variables++;
    t->setRowCount(t->rowCount() + 1);

    SignalCombobox *c = new SignalCombobox(t, row);

    c->row = row;
    if (registered_nominal_variables > 0){
        c->addItem("Nominal study variable");
        c->setItemData(c->count() - 1, NominalStudyVariable);

    }
    if (registered_numerical_variables > 0){
        c->addItem("Numerical study variable");
        c->setItemData(c->count() - 1, NumericalStudyVariable);
    }
    t->setCellWidget(row, 0, c);


    QTableWidgetItem *item = new QTableWidgetItem(QString("New study variable %1").arg(inserted_variables));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setItem(row, 1, item);


    SignalCombobox *cb = new SignalCombobox(t, row);
    t->setCellWidget(row, 2, cb);

    SignalLineEdit *le = new SignalLineEdit(row, NominalValue, t);
    le->setEnabled(false);
    t->setCellWidget(row, 3, le);
    le->setText("");

    connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboInsertedChanged);
    comboInsertedChanged(c);

    connect(cb, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboVariableChanged);
    comboVariableChanged(cb);
}




void StudyVariableMapping::comboInsertedChanged(SignalCombobox *combo){
    KeyTableWidget *t = ui->tableWidget;
    int combo_index = combo->currentIndex();
    int combo_row = combo->row;
    StudyVariableType ft = (StudyVariableType)combo->itemData(combo_index).toInt();

    SignalCombobox *cb = (SignalCombobox*)t->cellWidget(combo_row, 2);
    cb->clear();
    for (auto kv : lipid_space->study_variable_values){
        if (kv.second.variable_type == ft && kv.first != FILE_STUDY_VARIABLE_NAME){
            cb->addItem(QString("Map to '%1' with no default value").arg(kv.first.c_str()));
            cb->setItemData(cb->count() - 1, QString(kv.first.c_str()));
            cb->addItem(QString("Map to '%1' with default value").arg(kv.first.c_str()));
            cb->setItemData(cb->count() - 1, QString(kv.first.c_str()));
        }
    }

    ((SignalLineEdit*)t->cellWidget(combo_row, 3))->variable_type = ft;
}




void StudyVariableMapping::comboVariableChanged(SignalCombobox *combo){
    int combo_index = combo->currentIndex();
    int combo_row = combo->row;

    SignalLineEdit* edit = (SignalLineEdit*)ui->tableWidget->cellWidget(combo_row, 3);

    edit->setText("");
    edit->setEnabled(combo_index & 1);
}



void StudyVariableMapping::comboChanged(SignalCombobox *combo){
    int combo_index = combo->currentIndex();
    int row = combo->itemData(1).toInt();
    string combo_name = names_for_registration[row];
    StudyVariableType ft = (StudyVariableType)combo->itemData(0).toInt();
    SignalLineEdit *le = (SignalLineEdit*)ui->tableWidget->cellWidget(row, 3);
    le->variable_type = ft;
    le->setEnabled(combo_index == 1);
    le->setText("");
    combo->tool_tip = "";

    MappingAction action = NoAction;

    if (ft == NumericalStudyVariable){
        switch (combo_index){
            case 1: action = RenameAction; break;
            case 2: action = RegisterNewNaN; break;
            case 3: action = RegisterNewDefault; break;
            default: action = MappingTo; break;
        }

        mapping_data->at(NumericalStudyVariable)[combo_name].mapping = "";
        mapping_data->at(NumericalStudyVariable)[combo_name].action = action;
        if (action == MappingTo) mapping_data->at(NumericalStudyVariable)[combo_name].mapping = combo->itemData(combo_index).toString().toStdString();

        le->setEnabled(combo_index == 1 || combo_index == 3);
        le->setText("");
    }

    else if (ft == NominalStudyVariable){
        switch (combo_index){
            case 1: action = RenameAction; break;
            case 2: action = RegisterNewNaN; break;
            case 3: action = RegisterNewDefault; break;
            default: action = MappingTo; break;
        }

        mapping_data->at(NominalStudyVariable)[combo_name].mapping = "";
        mapping_data->at(NominalStudyVariable)[combo_name].action = action;
        if (action == MappingTo){
            string study_variable = combo->itemData(combo_index).toString().toStdString();
            mapping_data->at(NominalStudyVariable)[combo_name].mapping = study_variable;

            QStringList sl;
            if (contains_val(lipid_space->study_variable_values, study_variable)){
                for (auto value : lipid_space->study_variable_values[study_variable].nominal_values){
                    QString s = value.first.c_str();
                    sl << s;
                }
            }
            combo->tool_tip = QString("Currently registered values for '%1' are: %2").arg(study_variable.c_str()).arg(sl.join(", "));
        }

        le->setEnabled(combo_index == 1 || combo_index == 3);
        le->setText("");

        for (int value_index : variable_to_values[row]){
            SignalCombobox *sc = ((SignalCombobox*)ui->tableWidget->cellWidget(value_index, 2));
            int value_current_index = sc->currentIndex();
            if (value_current_index > 2) value_current_index = 0;
            disconnect(sc, &SignalCombobox::comboChanged, 0, 0);

            sc->clear();
            sc->addItem("No action");
            sc->addItem("Rename to");
            sc->setItemData(0, NominalValue);
            sc->setItemData(1, value_index);

            if (combo_index >= 4){
                int var_cnt = 2;
                string study_variable = combo->itemData(combo_index).toString().toStdString();
                if (contains_val(lipid_space->study_variable_values, study_variable)){
                    for (auto kv2 : lipid_space->study_variable_values[study_variable].nominal_values){
                        QString s = kv2.first.c_str();
                        sc->addItem(QString("Rename to %1").arg(s));
                        sc->setItemData(var_cnt++, s);
                    }
                }
            }


            connect(sc, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);
            sc->setCurrentIndex(value_current_index);
        }
    }

    else if (ft == NominalValue){
        switch (combo_index){
            case 1: action = RenameAction; break;
            default: action = MappingTo; break;
        }
        mapping_data->at(NominalValue)[combo_name].mapping = "";
        mapping_data->at(NominalValue)[combo_name].action = action;
        if (action == MappingTo){
            string nominal_value = combo->itemData(combo_index).toString().toStdString();
            mapping_data->at(NominalValue)[combo_name].mapping = nominal_value;
        }
    }
}


StudyVariableMapping::~StudyVariableMapping() {
    delete ui;
}


void StudyVariableMapping::doContinue() {
    set<string> numerical_registered;
    set<string> nominal_registered;
    set<string> numerical_import;
    set<string> nominal_import;

    KeyTableWidget *t = ui->tableWidget;

    // check if any default value for numerical study variable is not a number
    for (int r = 0; r < t->rowCount(); ++r){

        int combo_index = ((SignalCombobox*)t->cellWidget(r, 2))->currentIndex();
        if (!(((r < original_import_variables) && (combo_index == 3)) || ((r >= original_import_variables) && ((combo_index & 1) == 1)))) continue;

        SignalLineEdit *le = (SignalLineEdit*)t->cellWidget(r, 3);
        if (le->variable_type != NumericalStudyVariable) continue;
        QString le_text = le->text();

        bool is_number = false;
        le_text.toDouble(&is_number);
        if (!is_number){
            QString numerical_study_variable = t->item(r, 1)->text();

            if (r < original_import_variables){
                QMessageBox::warning(this, "Mismatch with study variables", QString("To register the numerical study variable '%1', all lipidomes in LipidSpace must get a default value for this variable. This default value must be a number.").arg(numerical_study_variable));
            }
            else {
                QMessageBox::warning(this, "Mismatch with study variables", QString("To register the numerical study variable '%1', it needs to get a default value. This default value must be a number.").arg(numerical_study_variable));
            }
            return;
        }
    }

    for (int i = 0; i < inserted_variables; ++i){
        int row = original_import_variables + i;

        SignalLineEdit* edit = (SignalLineEdit*)t->cellWidget(row, 3);

        if (edit->text() == "" && edit->isEnabled()){
            QMessageBox::warning(this, "Mismatch with study variables", QString("To register the nominal study variable '%1', it needs to get a default value. This default value must not be empty.").arg(t->item(row, 1)->text()));
            return;
        }
    }


    // adding registered study variables for match check
    for (auto kv : lipid_space->study_variable_values){
        if (kv.second.variable_type == NominalStudyVariable) nominal_registered.insert(kv.first);
        else numerical_registered.insert(kv.first);
    }
    if (contains_val(nominal_registered, FILE_STUDY_VARIABLE_NAME)) nominal_registered.erase(FILE_STUDY_VARIABLE_NAME);


    // adding numerical study variables to import for match check
    for (auto kv : mapping_data->at(NumericalStudyVariable)){
        if (kv.second.action == RegisterNewNaN) {
            continue;
        }
        else if (kv.second.action == RegisterNewDefault){
            if (kv.second.mapping == ""){
                QMessageBox::warning(this, "Mismatch with study variables", QString("To register the numerical study variable '%1', all lipidomes in LipidSpace must get a default value for this variable. This default value must not be empty.").arg(kv.second.name.c_str()));
                return;
            }
            continue;
        }
        string name = (kv.second.action == NoAction) ? kv.second.name : kv.second.mapping;

        if (name == ""){
            QMessageBox::warning(this, "Mismatch with study variables", QString("The new name for numerical study variable '%1' must not be empty.").arg(kv.second.name.c_str()));
            return;
        }

        if (contains_val(numerical_import, name)){
            QMessageBox::warning(this, "Mismatch with study variables", QString("It is not allowed to map two different study variables into one, here '%1'.").arg(name.c_str()));
            return;
        }
        numerical_import.insert(name);
    }

    // adding nominal study variables to import for match check
    for (auto kv : mapping_data->at(NominalStudyVariable)){
        if (kv.second.action == RegisterNewNaN) {
            continue;
        }
        else if (kv.second.action == RegisterNewDefault){
            if (kv.second.mapping == ""){
                QMessageBox::warning(this, "Mismatch with study variables", QString("To register the nominal study variable '%1', all lipidomes in LipidSpace must get a default value for this variable. This default value must not be empty.").arg(kv.second.name.c_str()));
                return;
            }
            continue;
        }
        string name = (kv.second.action == NoAction) ? kv.second.name : kv.second.mapping;
        if (name == ""){
            QMessageBox::warning(this, "Mismatch with study variables", QString("The new name for nominal study variable '%1' must not be empty.").arg(kv.second.name.c_str()));
            return;
        }

        if (contains_val(nominal_import, name)){
            QMessageBox::warning(this, "Mismatch with study variables", QString("It is not allowed to map two different study variables into one, here '%1'.").arg(name.c_str()));
            return;
        }
        nominal_import.insert(name);
    }


    // add inserted study variables
    for (int i = 0; i < inserted_variables; ++i){
        int row = original_import_variables + i;

        SignalCombobox* c = (SignalCombobox*)t->cellWidget(row, 0);
        StudyVariableType ft = (StudyVariableType)c->itemData(c->currentIndex()).toInt();

        SignalCombobox* cb = (SignalCombobox*)t->cellWidget(row, 2);
        string name = cb->itemData(cb->currentIndex()).toString().toStdString();

        if (ft == NumericalStudyVariable){
            if (contains_val(numerical_import, name)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("It is not allowed to map two different study variables into one, here '%1'.").arg(name.c_str()));
                return;
            }
            numerical_import.insert(name);
        }
        else if (ft == NominalStudyVariable){
            if (contains_val(nominal_import, name)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("It is not allowed to map two different study variables into one, here '%1'.").arg(name.c_str()));
                return;
            }
            nominal_import.insert(name);
        }
    }

    // match only study variables when LipidSpace has already registered study variables
    if (nominal_registered.size() > 0 || numerical_registered.size() > 0){
        for (auto name : numerical_import){
            if (contains_val(nominal_import, name)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("It is not allowed to map two different study variables into one, here '%1'.").arg(name.c_str()));
                return;
            }
        }


        for (auto kv : mapping_data->at(NominalValue)){
            if (kv.second.action != NoAction && kv.second.mapping == ""){
                QMessageBox::warning(this, "Mismatch with study variables", QString("For the nominal study variable '%1', the new value for '%2' must not be empty.").arg(kv.second.parent.c_str(), kv.second.name.c_str()));
                return;
            }
        }


        // checking match between registered and study variables for import
        for (auto nom_val : nominal_registered){
            if (uncontains_val(nominal_import, nom_val)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("No nominal study variable is yet mapped to '%1' registered in LipidSpace. Maybe insert a new study variable and map to '%1'.").arg(nom_val.c_str()));
                return;
            }
        }

        for (auto nom_val : nominal_import){
            if (uncontains_val(nominal_registered, nom_val)){
                QStringList sl;
                for (auto value : nominal_registered) sl << value.c_str();
                QMessageBox::warning(this, "Mismatch with study variables", QString("No nominal study variable \"%1\" is registered in LipidSpace. Registered nominal study variables are: %2").arg(nom_val.c_str()).arg(sl.join(", ")));
                return;
            }
        }

        for (auto num_val : numerical_registered){
            if (uncontains_val(numerical_import, num_val)){
                QMessageBox::warning(this, "Mismatch with study variables", QString("No numerical study variable is yet mapped to '%1' registered in LipidSpace. Maybe insert a new study variable and map to '%1'.").arg(num_val.c_str()));
                return;
            }
        }

        for (auto num_val : numerical_import){
            if (uncontains_val(numerical_registered, num_val)){
                QStringList sl;
                for (auto value : numerical_registered) sl << value.c_str();
                QMessageBox::warning(this, "Mismatch with study variables", QString("No numerical study variable \"%1\" is registered in LipidSpace. Registered numerical study variables are: %2").arg(num_val.c_str()).arg(sl.join(", ")));
                return;
            }
        }
    }

    // after everything is accepted, lets add the inserted study variables
    for (int i = 0; i < inserted_variables; ++i){
        int row = original_import_variables + i;

        SignalCombobox* c = (SignalCombobox*)t->cellWidget(row, 0);
        StudyVariableType ft = (StudyVariableType)c->itemData(c->currentIndex()).toInt();

        SignalCombobox* cb = (SignalCombobox*)t->cellWidget(row, 2);
        string name = cb->itemData(cb->currentIndex()).toString().toStdString();

        MappingAction action = (cb->currentIndex() & 1) ? InsertDefault : InsertNaN;

        mapping_data->at(ft).insert({name, Mapping(name, ft)});
        mapping_data->at(ft)[name].action = action;
        if (action == InsertDefault) mapping_data->at(ft)[name].mapping = ((SignalLineEdit*)t->cellWidget(row, 3))->text().toStdString();
    }

    accept();

}

void StudyVariableMapping::cancel() {
    reject();
}
