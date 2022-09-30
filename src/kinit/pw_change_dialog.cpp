#include "pw_change_dialog.h"

#include "ui_pw_change_dialog.h"

PWChangeDialog::PWChangeDialog( QWidget* parent, const char* name,
               bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    ui = new Ui::PWChangeDialog();
    ui->setupUi(this);

    setModal(modal);
    setAccessibleName(name);

}

PWChangeDialog::~PWChangeDialog() {}

void PWChangeDialog::titleTextLabelSetText(const QString& text)
{
    ui->titleTextLabel->setText(text);
}

void PWChangeDialog::errorLabelSetText(const QString& text)
{
    ui->errorLabel->setText(text);
}

QString PWChangeDialog::pwEdit1Text()
{
    return ui->pwEdit1->text();
}

QString PWChangeDialog::pwEdit2Text()
{
    return ui->pwEdit2->text();
}
