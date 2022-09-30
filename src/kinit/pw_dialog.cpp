#include <QDialog>
#include <QLineEdit>

#include "pw_dialog.h"
#include "ui_pw_dialog.h"

PWDialog::PWDialog(QWidget* parent, const char* name,
         bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    ui = new Ui::PWDialog();
    ui->setupUi(this);

    setModal(modal);
    setAccessibleName(name);
}

PWDialog::~PWDialog()
{

}

void PWDialog::krb5promptSetText(const QString& text)
{
    ui->krb5prompt->setText(text);
}

void PWDialog::promptEditSetEchoMode(QLineEdit::EchoMode mode)
{
    ui->promptEdit->setEchoMode(mode);
}

QString PWDialog::promptEditText()
{
    return ui->promptEdit->text();
}

void PWDialog::errorLabelSetText(const QString& text)
{
    ui->errorLabel->setText(text);
}
