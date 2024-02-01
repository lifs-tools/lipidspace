#include "lipidspace/changecolordialog.h"
#include "ui_changecolordialog.h"



ChangeColorDialog::ChangeColorDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ChangeColorDialog){
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    lipid_space = ((LipidSpaceGUI*)parent)->lipid_space;


    connect(ui->okButton, &QPushButton::clicked, this, &ChangeColorDialog::ok);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ChangeColorDialog::cancel);

    ui->colorsTreeWidget->header()->resizeSection(0, 350);
    ui->colorsTreeWidget->header()->resizeSection(1, 10);
    ui->colorsTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);

    connect(ui->colorsTreeWidget, &QTreeWidget::itemDoubleClicked, this, &ChangeColorDialog::doubleClicked);

    QTreeWidgetItem *lipid_classes_item = new QTreeWidgetItem();
    lipid_classes_item->setText(0, QString("Lipid classes"));
    ui->colorsTreeWidget->addTopLevelItem(lipid_classes_item);

    for (auto &kv : GlobalData::colorMap){
        QTreeWidgetItem *item = new QTreeWidgetItem();
        items.insert({item, &kv.second});
        item->setText(0, kv.first.c_str());
        lipid_classes_item->addChild(item);
        item->setBackground(1, QColor(kv.second));
    }

    for (auto kv : lipid_space->study_variable_values){
        if (kv.second.study_variable_type != NominalStudyVariable) continue;

        QTreeWidgetItem *study_variable_item = new QTreeWidgetItem();
        study_variable_item->setText(0, kv.first.c_str());
        ui->colorsTreeWidget->addTopLevelItem(study_variable_item);

        string study_variable_prefix = kv.first + "_";
        for (auto &kv_nom : kv.second.nominal_values){
            string study_variable = study_variable_prefix + kv_nom.first;
            if (!contains_val(GlobalData::colorMapStudyVariables, study_variable)) continue;

            QTreeWidgetItem *item = new QTreeWidgetItem();
            items.insert({item, &GlobalData::colorMapStudyVariables.at(study_variable)});
            item->setText(0, kv_nom.first.c_str());
            study_variable_item->addChild(item);
            item->setBackground(1, QColor(GlobalData::colorMapStudyVariables.at(study_variable)));
        }
    }
}

ChangeColorDialog::~ChangeColorDialog(){
    delete ui;
}


void ChangeColorDialog::doubleClicked(QTreeWidgetItem *item, int){
    if (!contains_val(items, item)) return;

    QColorDialog dialog;
    dialog.setCurrentColor (item->background(1).color());
    dialog.setOption (QColorDialog::DontUseNativeDialog);

    if (dialog.exec() == QColorDialog::Accepted){
        item->setBackground(1, dialog.currentColor());
    }
}


void ChangeColorDialog::cancel(){
    reject();
}

void ChangeColorDialog::ok(){
    for (auto &row : items){
        QColor c = row.first->background(1).color();
        row.second->setRgb(c.red(), c.green(), c.blue());
    }
    accept();
}
