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
    connect(ui->lipidomesTreeWidget, &QTreeWidget::itemChanged, this, &SelectLipidomes::itemChanged);
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



    // add group lipidomes
    for (auto &kv : lipid_space->group_lipidomes){
        if (kv.second.size() <= 1) continue;

        QTreeWidgetItem *group_item = new QTreeWidgetItem();
        group_item->setText(0, ("'" + kv.first + "' group lipidomes").c_str());
        ui->lipidomesTreeWidget->addTopLevelItem(group_item);

        for (auto lipidome : kv.second) {
            if (uncontains_val(lipidome_to_canvas_position, lipidome) || lipidome_to_canvas_position[lipidome] >= (int)lipidSpaceGUI->canvases.size()) continue;

            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipidome]]->pointSet->title);
            item_to_canvas_position.insert({item, lipidome_to_canvas_position[lipidome]});
            group_item->addChild(item);

            item->setCheckState(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipidome]]->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);
        }
    }



    // add individual lipidomes
    for (auto lipidome : lipid_space->selected_lipidomes){
        if (uncontains_val(lipidome_to_canvas_position, lipidome)) continue;
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipidome]]->pointSet->title);

        item->setCheckState(0, lipidSpaceGUI->canvases[lipidome_to_canvas_position[lipidome]]->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);
        item_to_canvas_position.insert({item, lipidome_to_canvas_position[lipidome]});
        ui->lipidomesTreeWidget->addTopLevelItem(item);
    }
}


void SelectLipidomes::itemChanged(QTreeWidgetItem *item, int column){
    if (updating_states || (item == 0)) return;
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
            if (parent){
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
        }
        else {
            bool all_checked = true;

            QTreeWidgetItem *parent = item->parent();
            if (parent){
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
