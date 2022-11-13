#include "lipidspace/selectlipidomes.h"
#include "ui_selectlipidomes.h"

SelectLipidomes::SelectLipidomes(QWidget *parent) : QDialog(parent), ui(new Ui::SelectLipidomes) {
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());
    updating_states = false;
    item_to_canvas_position.clear();

    connect(ui->okButton, &QPushButton::clicked, this, &SelectLipidomes::ok);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SelectLipidomes::cancel);
}


void SelectLipidomes::init(){
    updating_states = false;
    ui->lipidomesTreeWidget->clear();
    item_to_canvas_position.clear();
    LipidSpaceGUI* lipidSpaceGUI = (LipidSpaceGUI*)parentWidget();
    LipidSpace* lipid_space = lipidSpaceGUI->lipid_space;

    map<Lipidome*, int> lipidome_to_canvas_position;
    for (uint i = 0; i < lipidSpaceGUI->canvases.size(); ++i) lipidome_to_canvas_position.insert({lipidSpaceGUI->canvases[i]->lipidome, i});

    // add global item
    QTreeWidgetItem *global_item = new QTreeWidgetItem();
    global_item->setText(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipid_space->global_lipidome]]->pointSet->title);
    global_item->setCheckState(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipid_space->global_lipidome]]->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);
    item_to_canvas_position.insert({global_item, lipidome_to_canvas_position[lipid_space->global_lipidome]});
    ui->lipidomesTreeWidget->addTopLevelItem(global_item);

    for (auto lipidome : lipid_space->selected_lipidomes){
        if (uncontains_val(lipidome_to_canvas_position, lipidome)) continue;
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipidome]]->pointSet->title);

        item->setCheckState(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipidome]]->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);
        item_to_canvas_position.insert({item, lipidome_to_canvas_position[lipidome]});
        ui->lipidomesTreeWidget->addTopLevelItem(item);
    }

    /*
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
    */
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
    LipidSpaceGUI* lipidSpaceGUI = (LipidSpaceGUI*)parentWidget();
    for (auto &kv : item_to_canvas_position){
        Canvas *canvas = lipidSpaceGUI->canvases[kv.second];
        bool checked_state = kv.first->checkState(0) == Qt::Checked;
        if (canvas->marked_for_selected_view != checked_state){
            canvas->marked_for_selected_view = checked_state;
            do_update = true;
        }
    }
    if (do_update && ((LipidSpaceGUI*)parentWidget())->selected_tiles_mode) ((LipidSpaceGUI*)parentWidget())->updateGUI();
    accept();
}
