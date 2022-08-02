#include "lipidspace/studyvariablemapping.h"

StudyVariableMapping::StudyVariableMapping(FileTableHandler *fth, MappingData *md, vector<TableColumnType> *ct, LipidSpace *ls, QWidget *parent) : QDialog(parent), ui(new Ui::StudyVariableMapping) {
    ui->setupUi(this);
    file_table_handler = fth;
    lipid_space = ls;
    column_types = ct;
    mapping_data = md;

    setWindowTitle("Mapping of study variables");
    //setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    //setFixedSize(this->width(), this->height());

    connect(ui->continueButton, SIGNAL(clicked()), this, SLOT(doContinue()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));


    mapping_data->clear();
    mapping_data->insert({NumericalFeature, map<string, Mapping>()});
    mapping_data->insert({NominalFeature, map<string, Mapping>()});
    mapping_data->insert({NominalValue, map<string, Mapping>()});

    map<string, set<string>> nom_values;

    // TODO: check if ct size matches with header size

    // checking the study variables
    for (uint i = 0; i < ct->size(); ++i){
        if (ct->at(i) != FeatureColumnNumerical && ct->at(i) != FeatureColumnNominal) continue;

        string header = file_table_handler->headers[i];
        if (ct->at(i) == FeatureColumnNumerical){
            mapping_data->at(NumericalFeature).insert({header, Mapping(header, NumericalFeature)});
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
        mapping_data->at(NominalFeature).insert({kv.first, Mapping(kv.first, NominalFeature)});
        for (auto value : kv.second){
            string key = kv.first + "/" + value;
            mapping_data->at(NominalValue).insert({key, Mapping(value, NominalValue)});
            mapping_data->at(NominalValue)[key].parent = kv.first;
        }
    }

    // setting up the mapping table
    QTableWidget *t = ui->tableWidget;
    t->setRowCount(mapping_data->at(NumericalFeature).size() + mapping_data->at(NominalFeature).size() + mapping_data->at(NominalValue).size());
    t->setColumnCount(4);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QTableWidgetItem *item = new QTableWidgetItem("Study variable type");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(0, item);

    item = new QTableWidgetItem("Study variable Name");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(1, item);

    item = new QTableWidgetItem("Mapping action");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(2, item);

    item = new QTableWidgetItem("Renaming");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    t->setHorizontalHeaderItem(3, item);

    // initial filling of the mapping table
    int row_cnt = 0;
    for (auto kv : mapping_data->at(NumericalFeature)){
        item = new QTableWidgetItem("Numerical feature");
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 0, item);

        item = new QTableWidgetItem(kv.first.c_str());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 1, item);

        SignalCombobox *c = new SignalCombobox(t);
        c->addItem("No action");
        c->addItem("Rename");
        c->setItemData(0, NumericalFeature);
        c->setItemData(1, row_cnt);

        int var_cnt = 2;
        for (auto kv : lipid_space->feature_values){
            if (kv.second.feature_type != NumericalFeature) continue;
            QString s = kv.second.name.c_str();
            c->addItem(QString("Map to %1").arg(s));
            c->setItemData(var_cnt++, s);
        }

        t->setCellWidget(row_cnt, 2, c);
        connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);

        SignalLineEdit *le = new SignalLineEdit(row_cnt, NumericalFeature, t);
        le->setEnabled(false);
        connect(le, &SignalLineEdit::lineEditChanged, this, &StudyVariableMapping::lineEditChanged);
        t->setCellWidget(row_cnt++, 3, le);
        names_for_registration.push_back(kv.first);
    }
    // initial filling of the mapping table
    for (auto kv : mapping_data->at(NominalFeature)){
        string type_name = "Nominal feature";

        item = new QTableWidgetItem(type_name.c_str());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 0, item);

        item = new QTableWidgetItem(kv.second.name.c_str());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        t->setItem(row_cnt, 1, item);

        SignalCombobox *c = new SignalCombobox(t);
        c->addItem("No action");
        c->addItem("Rename to");
        c->setItemData(0, NominalFeature);
        c->setItemData(1, row_cnt);

        int var_cnt = 2;
        int match = -1;

        for (auto kv_f : lipid_space->feature_values){
            if (kv_f.second.feature_type != NominalFeature || kv_f.first == FILE_FEATURE_NAME) continue;

            QString s = kv_f.second.name.c_str();
            if (QString(kv_f.second.name.c_str()).toLower() == QString(kv.first.c_str()).toLower()) match = var_cnt;
            c->addItem(QString("Map to %1").arg(s));
            c->setItemData(var_cnt++, s);
        }


        t->setCellWidget(row_cnt, 2, c);
        connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);

        SignalLineEdit *le = new SignalLineEdit(row_cnt, NominalFeature, t);
        le->setEnabled(false);
        connect(le, &SignalLineEdit::lineEditChanged, this, &StudyVariableMapping::lineEditChanged);
        t->setCellWidget(row_cnt, 3, le);

        int variable_index = row_cnt++;
        variable_to_values.insert({variable_index, set<int>()});
        names_for_registration.push_back(kv.first);

        for (auto kv2 : mapping_data->at(NominalValue)){
            if (kv2.second.parent != kv.first) continue;

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
}



void StudyVariableMapping::lineEditChanged(SignalLineEdit *edit, QString txt){
    int line_index = edit->row;
    string line_name = names_for_registration[line_index];
    FeatureType ft = edit->feature_type;
    mapping_data->at(ft)[line_name].mapping = txt.toStdString();
}




void StudyVariableMapping::comboChanged(SignalCombobox *combo){



    int combo_index = combo->currentIndex();
    int row = combo->itemData(1).toInt();
    string combo_name = names_for_registration[row];
    FeatureType ft = (FeatureType)combo->itemData(0).toInt();
    QLineEdit *le = (QLineEdit*)ui->tableWidget->cellWidget(row, 3);
    le->setEnabled(combo_index == 1);
    le->setText("");
    combo->tool_tip = "";

    MappingAction action = NoAction;
    if (combo_index == 1) action = RenameAction;
    if (combo_index > 1) action = MappingTo;

    if (ft == NumericalFeature){
        mapping_data->at(NumericalFeature)[combo_name].mapping = "";
        mapping_data->at(NumericalFeature)[combo_name].action = action;
        if (action == MappingTo) mapping_data->at(NumericalFeature)[combo_name].mapping = combo->itemData(combo_index).toString().toStdString();
    }

    else if (ft == NominalFeature){
        mapping_data->at(NominalFeature)[combo_name].mapping = "";
        mapping_data->at(NominalFeature)[combo_name].action = action;
        string study_variable = combo->itemData(combo_index).toString().toStdString();
        if (action == MappingTo){
            mapping_data->at(NominalFeature)[combo_name].mapping = study_variable;

            QStringList sl;
            for (auto value : lipid_space->feature_values[study_variable].nominal_values){
                QString s = value.first.c_str();
                sl << s;
            }
            combo->tool_tip = QString("Currently registered values for '%1' are: %2").arg(study_variable.c_str()).arg(sl.join(", "));
        }

        for (int value_index : variable_to_values[row]){
            SignalCombobox *sc = ((SignalCombobox*)ui->tableWidget->cellWidget(value_index, 2));
            int value_current_index = sc->currentIndex();
            if (value_current_index > 1) value_current_index = 0;
            disconnect(sc, &SignalCombobox::comboChanged, 0, 0);

            sc->clear();
            sc->addItem("No action");
            sc->addItem("Rename to");
            sc->setItemData(0, NominalValue);
            sc->setItemData(1, value_index);

            if (combo_index >= 2){
                int var_cnt = 2;
                for (auto kv2 : lipid_space->feature_values[study_variable].nominal_values){
                    QString s = kv2.first.c_str();
                    sc->addItem(QString("Rename to %1").arg(s));
                    sc->setItemData(var_cnt++, s);
                }
            }


            connect(sc, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);
            sc->setCurrentIndex(value_current_index);
        }
    }

    else if (ft == NominalValue){
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

    // adding registered study variables for match check
    for (auto kv : lipid_space->feature_values){
        if (kv.second.feature_type == NominalFeature) nominal_registered.insert(kv.first);
        else numerical_registered.insert(kv.first);
    }
    if (contains_val(nominal_registered, FILE_FEATURE_NAME)) nominal_registered.erase(FILE_FEATURE_NAME);


    // adding study variables to import for match check
    for (auto kv : mapping_data->at(NumericalFeature)){
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
    for (auto kv : mapping_data->at(NominalFeature)){
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


    for (auto kv : mapping_data->at(NominalValue)){
        if (kv.second.action != NoAction && kv.second.mapping == ""){
            QMessageBox::warning(this, "Mismatch with study variables", QString("For the nominal study variable '%1', the new value for '%2' must not be empty.").arg(kv.second.parent.c_str(), kv.second.name.c_str()));
            return;
        }
    }


    // checking match between registered and study variables for import
    for (auto nom_val : nominal_registered){
        if (uncontains_val(nominal_import, nom_val)){
            QMessageBox::warning(this, "Mismatch with study variables", QString("No nominal study variable is yet mapped to '%1' registered in LipidSpace.").arg(nom_val.c_str()));
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
            QMessageBox::warning(this, "Mismatch with study variables", QString("No numerical study variable is yet mapped to '%1' registered in LipidSpace.").arg(num_val.c_str()));
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

    accept();

}

void StudyVariableMapping::cancel() {
    reject();
}
