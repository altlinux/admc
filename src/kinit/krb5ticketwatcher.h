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

#ifndef  KRB5_TICKET_WATCHER_H
#define  KRB5_TICKET_WATCHER_H

#include <QString>
#include <QTimer>
#include <QWidget>
#include <QTranslator>

#define KRB5_PRIVATE 1
#include <krb5.h>

#include "krb5_tw_gettext.h"

class QAction;
class QMenu;
class QEvent;

namespace Ui {
class KrbTicketsMenuWidget;
}

class Ktw : public QWidget
{
	Q_OBJECT
	
public:
    Ktw(int & argc, char ** argv,
	    QWidget* parent = 0, Qt::WindowFlags fl = 0  );
	~Ktw();
	
	enum reqAction {none, renew, reinit};

public slots:
    void forceRenewCredential();
    void destroyCredential();
	void initWorkflow(int type = 0);
	void restore();
	void kinit();
	void reReadCache();

private slots:
	int
    changePassword(const QString& oldpw = QString());

private:
	void initMainWindow();
	QString buildCcacheInfos();
	QString showCredential(krb5_creds *cred, char *defname);

	QString printtime(time_t tv);
	QString oneAddr(krb5_address *a);
	QString printInterval(krb5_timestamp time);

	QString
    passwordDialog(const QString& errorText = QString()) const;
	
	static const char *
	getUserName();

	int
    reinitCredential(const QString &password = QString());

	void
	setDefaultOptionsUsingCreds(krb5_context);

	void
	setOptions(krb5_context, krb5_get_init_creds_opt *opts);
		
	QTimer        waitTimer;
	QTranslator   translator;

	krb5_context   kcontext;
	krb5_principal kprincipal;
	krb5_timestamp tgtEndtime;

	bool           forwardable;
	bool           proxiable;
	
	krb5_deltat    lifetime;      // 0 default
	QString        lifetimeUnit;
	krb5_deltat    renewtime;     // 0 default, -1 no renewtime
	QString        renewtimeUnit;
	
	int            promptInterval;

    Ui::KrbTicketsMenuWidget *ui;
};

#endif //KRB5_TICKET_WATCHER_H
