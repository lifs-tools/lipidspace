#include "lipidspace/Tutorial.h"


Tutorial::Tutorial(QWidget *parent) : QFrame(parent) {
    setObjectName(QString::fromUtf8("TutorialFrame"));
    setGeometry(QRect(20, 110, 621, 251));
    setStyleSheet(QString::fromUtf8("QFrame#TutorialFrame {\n"
"	border: 7px solid #000000;\n"
"	background-color: rgb(255, 255, 255);\n"
"}"));
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setLineWidth(10);
    titleLabel = new QLabel(this);
    titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
    titleLabel->setGeometry(QRect(20, 20, 481, 61));
    QFont font2;
    font2.setPointSize(16);
    font2.setBold(true);
    font2.setWeight(75);
    titleLabel->setFont(font2);
    titleLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    titleLabel->setWordWrap(true);
    informationLabel = new QLabel(this);
    informationLabel->setObjectName(QString::fromUtf8("informationLabel"));
    informationLabel->setGeometry(QRect(20, 90, 581, 131));
    QFont font3;
    font3.setPointSize(12);
    informationLabel->setFont(font3);
    informationLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    informationLabel->setWordWrap(true);
    continuePushButton = new QPushButton(this);
    continuePushButton->setObjectName(QString::fromUtf8("continuePushButton"));
    continuePushButton->setGeometry(QRect(width() - 105, 50, 80, 26));
    continuePushButton->setStyleSheet(QString::fromUtf8(""));
    continuePushButton->setText(QApplication::translate("LipidSpaceGUI", "Continue", nullptr));
    pagesLabel = new QLabel(this);
    pagesLabel->setObjectName(QString::fromUtf8("pagesLabel"));
    pagesLabel->setGeometry(QRect(width() - 126, 220, 101, 20));
    pagesLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    xLabel = new ClickableLabel(this);
    xLabel->setObjectName(QString::fromUtf8("xLabel"));
    xLabel->setGeometry(QRect(550, 18, 51, 20));
    xLabel->move(width() - 25, 10);
    connect(xLabel, &ClickableLabel::clicked, this, &Tutorial::closeTutorial);
    
    QPixmap close_x("data/images/close-x.png");
    xLabel->setPixmap(close_x);
    xLabel->setAlignment(Qt::AlignLeading|Qt::AlignRight|Qt::AlignTop);
    QRect r = xLabel->geometry();
    r.setWidth(close_x.size().width());
    r.setHeight(close_x.size().height());
    xLabel->setGeometry(r);
    
    titleLabel->setText("Welcome");
    informationLabel->setText("Welcome to the first tutorial. We will guide you interactively through LipidSpace.");
    pagesLabel->setText("1 / 29");
}


Tutorial::~Tutorial() {
}


void Tutorial::closeTutorial(){
    if (QMessageBox::question(this, "Quit tutorial", "Do you want to quit the tutorial?") == QMessageBox::Yes){
        setVisible(false);
    }
}
