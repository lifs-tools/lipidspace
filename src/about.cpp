#include "lipidspace/about.h"

About::About(QWidget *parent) : QDialog(parent), ui(new Ui::About) {
    ui->setupUi(this);
    setWindowTitle("About");
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(width(), height());
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));
    
    
    ui->textBrowser->append("LipidSpace 0.0.1\n");
    ui->textBrowser->append("Main Contributers:");
    ui->textBrowser->append("Dominik Kopzcynski");
    ui->textBrowser->append("Robert Ahrends\n\n");
    
    std::string file_content;
    std::ifstream stream("LICENSE");
    if (stream.good()){
        std::getline(stream, file_content, '\0');
        ui->textBrowser->append(file_content.c_str());
    }
    
    
    
}

About::~About() {
    delete ui;
}


void About::ok(){
    close();
}
