#include "lipidspace/progressbar.h"

Progressbar::Progressbar(QWidget *parent) : QDialog(parent), ui(new Ui::Progressbar){
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(0);
    setWindowTitle("LipidSpace Analysis");
    connect(ui->abortButton, SIGNAL(clicked()), this, SLOT(abort()));
}

Progressbar::~Progressbar(){
    delete ui;
}


void Progressbar::abort(){
    closeWindow(false);
}

void Progressbar::interruptClose(){
    accept();
}


void Progressbar::reject(){
    closeWindow(false);
}

void Progressbar::finish(){
    closeWindow(true);
}


void Progressbar::set_max(int m){
    ui->progressBar->setMaximum(m);
}


void Progressbar::set_current(int c){
    ui->progressBar->setValue(c);
}

void Progressbar::closeWindow(bool successful){
    if (successful){
        refreshCanvas();
    }
    else {
        interrupt();
    }
    accept();
}
