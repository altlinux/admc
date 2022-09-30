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

#ifndef  KRB5_TICKET_WATCHER_V5_H
#define  KRB5_TICKET_WATCHER_V5_H

#define KRB5_PRIVATE 1
#include <krb5.h>

class QString;
class QStringList;

class v5
{
	public:

	static bool
	getTgtFromCcache (krb5_context context, krb5_creds *creds);

	static int
	credentialCheck(krb5_context kcontext, krb5_principal kprincipal,
	                int promptInterval, krb5_timestamp *tgtEndtime);

	static int
	renewCredential(krb5_context kcontext, krb5_principal kprincipal, krb5_timestamp *tgtEndtime);

	static int
	initCredential(krb5_context kcontext, krb5_principal kprincipal,
	               krb5_get_init_creds_opt *opts, const QString& password,
	               krb5_timestamp *tgtEndtime);

	static int
	destroyCcache(krb5_context kcontext);
	
	static int
	getCredForwardable(krb5_creds *creds);

	static int
	getCredForwarded(krb5_creds *creds);

	static int
	getCredProxiable(krb5_creds *creds);

	static int
	getCredProxy(krb5_creds *creds);

	static int
	getCredMayPostdate(krb5_creds *creds);

	static int
	getCredPostdated(krb5_creds *creds);

	static int
	getCredInvalid(krb5_creds *creds);

	static int
	getCredRenewable(krb5_creds *creds);

	static int
	getCredInitial(krb5_creds *creds);

	static int
	getCredHwAuth(krb5_creds *creds);

	static int
	getCredPreAuth(krb5_creds *creds);

	static int
	getCredTransitPolicyChecked(krb5_creds *creds);

	static int
	getCredOkAsDelegate(krb5_creds *creds);

	static int
	getCredAnonymous(krb5_creds *creds);

	static krb5_timestamp
	getNow(krb5_context kcontext);

	static QStringList
	getRealms(krb5_context kcontext);

	static size_t
	getPrincipalRealmLength(krb5_principal p);

	static const char *
	getPrincipalRealmData(krb5_principal p);

	static QString
	etype2String(krb5_enctype enctype);

	static QString
	getKrb5ErrorMessage(krb5_context kcontext, krb5_error_code code);
};

#endif /* KRB5_TICKET_WATCHER_V5_H */
