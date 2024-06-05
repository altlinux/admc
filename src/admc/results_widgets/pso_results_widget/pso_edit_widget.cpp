#include "pso_edit_widget.h"
#include "ui_pso_edit_widget.h"
#include "ad_interface.h"

PSOEditWidget::PSOEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PsoEditWidget) {

    ui->setupUi(this);
}

PSOEditWidget::~PSOEditWidget() {
    delete ui;
}
