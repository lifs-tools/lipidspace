#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <map>
#include <vector>
#include "lipidspace/AssistanceFunctions.h"

using namespace std;

namespace Ui {
class FilterDialog;
}

class FilterDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilterDialog(pair<FeatureFilter, vector<double>>& _filter, QWidget *parent = nullptr);
    ~FilterDialog();
    
public slots:
    void changeUI(int i);
    void ok();
    void cancel();

private:
    Ui::FilterDialog *ui;
    pair<FeatureFilter, vector<double>> &filter;
};

#endif // FILTERDIALOG_H
