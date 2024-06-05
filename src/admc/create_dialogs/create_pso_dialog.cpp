#include "create_pso_dialog.h"
#include "ui_create_pso_dialog.h"

#include "ad_interface.h"

CreatePSODialog::CreatePSODialog(QWidget *parent) :
    CreateObjectDialog(parent),
    ui(new Ui::CreatePSODialog) {

    ui->setupUi(this);


}

CreatePSODialog::~CreatePSODialog() {
    delete ui;
}

void CreatePSODialog::accept() {

}

QString CreatePSODialog::get_created_dn() const {
    return QString();
}

