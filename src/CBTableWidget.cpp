#include "lipidspace/CBTableWidget.h"

CBTableWidget::CBTableWidget(QWidget *parent) : QTableWidget(parent) {
    QAbstractButton *btn = findChild<QAbstractButton *>();
    if (btn != NULL) {
        QVBoxLayout* lay = new QVBoxLayout(btn);
        lay->setContentsMargins(0, 0, 0, 0);
        QLabel* label = new QLabel("Transpose");
        //label->setStyleSheet("QLabel {font-size: 10px; color: #FFFFFF; font-weight: bold; }""QToolTip { color: #ffffff; background-color: #000000; border: 1px #000000; }");
        label->setAlignment(Qt::AlignCenter);
        label->setContentsMargins(2, 2, 2, 2);
        lay->addWidget(label);
        btn->disconnect(SIGNAL(clicked()));
        connect(btn, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
    }
}


void CBTableWidget::cornerButtonClicked(){
    cornerButtonClick();
}
