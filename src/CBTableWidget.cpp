#include "lipidspace/CBTableWidget.h"

CBTableWidget::CBTableWidget(QWidget *parent) : QTableWidget(parent) {
    QAbstractButton *btn = findChild<QAbstractButton *>();
    transpose_label = 0;
    
    if (btn != NULL) {
        QVBoxLayout* lay = new QVBoxLayout(btn);
        lay->setContentsMargins(0, 0, 0, 0);
        transpose_label = new QLabel("Transpose");
        transpose_label->setAlignment(Qt::AlignCenter);
        transpose_label->setContentsMargins(2, 2, 2, 2);
        lay->addWidget(transpose_label);
        btn->disconnect(SIGNAL(clicked()));
        connect(btn, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
    }
    ctrl_pressed = 0;
}


void CBTableWidget::cornerButtonClicked(){
    cornerButtonClick();
}


void CBTableWidget::set_ctrl(bool *_ctrl_pressed){
    ctrl_pressed = _ctrl_pressed;
}


void CBTableWidget::wheelEvent(QWheelEvent* event){
    if (ctrl_pressed){
        if (*ctrl_pressed){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool zoom_in = event->angleDelta().y() > 0;
#else
        bool zoom_in = event->delta() > 0;
#endif
        
        int zoom_value = max(1, (int)GlobalData::gui_num_var["table_zoom"] + (2 * zoom_in - 1));
        GlobalData::gui_num_var["table_zoom"] = zoom_value;
        QFont item_font("Helvetica", zoom_value);
        transpose_label->setFont(item_font);
        emit zooming();
        }
        else {
            QTableWidget::wheelEvent(event);
        }
    }
    else {
        QTableWidget::wheelEvent(event);
    }
}
