    /****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "kinitdialog.h"
#include "ui_kinitdialog.h"

KinitDialog::KinitDialog( QWidget* parent, const char* name,
                         bool modal, Qt::WindowFlags fl)
	: QDialog(parent, fl)
{
	setModal(modal);
	setAccessibleName(name);

    ui = new Ui::KinitDialog();
    ui->setupUi(this);
    changeDetailsState(Qt::Unchecked);
    changeRenewState(Qt::Checked);
}

KinitDialog::~KinitDialog()
{
    delete ui;
}

void KinitDialog::changeDetailsState( int state )
{
	qDebug("changeDetailsState called");
	if(state == Qt::Checked)
	{
        ui->detailsBox->show();
	}
	else if(state == Qt::Unchecked)
	{
        ui->detailsBox->hide();
	}
	adjustSize();
}


void KinitDialog::errorLabelSetText(const QString& text)
{
    ui->errorLabel->setText(text);
}

void KinitDialog::userLineEditSetText(const QString& text)
{
    ui->userLineEdit->setText(text);
}

QString KinitDialog::userLineEditText()
{
    return ui->userLineEdit->text();
}

void KinitDialog::realmLineEditSetText(const QString& text)
{
    ui->realmLineEdit->setText(text);
}

QString KinitDialog::realmLineEditText()
{
    return ui->realmLineEdit->text();
}

void KinitDialog::passwordLineEditSetFocus()
{
    ui->passwordLineEdit->setFocus();
}

QString KinitDialog::passwordLineEditText()
{
    return ui->passwordLineEdit->text();
}

void KinitDialog::forwardCheckBoxSetChecked(bool check)
{
    ui->forwardCheckBox->setChecked(check);
}

bool KinitDialog::forwardCheckBoxIsChecked()
{
    return ui->forwardCheckBox->isChecked();
}

void KinitDialog::proxyCheckBoxSetChecked(bool check)
{
    ui->proxyCheckBox->setChecked(check);
}

bool KinitDialog::proxyCheckBoxIsChecked()
{
    return ui->proxyCheckBox->isChecked();
}

void KinitDialog::lifetimeSpinBoxSetValue(int v)
{
    ui->lifetimeSpinBox->setValue(v);
}

int KinitDialog::lifetimeSpinBoxValue()
{
    return ui->lifetimeSpinBox->value();
}

void KinitDialog::lifetimeUnitComboBoxSetCurrentText(const QString& text)
{
    int index = ui->lifetimeUnitComboBox->findText(text);
    ui->lifetimeUnitComboBox->setCurrentIndex(index);
}

QString KinitDialog::lifetimeUnitComboBoxCurrentText()
{
    return ui->lifetimeUnitComboBox->currentText();
}

void KinitDialog::renewtimeSpinBoxSetValue(int v)
{
    ui->renewtimeSpinBox->setValue(v);
}

int KinitDialog::renewtimeSpinBoxValue()
{
    return ui->renewtimeSpinBox->value();
}

void KinitDialog::renewUnitComboBoxSetCurrentText(const QString& text)
{
    int index = ui->renewUnitComboBox->findText(text);
    ui->renewUnitComboBox->setCurrentIndex(index);
}

QString KinitDialog::renewUnitComboBoxCurrentText()
{
    return ui->renewUnitComboBox->currentText();
}

void KinitDialog::renewCheckBoxSetChecked(bool check)
{
    ui->renewCheckBox->setChecked(check);
}

bool KinitDialog::renewCheckBoxIsChecked()
{
    return ui->renewCheckBox->isChecked();
}

void KinitDialog::changeRenewState(int state)
{
	qDebug("changeRenewState called");
	if(state == Qt::Checked)
	{
        ui->renewtimeSpinBox->setEnabled(true);
        ui->renewUnitComboBox->setEnabled(true);
	}
	else if(state == Qt::Unchecked)
	{
        ui->renewtimeSpinBox->setEnabled(false);
        ui->renewUnitComboBox->setEnabled(false);
	}
}
