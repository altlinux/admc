#include "get_krb_ticket_dialog.h"
#include "ui_get_krb_ticket_dialog.h"
#include "kinit/kinit.c"
#include "settings.h"
#include "status.h"
#include "ad_interface.h"

#include <QDebug>
#include <QSettings>

GetKrbTicketDialog::GetKrbTicketDialog(QWidget *parent)
    : QDialog(parent) {
        ui = new Ui::GetKrbTicketDialog();
        ui->setupUi(this);

        connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GetKrbTicketDialog::get_krb5_token);

        connect(ui->login_edit, &QLineEdit::editingFinished, this, &GetKrbTicketDialog::validate_login_input);
        connect(ui->password_edit, &QLineEdit::editingFinished, this, &GetKrbTicketDialog::validate_password_input);
        connect(ui->domain_name_edit, &QLineEdit::editingFinished, this, &GetKrbTicketDialog::validate_domain_name_input);
}

void GetKrbTicketDialog::validate_login_input()
{
    if (!validate_data_string(ui->login_edit->text()))
    {
        ui->login_edit->setText(QString());
    }
}

void GetKrbTicketDialog::validate_password_input()
{
    if (!validate_data_string(ui->password_edit->text()))
    {
        ui->login_edit->setText(QString());
    }
}

bool GetKrbTicketDialog::validate_data_string(QString data_string)
{
    return !(data_string.isNull() || data_string.isEmpty());
}

void GetKrbTicketDialog::validate_domain_name_input()
{
    if (!validate_data_string(ui->domain_name_edit->text()))
    {
        ui->domain_name_edit->setText(QString());
    }
}

void GetKrbTicketDialog::get_krb5_token()
{
    if (validate_data_string(ui->login_edit->text()) &&
        validate_data_string(ui->password_edit->text()) &&
        validate_data_string(ui->domain_name_edit->text()))
    {
        QString principal = validate_data_string(ui->password_edit->text()) ?
                    QStringList{ui->login_edit->text(), ui->domain_name_edit->text().toUpper()}.join('@').toLocal8Bit().data() :
                    ui->login_edit->text();

        char data_from_kinit[errorBufSize]{0};
        kinit(principal.toLocal8Bit().data(), ui->password_edit->text().toLocal8Bit().data(), data_from_kinit, errorBufSize);

        QString kinit_result = QString::fromUtf8(data_from_kinit);

        if(!kinit_result.isEmpty())
        {
            error_log(QList<QString>{kinit_result}, this->parentWidget());
        }
    }
}

GetKrbTicketDialog::~GetKrbTicketDialog()
{
    delete ui;
}
