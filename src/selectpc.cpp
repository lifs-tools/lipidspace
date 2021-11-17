#include "lipidspace/selectpc.h"
#include "ui_selectpc.h"

SelectPC::SelectPC(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectPC)
{
    ui->setupUi(this);
}

SelectPC::~SelectPC()
{
    delete ui;
}
