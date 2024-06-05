#ifndef CREATE_PSO_DIALOG_H
#define CREATE_PSO_DIALOG_H

#include "create_object_dialog.h"

namespace Ui {
class CreatePSODialog;
}

class CreatePSODialog final : public CreateObjectDialog {

    Q_OBJECT

public:
    explicit CreatePSODialog(QWidget *parent = nullptr);
    ~CreatePSODialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    Ui::CreatePSODialog *ui;
};

#endif // CREATE_PSO_DIALOG_H
