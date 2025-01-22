#ifndef SDDL_VIEW_DIALOG_H
#define SDDL_VIEW_DIALOG_H

#include <QDialog>

namespace Ui {
class SDDLViewDialog;
}

struct security_descriptor;

class SDDLViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit SDDLViewDialog(QWidget *parent = nullptr);
    ~SDDLViewDialog();

public slots:
    void update(security_descriptor *sd_arg);
    void set_trustee(const QByteArray &trustee_arg);

private:
    Ui::SDDLViewDialog *ui;
    security_descriptor *sd;
    QByteArray trustee;

    QString get_sddl() const;
    void update();
};

#endif // SDDL_VIEW_DIALOG_H
