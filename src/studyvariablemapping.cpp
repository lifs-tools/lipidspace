#include "lipidspace/studyvariablemapping.h"

StudyVariableMapping::StudyVariableMapping(FileTableHandler *fth, vector<TableColumnType> *ct, LipidSpace *ls, QWidget *parent) : QDialog(parent), ui(new Ui::StudyVariableMapping) {
    ui->setupUi(this);
    file_table_handler = fth;
    lipid_space = ls;
    column_types = ct;

    setWindowTitle("Mapping of study variables");
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());

    connect(ui->continueButton, SIGNAL(clicked()), this, SLOT(doContinue()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));


    map< FeatureType, map<string, Mapping> > mappings;
    mappings.insert({NumericalFeature, map<string, Mapping>()});
    mappings.insert({NominalFeature, map<string, Mapping>()});

    map<string, set<string>> nom_values;

    // TODO: check if ct size matches with header size

    for (uint i = 0; i < ct->size(); ++i){
        if (ct->at(i) != FeatureColumnNumerical && ct->at(i) != FeatureColumnNominal) continue;

        string header = file_table_handler->headers[i];
        if (ct->at(i) == FeatureColumnNumerical){
            mappings[NumericalFeature].insert({header, Mapping(header)});
            continue;
        }


        // TODO: check if header is already registered in nom_values;
        nom_values.insert({header, set<string>()});
        set<string> &value_set = nom_values[header];

        for (string value : file_table_handler->rows[i]){
            value_set.insert(value);
        }
    }

    for (auto kv : nom_values){
        mappings[NominalFeature].insert({kv.first, Mapping(kv.first)});
        for (auto value : kv.second){
            string key = kv.first + "/" + value;
            mappings[NominalFeature].insert({key, Mapping(key)});
        }
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
