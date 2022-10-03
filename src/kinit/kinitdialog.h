#ifndef KINITDIALOG_H
#define KINITDIALOG_H

#include <QString>
#include <QStringList>
#include <QDialog>

#include "krb5_tw_gettext.h"

namespace Ui {
class KinitDialog;
}

class KinitDialog : public QDialog
{
	Q_OBJECT

	public:
	KinitDialog(QWidget* parent = 0, const char* name = 0,
                bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags() );
	~KinitDialog();

    void errorLabelSetText(const QString& text);
	
    void userLineEditSetText(const QString& text);

    QString userLineEditText();
	
    void realmLineEditSetText(const QString& text);

    QString realmLineEditText();
	
    void passwordLineEditSetFocus();
	
    QString passwordLineEditText();
	
    void forwardCheckBoxSetChecked(bool check);

    bool forwardCheckBoxIsChecked();

    void proxyCheckBoxSetChecked(bool check);

    bool proxyCheckBoxIsChecked();

    void lifetimeSpinBoxSetValue(int v);

    int lifetimeSpinBoxValue();

    void lifetimeUnitComboBoxSetCurrentText(const QString& text);

    QString lifetimeUnitComboBoxCurrentText();

    void renewtimeSpinBoxSetValue(int v);
	
    int renewtimeSpinBoxValue();
	
    void renewUnitComboBoxSetCurrentText(const QString& text);

    QString renewUnitComboBoxCurrentText();

    void renewCheckBoxSetChecked(bool check);

    bool renewCheckBoxIsChecked();

    private slots:
    void changeDetailsState( int state );
	void changeRenewState(int state);

    private:
    Ui::KinitDialog *ui;

};

#endif
