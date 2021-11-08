#include "managelipidomes.h"
#include "ui_managelipidomes.h"

ManageLipidomes::ManageLipidomes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageLipidomes)
{
    ui->setupUi(this);
}

ManageLipidomes::~ManageLipidomes()
{
    delete ui;
}
