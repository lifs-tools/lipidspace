#ifndef TUTORIAL_H
#define TUTORIAL_H


#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QMessageBox>
#include <QtWidgets/QWidget>
#include "lipidspace/AssistanceFunctions.h"
#include <iostream>

using namespace std;




class Tutorial : public QFrame {
public:
    QFrame *tutorialFrame;
    QLabel *titleLabel;
    QLabel *informationLabel;
    QPushButton *continuePushButton;
    QLabel *pagesLabel;
    ClickableLabel *xLabel;
    
    Tutorial(QWidget *parent = 0);
    ~Tutorial();
    
public slots:
    void closeTutorial();
};

#endif /* TUTORIAL_H */
