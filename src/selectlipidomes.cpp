#include "lipidspace/selectlipidomes.h"
#include "ui_selectlipidomes.h"

SelectLipidomes::SelectLipidomes(QWidget *parent) : QDialog(parent), ui(new Ui::SelectLipidomes) {
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(this->width(), this->height());

    connect(ui->okButton, &QPushButton::clicked, this, &SelectLipidomes::ok);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SelectLipidomes::cancel);

    for (auto canvas : ((LipidSpaceGUI*)parentWidget())->canvases){
        QListWidgetItem *item = new QListWidgetItem(canvas->pointSet->title);
        item->setCheckState(canvas->marked_for_selected_view ? Qt::Checked : Qt::Unchecked);
        ui->lipidomesListWidget->addItem(item);
    }
}

SelectLipidomes::~SelectLipidomes() {
    delete ui;
}

void SelectLipidomes::cancel(){
    close();
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
    close();
}
