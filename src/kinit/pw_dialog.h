#ifndef PW_DIALOG_H
#define PW_DIALOG_H
#include <QString>
#include <QObject>
#include <QLineEdit>
#include <QDialog>

#include "krb5_tw_gettext.h"

namespace Ui {
    class PWDialog;
}

class PWDialog : public QDialog
{
    Q_OBJECT

public:
    PWDialog(QWidget* parent = 0, const char* name = 0,
             bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags() );
	
    ~PWDialog();

    void krb5promptSetText(const QString& text);

    void promptEditSetEchoMode(QLineEdit::EchoMode mode);

    QString promptEditText();
	
    void errorLabelSetText(const QString& text);
private:
    Ui::PWDialog *ui;
};

#endif //PW_DIALOG_H
