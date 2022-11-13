#include "lipidspace/selectlipidomes.h"
#include "ui_selectlipidomes.h"

SelectLipidomes::SelectLipidomes(QWidget *parent) : QDialog(parent), ui(new Ui::SelectLipidomes) {
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    updating_states = false;

    connect(ui->okButton, &QPushButton::clicked, this, &SelectLipidomes::ok);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SelectLipidomes::cancel);
}


void SelectLipidomes::init(){
    updating_states = false;
    ui->lipidomesListWidget->clear();
    for (auto canvas : ((LipidSpaceGUI*)parentWidget())->canvases){
        QListWidgetItem *item = new QListWidgetItem(canvas->pointSet->title);
        item->setCheckState(canvas->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);
        ui->lipidomesListWidget->addItem(item);
    }

    ui->lipidomesTreeWidget->clear();
    for (auto canvas : ((LipidSpaceGUI*)parentWidget())->canvases){
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, canvas->pointSet->title);
        item->setCheckState(0, canvas->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);


        QTreeWidgetItem *test = new QTreeWidgetItem();
        test->setText(0, "foo");
        test->setCheckState(0, Qt::Unchecked);
        QTreeWidgetItem *test2 = new QTreeWidgetItem();
        test2->setText(0, "foo");
        test2->setCheckState(0, Qt::Unchecked);

        item->addChild(test);
        item->addChild(test2);

        connect(ui->lipidomesTreeWidget, &QTreeWidget::itemChanged, this, &SelectLipidomes::foo);

        ui->lipidomesTreeWidget->addTopLevelItem(item);
    }
}


void SelectLipidomes::foo(QTreeWidgetItem *item, int column){
    if (updating_states) return;
    updating_states = true;

    if (item->childCount()){ // parent node
        if (item->checkState(column) != Qt::PartiallyChecked) {
            for (int c = 0; c < item->childCount(); c++){
                item->child(c)->setCheckState(column, item->checkState(column));
            }
        }
    }
    else { // child node
        if (item->checkState(column) == Qt::Unchecked){
            bool any_checked = false;

            QTreeWidgetItem *parent = item->parent();
            for (int c = 0; c < parent->childCount(); c++){
                QTreeWidgetItem *child = parent->child(c);
                if (child == item) continue;
                if (child->checkState(column) == Qt::Checked){
                    any_checked = true;
                    break;
                }
            }

            parent->setCheckState(column, any_checked ? Qt::PartiallyChecked : Qt::Unchecked);
        }
        else {
            bool all_checked = true;

            QTreeWidgetItem *parent = item->parent();
            for (int c = 0; c < parent->childCount(); c++){
                QTreeWidgetItem *child = parent->child(c);
                if (child == item) continue;
                if (child->checkState(column) == Qt::Unchecked){
                    all_checked = false;
                    break;
                }
            }

            parent->setCheckState(column, all_checked ? Qt::Checked : Qt::PartiallyChecked);
        }
    }

    updating_states = false;
}


SelectLipidomes::~SelectLipidomes() {
    delete ui;
}

void SelectLipidomes::cancel(){
    reject();
}

void SelectLipidomes::ok(){
    bool do_update = false;
    for (int i = 0; i < ui->lipidomesListWidget->count(); ++i){
        Canvas *canvas = ((LipidSpaceGUI*)parentWidget())->canvases[i];
        bool checked_state = ui->lipidomesListWidget->item(i)->checkState() == Qt::Checked;
        if (canvas->marked_for_selected_view != checked_state){
            canvas->marked_for_selected_view = checked_state;
            do_update = true;
        }
    }
    if (do_update && ((LipidSpaceGUI*)parentWidget())->selected_tiles_mode) ((LipidSpaceGUI*)parentWidget())->updateGUI();
    accept();
}
