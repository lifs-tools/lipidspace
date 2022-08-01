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
            c->addItem(QString("Map to %1").arg(kv.second.name.c_str()));
            c->setItemData(var_cnt++, kv.second.name.c_str());
        }

        t->setCellWidget(row_cnt, 2, c);
        connect(c, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);

        SignalLineEdit *le = new SignalLineEdit(row_cnt, NumericalFeature, t);
        le->setEnabled(false);
        connect(le, &SignalLineEdit::lineEditChanged, this, &StudyVariableMapping::lineEditChanged);
        t->setCellWidget(row_cnt++, 3, le);
        names_for_registration.push_back(kv.first);
    }

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
        for (auto kv_f : lipid_space->feature_values){
            if (kv_f.second.feature_type != NominalFeature || kv_f.first == FILE_FEATURE_NAME) continue;
            c->addItem(QString("Map to %1").arg(kv_f.second.name.c_str()));
            c->setItemData(var_cnt++, kv_f.second.name.c_str());
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

            variable_to_values[variable_index].insert(row_cnt);
            value_to_variable.insert({row_cnt, variable_index});
            row_cnt++;
            names_for_registration.push_back(kv2.first);
        }
    }
}



void StudyVariableMapping::lineEditChanged(SignalLineEdit *edit, QString txt){
    int line_index = edit->row;
    string line_name = names_for_registration[line_index];
    FeatureType ft = edit->feature_type;
    string study_variable = txt.toStdString();
    mapping_data->at(ft)[line_name].rename = study_variable;
}



void StudyVariableMapping::comboChanged(SignalCombobox *combo){
    int combo_index = combo->currentIndex();
    int row = combo->itemData(1).toInt();
    string combo_name = names_for_registration[row];
    FeatureType ft = (FeatureType)combo->itemData(0).toInt();
    QLineEdit *le = (QLineEdit*)ui->tableWidget->cellWidget(row, 3);
    le->setEnabled(combo_index == 1);
    if (combo_index != 1) le->setText("");

    MappingAction action = NoAction;
    if (combo_index == 1) action = RenameAction;
    else if (combo_index > 1) action = MappingTo;

    if (ft == NumericalFeature){
        mapping_data->at(NumericalFeature)[combo_name].action = action;
        string study_variable = combo->itemData(combo_index).toString().toStdString();
        if (action == MappingTo) mapping_data->at(NumericalFeature)[combo_name].mapping = study_variable;
    }

    else if (ft == NominalFeature){
        mapping_data->at(NominalFeature)[combo_name].action = action;
        string study_variable = combo->itemData(combo_index).toString().toStdString();
        if (action == MappingTo) mapping_data->at(NominalFeature)[combo_name].mapping = study_variable;

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

            if (combo_index > 1){
                int var_cnt = 2;
                for (auto kv : lipid_space->feature_values[study_variable].nominal_values){
                    sc->addItem(QString("Map to %1").arg(kv.first.c_str()));
                    sc->setItemData(var_cnt++, kv.first.c_str());
                }
            }
            connect(sc, &SignalCombobox::comboChanged, this, &StudyVariableMapping::comboChanged);
            sc->setCurrentIndex(value_current_index);
        }
    }

    else if (ft == NominalValue){
        mapping_data->at(NominalValue)[combo_name].action = action;
        string study_variable = combo->itemData(combo_index).toString().toStdString();
        if (action == MappingTo) mapping_data->at(NominalValue)[combo_name].mapping = study_variable;
    }
}


StudyVariableMapping::~StudyVariableMapping() {
    delete ui;
}


void StudyVariableMapping::doContinue() {
    accept();
}

void StudyVariableMapping::cancel() {
    reject();
}
