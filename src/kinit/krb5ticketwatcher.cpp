/*
 * krb5-ticket-watcher
 *
 * Copyright (C) 2006  Michael Calmer (Michael.Calmer at gmx.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QCoreApplication>

#include <QtDebug>
#include <QHostAddress>
#include <QTextCodec>
#include <QMessageBox>
#include <QEvent>
#include <QTime>

#include "krb5ticketwatcher.h"
#include "v5.h"
#include "pw_dialog.h"
#include "pw_change_dialog.h"
#include "kinitdialog.h"

#include "ui_mainwidget.h"

#include <pwd.h>
#include <unistd.h>

Ktw::Ktw( int & argc, char ** argv, QWidget* parent, Qt::WindowFlags fl )
	: QWidget( parent, fl )
    , waitTimer()
	, translator()
	, kcontext(0)
	, kprincipal(0)
	, tgtEndtime(0)
	, forwardable(true)
	, proxiable(false)
	, lifetime(0)        // 0 default
	, lifetimeUnit("hours")
	, renewtime(0)       // 0 default, -1 no renewtime
	, renewtimeUnit("days")
	, promptInterval(31)  // default 31 minutes
{
	lifetimeUnit  = ki18n("hours");
	renewtimeUnit = ki18n("days");

    ui = new Ui::KrbTicketsMenuWidget();
    ui->setupUi(this);
	
	if(argc >= 3)
	{
		if(QString(argv[1]) == "-i" ||
		   QString(argv[1]) == "--interval")
		{
			bool ok = false;
			promptInterval = QString(argv[2]).toInt(&ok);

			if(!ok)
			{
				qWarning("invalid value for prompt interval. Setting default."); 
				promptInterval = 31;
			}
		}
	}

	qDebug("PromptInterval is %d min.", promptInterval);
	
	krb5_error_code err = krb5_init_context(&kcontext);
	if (err)
    {
		qFatal("Error at krb5_init_context");
		return;
	}

	setDefaultOptionsUsingCreds(kcontext);
	initMainWindow();
	qApp->processEvents();

    krb5_creds creds;

    if(!v5::getTgtFromCcache(kcontext, &creds))
    {
        kinit();
    }

	connect( &waitTimer, SIGNAL(timeout()), this, SLOT(initWorkflow()) );

	qDebug("start the timer");
	waitTimer.start( promptInterval*60*1000); // retryTime is in minutes

    setAttribute(Qt::WA_DeleteOnClose);
}

Ktw::~Ktw()
{
	if(kprincipal)
		krb5_free_principal(kcontext, kprincipal);
	kprincipal = NULL;
	
	krb5_free_context(kcontext);
	kcontext = NULL;

    delete ui;
}

// private ------------------------------------------------------------------

void
Ktw::initMainWindow()
{
    ui->textLabel1->setText(
						// Legend: Explain ticket flag "F"
                        QString("<qt><table><tr><td><b>F</b></td><td>")+ki18n("Forwardable")+QString("</td></tr>")+
						// Legend: Explain ticket flag "f"
						QString("<tr><td><b>f</b></td><td>")+ki18n("Forwarded")+QString("</td></tr>")+
						// Legend: Explain ticket flag "p"
						QString("<tr><td><b>P</b></td><td>")+ki18n("Proxiable")+QString("</td></tr>")+
						// Legend: Explain ticket flag "P"
						QString("<tr><td><b>p</b></td><td>")+ki18n("Proxy")+QString("</td></tr>")+
						// Legend: Explain ticket flag "D"
						QString("<tr><td><b>D</b></td><td>")+ki18n("May Postdate")+QString("</td></tr>")+
						// Legend: Explain ticket flag "d"
						QString("<tr><td><b>d</b></td><td>")+ki18n("Postdated")+QString("</td></tr>")+
						// Legend: Explain ticket flag "i"
						QString("<tr><td><b>i</b></td><td>")+ki18n("Invalid")+QString("</td></tr>")+
						// Legend: Explain ticket flag "R"
						QString("<tr><td><b>R</b></td><td>")+ki18n("Renewable")+QString("</td></tr>")+
						// Legend: Explain ticket flag "I"
						QString("<tr><td><b>I</b></td><td>")+ki18n("Initial")+QString("</td></tr>")+
						// Legend: Explain ticket flag "H"
						QString("<tr><td><b>H</b></td><td>")+ki18n("HW Auth")+QString("</td></tr>")+
						// Legend: Explain ticket flag "A"
						QString("<tr><td><b>A</b></td><td>")+ki18n("Pre Auth")+QString("</td></tr>")+
						// Legend: Explain ticket flag "T"
						QString("<tr><td><b>T</b></td><td>")+ki18n("Transit Policy Checked")+QString("</td></tr>")+
						// Legend: Explain ticket flag "O"
						QString("<tr><td><b>O</b></td><td>")+ki18n("Ok as Delegate")+QString("</td></tr>")+
						// Legend: Explain ticket flag "a"
						QString("<tr><td><b>a</b></td><td>")+ki18n("Anonymous")+QString("</td></tr></table></qt>")
					   );
	
    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(reReadCache()));
}

// public slot --------------------------------------------------------------

void
Ktw::reReadCache()
{
	QString ret = buildCcacheInfos();
	
	if(!ret.isEmpty())
	{
		qDebug() << "Error: " << ret;
        ui->commonLabel->setText("<qt><b>" + ret + "</b></qt>");
	}
}

// public ------------------------------------------------------------------

void
Ktw::forceRenewCredential()
{
	qDebug("forceRenewCredential called");
	initWorkflow(1);
}

void
Ktw::destroyCredential()
{
	if ( QMessageBox::Yes == QMessageBox::question(this,
												   // title
												   ki18n("Destroy Kerberos Ticket Cache?"),
												   // message text
												   ki18n("Do you want to destroy the ticket cache?"),
												   QMessageBox::Yes| QMessageBox::No, QMessageBox::No ))
	{
		int code = v5::destroyCcache(kcontext);
		if(code)
		{
			QMessageBox::critical(this,
			                      ki18n("Error !"),
			                      ki18n("Ticket cache cannot be destroyed."));
		}
	}
	else
	{
		qDebug("Destroy Kerberos Ticket Cache? => No");
	}
}

void
Ktw::initWorkflow(int type)
{
	bool have_tgt = FALSE;
	krb5_creds creds;

	have_tgt = v5::getTgtFromCcache(kcontext, &creds);
	if (have_tgt)
	{
		krb5_copy_principal(kcontext, creds.client, &kprincipal);
		krb5_free_cred_contents (kcontext, &creds);
	}
	
	int  retval = 0;
	switch(v5::credentialCheck(kcontext, kprincipal, promptInterval, &tgtEndtime))
	{
		case renew:
			retval = v5::renewCredential(kcontext, kprincipal, &tgtEndtime);
			if(!retval)
			{
				break;
			}
		case reinit:
			qDebug("stop the timer");
			waitTimer.stop();

			retval = reinitCredential();
			
			if(retval == KRB5_KDC_UNREACH)
			{
				// cannot reach the KDC sleeping. Try next time
				qWarning("cannot reach the KDC. Sleeping ...");
				retval = 0;
			}
			
			qDebug("start the timer");
            waitTimer.start( promptInterval*60*1000);

			break;
		default:
			if(type != 0)
			{
				retval = v5::renewCredential(kcontext, kprincipal, &tgtEndtime);
				if(!retval)
				{
					break;
				}
			}
	}
	
	qDebug("Workflow finished");
}

// public slots ------------------------------------------------------------- 

void
Ktw::kinit()
{
	krb5_get_init_creds_opt opts;

	krb5_error_code retval;
	bool ok = false;
	QString errorTxt;
	char *r = NULL;

	krb5_get_init_creds_opt_init(&opts);
	
	krb5_get_default_realm(kcontext, &r);
	QString defRealm(r);
	krb5_free_default_realm(kcontext, r);

	do
	{
		KinitDialog *dlg = new KinitDialog(this, "kinitDialog", true);

		dlg->errorLabelSetText(errorTxt);
		dlg->userLineEditSetText(getUserName());
		dlg->realmLineEditSetText(defRealm);
		dlg->passwordLineEditSetFocus();
		
		dlg->forwardCheckBoxSetChecked(forwardable);
		dlg->proxyCheckBoxSetChecked(proxiable);
		

		if(lifetime >= 0)
		{
			dlg->lifetimeSpinBoxSetValue(lifetime);
			dlg->lifetimeUnitComboBoxSetCurrentText(lifetimeUnit);
		}
		else
		{
			dlg->lifetimeSpinBoxSetValue(0);
		}

		if(renewtime >= 0)
		{
			dlg->renewtimeSpinBoxSetValue(renewtime);
			dlg->renewUnitComboBoxSetCurrentText(renewtimeUnit);
			dlg->renewCheckBoxSetChecked(true);
		}
		else
		{
			dlg->renewCheckBoxSetChecked(false);
		}

		
		int ret = dlg->exec();
		if(ret == QDialog::Rejected)
		{
			qDebug("rejected");
			return;
		}
		qDebug("accepted");

		errorTxt = "";

		QString principal = dlg->userLineEditText() + "@" + dlg->realmLineEditText();

		krb5_free_principal(kcontext, kprincipal);
		retval = krb5_parse_name(kcontext, principal.toUtf8(),
		                         &kprincipal);
		if (retval)
		{
			qDebug("Error during parse_name: %d", retval);
			ok = true;
			errorTxt = ki18n("Invalid principal name");
			continue;
		}
		
		forwardable = dlg->forwardCheckBoxIsChecked();
		proxiable = dlg->proxyCheckBoxIsChecked();

		if(dlg->lifetimeSpinBoxValue() >= 0)
		{
			lifetime = dlg->lifetimeSpinBoxValue();
			lifetimeUnit = dlg->lifetimeUnitComboBoxCurrentText();
		}
		else
		{
			lifetime = 0;
		}

		if(!dlg->renewCheckBoxIsChecked())
		{
			renewtime = -1;
		}
		else if(dlg->renewtimeSpinBoxValue() >= 0)
		{
			renewtime = dlg->renewtimeSpinBoxValue();
			renewtimeUnit = dlg->renewUnitComboBoxCurrentText();
		}
		else
		{
			renewtime = 0;
		}

		setOptions(kcontext, &opts);
		
		retval = v5::initCredential(kcontext, kprincipal,
		                            &opts, dlg->passwordLineEditText(),
		                            &tgtEndtime);
		if (retval)
		{
			qDebug("Error during initCredential(): %d", retval);
			ok = true;

			switch (retval)
			{
				case KRB5KDC_ERR_PREAUTH_FAILED:
				case KRB5KRB_AP_ERR_BAD_INTEGRITY:
					errorTxt = ki18n("Invalid Password");
					break;
				case KRB5KDC_ERR_KEY_EXP:
					retval = changePassword(dlg->passwordLineEditText());
					if(!retval)
						ok = false;
					
					break;
				case KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN:
				case KRB5KDC_ERR_S_PRINCIPAL_UNKNOWN:
					errorTxt = ki18n("Unknown principal");
					break;
				case KRB5_REALM_CANT_RESOLVE:
					errorTxt = ki18n("Unknown realm");
					break;
				default:
					errorTxt = v5::getKrb5ErrorMessage(kcontext, retval);
					break;
			}
		}
		else
		{
			ok = false;
		}
		delete dlg;
	}
	while(ok);
}

// public slots ------------------------------------------------------------- 

void
Ktw::restore()
{
	reReadCache();
	show();
}

// public slots ------------------------------------------------------------- 

int
Ktw::reinitCredential(const QString& password)
{
	krb5_error_code retval;
	krb5_creds my_creds;
	krb5_get_init_creds_opt opts;

	QString passwd = password;
	
	qDebug("reinit called");
	
	if (kprincipal == NULL)
	{
		qDebug("no principal found");
		retval = krb5_parse_name(kcontext, getUserName(),
		                         &kprincipal);
		if (retval)
		{
			return retval;
		}
	}

	krb5_get_init_creds_opt_init(&opts);

	setOptions(kcontext, &opts);
	
	if (v5::getTgtFromCcache (kcontext, &my_creds))
	{
		qDebug("got tgt from ccache");
		tgtEndtime = my_creds.times.endtime;
		krb5_free_cred_contents(kcontext, &my_creds);
	}
	else
	{
		qDebug("TGT expired");
		tgtEndtime = 0;
	}

	bool repeat = true;
    QString errorText;
	do
	{
        if(passwd.isEmpty())
		{
            passwd = passwordDialog(errorText);

			if(passwd.isNull())
				return -1;
		}
		
		retval = v5::initCredential(kcontext, kprincipal, &opts, passwd, &tgtEndtime);
		
		if(retval)
		{
			qDebug("Error during initCredential(): %d", retval);
			
			switch (retval)
			{
				case KRB5KDC_ERR_PREAUTH_FAILED:
				case KRB5KRB_AP_ERR_BAD_INTEGRITY:
                    errorText = QString("The password you entered is invalid");
					break;
				case KRB5KDC_ERR_KEY_EXP:
					retval = changePassword(passwd);
					if(!retval)
						repeat = false;
					
					break;
				case KRB5_KDC_UNREACH:
					QMessageBox::critical(this, ki18n("Failure"),
					                      v5::getKrb5ErrorMessage(kcontext, retval),
					                      QMessageBox::Ok, QMessageBox::Ok);
					repeat = false;
                    break;
				default:
					errorText = v5::getKrb5ErrorMessage(kcontext, retval);
					break;
			}
		}
		else
		{
			repeat = false;
		}
		
        passwd = QString();
	}
	while(repeat);

	return retval;
}

QString
Ktw::passwordDialog(const QString& errorText) const
{
	char* princ = NULL;
	krb5_error_code retval;
	
	if((retval = krb5_unparse_name(kcontext, kprincipal, &princ)))
	{
		qWarning("Error while unparsing principal name");
        return QString();
	}
	
    PWDialog pwd(this->parentWidget(), "pwdialog", true);
    pwd.krb5promptSetText(ki18n("Please enter the Kerberos password for <b>%1</b>").arg(princ));
    pwd.promptEditSetEchoMode(QLineEdit::Password);
	
    if(!errorText.isEmpty())
    {
        pwd.errorLabelSetText(errorText);
    }

	krb5_free_unparsed_name(kcontext, princ);
	
    int code = pwd.exec();
	if(code == QDialog::Rejected)
        return QString();
	
	return pwd.promptEditText();
}


int
Ktw::changePassword(const QString &oldpw)
{
	krb5_error_code retval;
	krb5_creds my_creds;
	krb5_get_init_creds_opt opts;
	QString oldPasswd( oldpw );
	
	qDebug("changePassword called");
	
	if (kprincipal == NULL)
	{
		retval = krb5_parse_name(kcontext, getUserName(),
		                         &kprincipal);
		if (retval)
		{
			return retval;
		}
	}

	krb5_get_init_creds_opt_init(&opts);
	krb5_get_init_creds_opt_set_tkt_life(&opts, 5*60);
	krb5_get_init_creds_opt_set_renew_life(&opts, 0);
	krb5_get_init_creds_opt_set_forwardable(&opts, 0);
	krb5_get_init_creds_opt_set_proxiable(&opts, 0);

    QString errorText;
	do
	{
		qDebug("call krb5_get_init_creds_password for kadmin/changepw");
		
		if(oldPasswd.isEmpty() || !errorText.isEmpty())
		{
			oldPasswd = passwordDialog(errorText);
			
			if(oldPasswd.isNull())
				return -1;
		}
		QString srv = "kadmin/changepw";
		retval = krb5_get_init_creds_password(kcontext,
		                                      &my_creds,
		                                      kprincipal,
		                                      oldPasswd.toUtf8().data(),
		                                      NULL,
		                                      NULL,
		                                      0,
		                                      srv.toUtf8().data(),
		                                      &opts);
		if(retval)
		{
			switch(retval)
			{
				case KRB5KDC_ERR_PREAUTH_FAILED:
				case KRB5KRB_AP_ERR_BAD_INTEGRITY:
					errorText = ki18n("The password you entered is invalid");
					break;
				case KRB5_KDC_UNREACH:
					/* kdc unreachable, return */
					krb5_free_cred_contents (kcontext, &my_creds);
					QMessageBox::critical(this, ki18n("Failure"),
					                      v5::getKrb5ErrorMessage(kcontext, retval),
					                      QMessageBox::Ok, QMessageBox::Ok);
					return retval;
				default:
					errorText = v5::getKrb5ErrorMessage(kcontext, retval);
					break;
			}
		}
	}
	while((retval != 0));
	
	bool    pwEqual = true;
	QString p1;
	QString p2;
	QString principal;
	char *princ = NULL;
	
	if((retval = krb5_unparse_name(kcontext, kprincipal, &princ)))
	{
		qWarning("Error while unparsing principal name");
		principal = "";
	}
	else
	{
		principal = QString(princ);
	}
	krb5_free_unparsed_name(kcontext, princ);

	do
	{
        PWChangeDialog pwd(NULL, "pwchangedialog", true);
        pwd.titleTextLabelSetText(ki18n("Change the password for principal <b>%1</b>").arg(principal));

        if(!pwEqual)
        {
            pwd.errorLabelSetText(ki18n("The passwords are not equal"));
        }
		
        int code = pwd.exec();
		
		if(code == QDialog::Accepted)
		{
			p1 = pwd.pwEdit1Text();
			p2 = pwd.pwEdit2Text();

			if(p1 != p2)
			{
				pwEqual = false;
			}
			else
			{
				pwEqual = true;
			}
		}
		else
		{
			krb5_free_cred_contents (kcontext, &my_creds);
			return retval;
		}
	}
	while(!pwEqual);

	
	int result_code;
	krb5_data result_code_string, result_string;
	
	if ((retval = krb5_change_password(kcontext, &my_creds, p1.toUtf8().data(),
	                                   &result_code, &result_code_string,
	                                   &result_string)))
	{
		qDebug("changing password failed. %d", retval);
		krb5_free_cred_contents (kcontext, &my_creds);
		QMessageBox::critical(this, ki18n("Password change failed"),
		                      v5::getKrb5ErrorMessage(kcontext, retval),
		                      QMessageBox::Ok, QMessageBox::Ok);
		return retval;
	}

	krb5_free_cred_contents (kcontext, &my_creds);
	
	if (result_code)
	{
		qDebug("%.*s%s%.*s\n",
		         (int) result_code_string.length, result_code_string.data,
		         result_string.length?": ":"",
		         (int) result_string.length,
		         result_string.data ? result_string.data : "");
		return 2;
	}
	if (result_string.data != NULL)
		free(result_string.data);
	if (result_code_string.data != NULL)
		free(result_code_string.data);
	
	return reinitCredential(p1);
}

void
Ktw::setDefaultOptionsUsingCreds(krb5_context k_context)
{
	krb5_creds creds;
	
    if (v5::getTgtFromCcache (k_context, &creds))
	{
		forwardable = v5::getCredForwardable(&creds) != 0;
		
		proxiable = v5::getCredProxiable(&creds) != 0;
		
 		krb5_deltat tkt_lifetime = creds.times.endtime - creds.times.starttime;
		
		if( (lifetime = (krb5_deltat) tkt_lifetime / (60*60*24)) > 0 )
		{
			lifetimeUnit = ki18n("days");
		}
		else if( (lifetime = (krb5_deltat) tkt_lifetime / (60*60)) > 0 )
		{
			lifetimeUnit = ki18n("hours");
		}
		else if( (lifetime = (krb5_deltat) tkt_lifetime / (60)) > 0 )
		{
			lifetimeUnit = ki18n("minutes");
		}
		else
		{
			lifetime = tkt_lifetime;
			lifetimeUnit = ki18n("seconds");
		}
		
		if(creds.times.renew_till == 0)
		{
			renewtime = -1;
		}
		else
		{
			krb5_deltat tkt_renewtime = creds.times.renew_till - creds.times.starttime;

			if( (renewtime = (krb5_deltat) (tkt_renewtime / (60*60*24))) > 0 )
			{
				renewtimeUnit = ki18n("days");
			}
			else if( (renewtime = (krb5_deltat) (tkt_renewtime / (60*60))) > 0 )
			{
				renewtimeUnit = ki18n("hours");
			}
			else if( (renewtime = (krb5_deltat) (tkt_renewtime / (60))) > 0 )
			{
				renewtimeUnit = ki18n("minutes");
			}
			else
			{
				renewtime = tkt_renewtime;
				renewtimeUnit = ki18n("seconds");
			}
		}
        krb5_free_cred_contents (k_context, &creds);
	}
}

void
Ktw::setOptions(krb5_context, krb5_get_init_creds_opt *opts)
{
	qDebug("================= Options =================");
	qDebug("Forwardable %s",(forwardable)?"true":"false");
	qDebug("Proxiable %s",(proxiable)?"true":"false");
	
	krb5_get_init_creds_opt_set_forwardable(opts, (forwardable)?1:0);

	krb5_get_init_creds_opt_set_proxiable(opts, (proxiable)?1:0);


	if(lifetime > 0)
	{
		krb5_deltat tkt_lifetime = 0;
		
		if(lifetimeUnit == ki18n("seconds")) tkt_lifetime = lifetime;
		if(lifetimeUnit == ki18n("minutes")) tkt_lifetime = lifetime*60;
		if(lifetimeUnit == ki18n("hours")) tkt_lifetime = lifetime*60*60;
		if(lifetimeUnit == ki18n("days")) tkt_lifetime = lifetime*60*60*24;

		qDebug("Set lifetime to: %d", tkt_lifetime);
		
		krb5_get_init_creds_opt_set_tkt_life(opts, tkt_lifetime);
	}
	

	if(renewtime > 0)
	{
		krb5_deltat tkt_renewtime = 0;

		if(renewtimeUnit == ki18n("seconds")) tkt_renewtime = renewtime;
		if(renewtimeUnit == ki18n("minutes")) tkt_renewtime = renewtime*60;
		if(renewtimeUnit == ki18n("hours")) tkt_renewtime = renewtime*60*60;
		if(renewtimeUnit == ki18n("days")) tkt_renewtime = renewtime*60*60*24;

		qDebug("Set renewtime to: %d", tkt_renewtime);

		krb5_get_init_creds_opt_set_renew_life(opts, tkt_renewtime);
	}
	else if(renewtime < 0)
	{
		qDebug("Set renewtime to: %d", 0);

		krb5_get_init_creds_opt_set_renew_life(opts, 0);
	}
	else
	{
		qDebug("Use default renewtime");
	}
	
	qDebug("================= END Options =================");
}

// static -----------------------------------------------------------


const char *
Ktw::getUserName()
{
	return getpwuid(getuid())->pw_name;
}


QString
Ktw::buildCcacheInfos()
{
	krb5_ccache cache = NULL;
	krb5_cc_cursor cur;
	krb5_creds creds;
	krb5_flags flags;
    krb5_error_code code;
    char *defname = NULL;
    QString errmsg;
    bool done = false;

    ui->ticketView->clear();
    
    if ((code = krb5_cc_default(kcontext, &cache)))
    {
    	errmsg += "Error while getting default ccache";
    	goto done;
    }
    
    flags = 0;
    if ((code = krb5_cc_set_flags(kcontext, cache, flags)))
    {
    	if (code == KRB5_FCC_NOFILE)
    	{
    		errmsg += ki18n("No credentials cache found") + " (" + ki18n("Ticket cache: %1:%2")
    			.arg(krb5_cc_get_type(kcontext, cache))
    			.arg(krb5_cc_get_name(kcontext, cache)) + ")";
    	}
    	else
    	{
    		errmsg += ki18n("Error while setting cache flags") + " (" + ki18n("Ticket cache: %1:%2")
    			.arg(krb5_cc_get_type(kcontext, cache))
    			.arg(krb5_cc_get_name(kcontext, cache)) + ")";
    	}
    	goto done;
    }
    if(kprincipal != NULL)
    	krb5_free_principal(kcontext, kprincipal);
    
    if ((code = krb5_cc_get_principal(kcontext, cache, &kprincipal)))
    {
    	errmsg += "Error while retrieving principal name";
    	goto done;
    }
    if ((code = krb5_unparse_name(kcontext, kprincipal, &defname)))
    {
    	errmsg += "Error while unparsing principal name";
    	goto done;
    }

    ui->commonLabel->setText(QString("<qt><b>")+
                         ki18n("Ticket cache: %1:%2")
                         .arg(krb5_cc_get_type(kcontext, cache))
                         .arg(krb5_cc_get_name(kcontext, cache)) +
                         QString("</b><br><b>")+
                         ki18n("Default principal: %3").arg(defname)+
                         QString("</b><br><br>")
                         );

    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur)))
    {
    	errmsg += "Error while starting to retrieve tickets";
    	goto done;
    }
    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds)))
    {
    	errmsg += showCredential(&creds, defname);
    	krb5_free_cred_contents(kcontext, &creds);
    	if(!errmsg.isEmpty())
    	{
    		goto done;
    	}
    }
    if (code == KRB5_CC_END)
    {
    	if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur)))
    	{
    		errmsg += "Error while finishing ticket retrieval";
    		goto done;
    	}
    	flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
    	if ((code = krb5_cc_set_flags(kcontext, cache, flags)))
    	{
    		errmsg += "Error while closing ccache";
    		goto done;
    	}
    }
    else
    {
    	errmsg += "Error while retrieving a ticket";
    	goto done;
    }

    ui->ticketView->topLevelItem(0)->setExpanded(true);
    ui->ticketView->resizeColumnToContents(0);
    ui->ticketView->resizeColumnToContents(1);
    
    done = true;
    
done:
    if(defname != NULL)
    	krb5_free_unparsed_name(kcontext, defname);
    if(cache != NULL)
    	krb5_cc_close(kcontext, cache);
    
    if(!done)
    {
    	qDebug(errmsg.toUtf8());
    	return errmsg;
    }
    else
    {
    	return errmsg;
    }
}

QString
Ktw::showCredential(krb5_creds *cred, char *defname)
{
	krb5_error_code retval;
	krb5_ticket *tkt;
	char *name, *sname;
    QTreeWidget *lv = ui->ticketView;
	QTreeWidgetItem *last = NULL;
	
	retval = krb5_unparse_name(kcontext, cred->client, &name);
	if (retval)
	{
		return "Error while unparsing client name";
	}
	retval = krb5_unparse_name(kcontext, cred->server, &sname);
	if (retval)
	{
		krb5_free_unparsed_name(kcontext, name);
		return "Error while unparsing server name";
	}
	if (!cred->times.starttime)
		cred->times.starttime = cred->times.authtime;

	QString n = QString(sname) +
		((strcmp(name, defname))? ki18n(" for client %1").arg(name):QString());

	QTreeWidgetItem *after = 0;
	int count = lv->topLevelItemCount();
	if(count > 0)
	{
		after = lv->topLevelItem( count - 1 );
	}
	
	krb5_timestamp expires = cred->times.endtime - v5::getNow(kcontext);
	if(expires <= 0)
		expires = 0;
	
	QTreeWidgetItem *lvi = new QTreeWidgetItem(lv, after);
	lvi->setText(0, ki18n("Service principal"));
	lvi->setText(1, n);
	lvi->setText(2, printInterval(expires));

	QBrush brush(Qt::green);
	
	if( expires == 0 )
		brush = QBrush( Qt::red );
	else if(expires > 0 && expires < 60)
		brush = QBrush( Qt::yellow );
	
	lvi->setBackground(0, brush);
	lvi->setBackground(1, brush);
	lvi->setBackground(2, brush);
	
	last = new QTreeWidgetItem(lvi);
	last->setText(0, ki18n("Valid starting"));
	last->setText(1, printtime(cred->times.starttime));
	
	last = new QTreeWidgetItem(lvi, last);
	last->setText(0, ki18n("Expires"));
	last->setText(1, printtime(cred->times.endtime));

	if(cred->times.renew_till)
	{
		last = new QTreeWidgetItem(lvi, last);
		last->setText(0, ki18n("Renew until"));
		last->setText(1, printtime(cred->times.renew_till));
	}
	
	QString tFlags;
	if (v5::getCredForwardable(cred) != 0)
		tFlags += 'F';
	if (v5::getCredForwarded(cred) != 0)
		tFlags += 'f';
	if (v5::getCredProxiable(cred) != 0)
		tFlags += 'P';
	if (v5::getCredProxy(cred) != 0)
		tFlags += 'p';
	if (v5::getCredMayPostdate(cred) != 0)
		tFlags += 'D';
	if (v5::getCredPostdated(cred) != 0)
		tFlags += 'd';
	if (v5::getCredInvalid(cred) != 0)
		tFlags += 'i';
	if (v5::getCredRenewable(cred) != 0)
		tFlags += 'R';
	if (v5::getCredInitial(cred) != 0)
		tFlags += 'I';
	if (v5::getCredHwAuth(cred) != 0)
		tFlags += 'H';
	if (v5::getCredPreAuth(cred) != 0)
		tFlags += 'A';
	if (v5::getCredTransitPolicyChecked(cred) != 0)
		tFlags += 'T';
	if (v5::getCredOkAsDelegate(cred) != 0)
        tFlags += 'O';         /* D/d are taken.  Use short strings?  */
	if (v5::getCredAnonymous(cred) != 0)
		tFlags += 'a';
	
	last = new QTreeWidgetItem(lvi, last);
	last->setText(0, ki18n("Ticket flags"));
	last->setText(1, tFlags);

	retval = krb5_decode_ticket(&cred->ticket, &tkt);
	if(!retval)
	{
		last = new QTreeWidgetItem(lvi, last);
		last->setText(0, ki18n("Key Encryption Type"));
		last->setText(1, v5::etype2String(cred->keyblock.enctype));
		last = new QTreeWidgetItem(lvi, last);
		last->setText(0, ki18n("Ticket Encryption Type" ));
		last->setText(1, v5::etype2String(tkt->enc_part.enctype));
	}
	if (tkt != NULL)
		krb5_free_ticket(kcontext, tkt);

	QString addresses;
	if (!cred->addresses|| !cred->addresses[0])
	{
		addresses += ki18n("(none)");
	}
	else
	{
		int i;
		
		addresses += oneAddr(cred->addresses[0]);

		for (i=1; cred->addresses[i]; i++)
		{
			addresses += QString(", ");
			addresses += oneAddr(cred->addresses[i]);
		}
	}
	last = new QTreeWidgetItem(lvi, last);
	last->setText(0, ki18n("Addresses" ));
	last->setText(1, addresses);
	
	krb5_free_unparsed_name(kcontext, name);
	krb5_free_unparsed_name(kcontext, sname);
	last = NULL;
	lvi = NULL;
	
	return "";
}

QString
Ktw::oneAddr(krb5_address *a)
{
	QHostAddress addr;
	
	switch (a->addrtype)
	{
		case ADDRTYPE_INET:
    		if (a->length != 4)
    		{
    		broken:
    			return ki18n("Broken address (type %1 length %2)")
    				.arg(a->addrtype)
    				.arg(a->length);
    		}
            quint32 quint_ad;
            memcpy (&quint_ad, a->contents, 4);
            addr = QHostAddress(quint_ad);

    		break;
    	case ADDRTYPE_INET6:
    		if (a->length != 16)
    			goto broken;
    		{
    			Q_IPV6ADDR ad;
    			memcpy(&ad, a->contents, 16);
    			addr = QHostAddress(ad);
    		}
    		break;
    	default:
    		return ki18n("Unknown address type %1").arg(a->addrtype);
    }
	if(addr.isNull())
	{
		return ki18n("(none)");
	}
	return addr.toString();
}


QString
Ktw::printtime(time_t tv)
{
	char timestring[30];
	char fill = ' ';

	if(tv == 0)
	{
		return "";
	}
	
	if (!krb5_timestamp_to_sfstring((krb5_timestamp) tv,
	                                timestring, 29, &fill))
	{
		return timestring;
	}
	return "";
}


QString
Ktw::printInterval(krb5_timestamp time)
{
	if(time < 1)	
		time = 1;

    int sec = time % 60;

    time = int(time / 60);

    int min = time % 60;

    time = int(time / 60);

    int hour = time % 24;

    time = int(time /24);

    QString str;
    if(time > 0)
        str.asprintf("%d %s %02d:%02d:%02d", time, ki18n("Day(s)").toUtf8().data(), hour, min, sec);
    else
        str.asprintf("%02d:%02d:%02d", hour, min, sec);
    
    return str;
}
