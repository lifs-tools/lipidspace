#include "lipidspace/about.h"


About::About(QWidget *parent, bool log_view) : QDialog(parent), ui(new Ui::About) {
    ui->setupUi(this);
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
        setFixedSize(width(), height());
        connect(ui->okButton, SIGNAL(clicked()), this, SLOT(ok()));

    if (log_view){
        setWindowTitle("Log messages");
        ui->textBrowser->append(Logging::get_log().c_str());
    }
    else {
        setWindowTitle("About");


        ui->textBrowser->append(("LipidSpace " + GlobalData::LipidSpace_version + " (MIT License)\n").c_str());
        ui->textBrowser->append("Main Contributers:");
        ui->textBrowser->append("Dominik Kopczynski");
        ui->textBrowser->append("Nils Hoffmann");
        ui->textBrowser->append("Robert Ahrends\n\n");

        ui->textBrowser->append("More information is available on:");
        ui->textBrowser->append("https://lipidomics.at\n\n");


        ui->textBrowser->append("LipidSpace is using third-party libriaries. All libraries are listed with their according licenses:");
        ui->textBrowser->append("- Qt 5 (LGPL license)");
        ui->textBrowser->append("- cppGoslin (MIT license)");
        ui->textBrowser->append("- openBlas (BSD license)");
        ui->textBrowser->append("- openXLSX (BSD license)");
        ui->textBrowser->append("- libAnova (MIT license)");
        ui->textBrowser->append("- lambda-lanczos (MIT license)\n\n");


        std::string file_content;
        std::ifstream stream_license("LICENSE");
        if (stream_license.good()){
            std::getline(stream_license, file_content, '\0');
            ui->textBrowser->append((file_content + "\n\n").c_str());
            ui->textBrowser->append("----------------------------------------------------------------------------\n\n");
        }



        file_content = "";
        std::ifstream stream_license_qt("LICENSE_QT");
        if (stream_license_qt.good()){
            std::getline(stream_license_qt, file_content, '\0');
            ui->textBrowser->append((file_content + "\n\n").c_str());
            ui->textBrowser->append("----------------------------------------------------------------------------\n\n");
        }


        file_content = "";
        std::ifstream stream_license_openblas("LICENSE_OPENBLAS");
        if (stream_license_openblas.good()){
            std::getline(stream_license_openblas, file_content, '\0');
            ui->textBrowser->append((file_content + "\n\n").c_str());
            ui->textBrowser->append("----------------------------------------------------------------------------\n\n");
        }


        file_content = "";
        std::ifstream stream_license_openxlsx("LICENSE_XLSX");
        if (stream_license_openxlsx.good()){
            std::getline(stream_license_openxlsx, file_content, '\0');
            ui->textBrowser->append((file_content + "\n\n").c_str());
            ui->textBrowser->append("----------------------------------------------------------------------------\n\n");
        }


        file_content = "";
        std::ifstream stream_license_ll("LICENSE_LL");
        if (stream_license_ll.good()){
            std::getline(stream_license_ll, file_content, '\0');
            ui->textBrowser->append((file_content + "\n\n").c_str());
        }

        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.setPosition(0);
        ui->textBrowser->setTextCursor(cursor);
    }
}

About::~About() {
    delete ui;
}


void About::ok(){
    close();
}
