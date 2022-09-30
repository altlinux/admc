#ifndef PWCHANGEDIALOG_H
#define PWCHANGEDIALOG_H

#include <QDialog>
#include <QString>

#include "krb5_tw_gettext.h"

namespace Ui{
    class PWChangeDialog;
}

class PWChangeDialog : public QDialog
{
    Q_OBJECT

public:
    PWChangeDialog( QWidget* parent = 0, const char* name = 0,
                   bool modal = false, Qt::WindowFlags fl = 0 );
	
    ~PWChangeDialog();

    void titleTextLabelSetText(const QString& text);

    void errorLabelSetText(const QString& text);

    QString pwEdit1Text();

    QString pwEdit2Text();
private:
    Ui::PWChangeDialog *ui;

};

#endif // PWCHANGEDIALOG_H
