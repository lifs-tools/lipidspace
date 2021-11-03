#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>
#include "ui_lipidspacegui.h"
#include "lipidspace/lipidspace.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class LipidSpaceGUI : public QMainWindow
{
    Q_OBJECT

public:
    LipidSpaceGUI(LipidSpace *_lipid_space, QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    
    
public slots:
    void quitProgram();
    void openLists();
    void openTable();

private:
    Ui::LipidSpaceGUI *ui;
    LipidSpace* lipid_space;
};
#endif // LIPIDSPACEGUI_H
