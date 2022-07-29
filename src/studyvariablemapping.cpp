#include "lipidspace/studyvariablemapping.h"
#include "ui_studyvariablemapping.h"

StudyVariableMapping::StudyVariableMapping(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyVariableMapping)
{
    ui->setupUi(this);
}

StudyVariableMapping::~StudyVariableMapping()
{
    delete ui;
}
