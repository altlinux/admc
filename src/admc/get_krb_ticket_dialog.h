#ifndef GET_KRB_TICKET_DIALOG_H
#define GET_KRB_TICKET_DIALOG_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class GetKrbTicketDialog;
}

class GetKrbTicketDialog: public QDialog {
    Q_OBJECT
public:
    GetKrbTicketDialog(QWidget *parent = nullptr);

    ~GetKrbTicketDialog();
    void get_krb5_token();

    void validate_login_input();
    void validate_password_input();
    void validate_domain_name_input();
    bool validate_data_string(QString data_string);

private:
    const int errorBufSize = 1000;
    Ui::GetKrbTicketDialog *ui;
};

#endif // GET_KRB_TICKET_DIALOG_H
