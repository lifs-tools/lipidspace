#ifndef LIPIDSPACEGUI_H
#define LIPIDSPACEGUI_H

#include <QMainWindow>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class LipidSpaceGUI; }
QT_END_NAMESPACE

class LipidSpaceGUI : public QMainWindow
{
    Q_OBJECT

public:
    LipidSpaceGUI(QWidget *parent = nullptr);
    ~LipidSpaceGUI();
    
    
public slots:
    void quitProgram();
    void openLists();
    void openTable();

private:
    Ui::LipidSpaceGUI *ui;
};
#endif // LIPIDSPACEGUI_H
