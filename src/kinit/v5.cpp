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

#include "v5.h"
#include <stdio.h>
#include <QString>
#include <QStringList>
#include "krb5ticketwatcher.h"

bool
v5::getTgtFromCcache (krb5_context context, krb5_creds *creds)
{
	krb5_ccache ccache;
	krb5_creds mcreds;
	krb5_principal principal, tgt_principal;
	bool ret;

	memset(&ccache, 0, sizeof(ccache));
	ret = FALSE;
	if (krb5_cc_default(context, &ccache) == 0)
	{
		memset(&principal, 0, sizeof(principal));
		if (krb5_cc_get_principal(context, ccache, &principal) == 0)
		{
			memset(&tgt_principal, 0, sizeof(tgt_principal));
			if (krb5_build_principal_ext(context, &tgt_principal,
			                             getPrincipalRealmLength(principal),
			                             getPrincipalRealmData(principal),
			                             KRB5_TGS_NAME_SIZE,
			                             KRB5_TGS_NAME,
			                             getPrincipalRealmLength(principal),
			                             getPrincipalRealmData(principal),
			                             0) == 0)
			{
				memset(creds, 0, sizeof(*creds));
				memset(&mcreds, 0, sizeof(mcreds));
				mcreds.client = principal;
				mcreds.server = tgt_principal;
				if (krb5_cc_retrieve_cred(context, ccache,
				                          0,
				                          &mcreds,
				                          creds) == 0)
				{
					ret = TRUE;
				}
				else
				{
					memset(creds, 0, sizeof(*creds));
				}
				krb5_free_principal(context, tgt_principal);
			}
			krb5_free_principal(context, principal);
		}
		krb5_cc_close(context, ccache);
	}
	return ret;
}

int
v5::credentialCheck(krb5_context kcontext, krb5_principal kprincipal,
                    int promptInterval, krb5_timestamp *tgtEndtime)
{
	krb5_creds my_creds;
	krb5_timestamp now;
	Ktw::reqAction retval = Ktw::none;

	if (!getTgtFromCcache(kcontext, &my_creds))
	{
		tgtEndtime = 0;
		return retval;
	}

	if (krb5_principal_compare (kcontext, my_creds.client, kprincipal))
	{
		krb5_free_principal(kcontext, kprincipal);
		krb5_copy_principal(kcontext, my_creds.client, &kprincipal);
	}
	
	if ((krb5_timeofday(kcontext, &now) == 0) &&
	    (now + (promptInterval * 60) >= my_creds.times.endtime))
	{
		qDebug("now:                   %d", now);
		qDebug("starttime:             %d", my_creds.times.starttime);
		qDebug("endtime:               %d", my_creds.times.endtime);
		qDebug("next Prompt:           %d", (now + (promptInterval * 60)));
		qDebug("renew possible untill: %d", my_creds.times.renew_till);
		if(now + (promptInterval * 60) >= my_creds.times.renew_till)
		{
			retval = Ktw::reinit;
		}
		else
		{		
			retval = Ktw::renew;
		}
	}
	
	*tgtEndtime = my_creds.times.endtime;

	krb5_free_cred_contents(kcontext, &my_creds);

	qDebug("credentials_expiring_real returns: %d", retval);
	
	return retval;
}

int
v5::renewCredential(krb5_context kcontext, krb5_principal kprincipal, krb5_timestamp *tgtEndtime)
{
	krb5_error_code         retval;
	krb5_creds              my_creds;
	krb5_ccache             ccache;
	krb5_get_init_creds_opt opts;
	
	qDebug("renew called");

	if (kprincipal == NULL)
	{
		qDebug("No principal name");

		return KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN;
	}

	krb5_get_init_creds_opt_init(&opts);
	if (getTgtFromCcache (kcontext, &my_creds))
	{
		qDebug("got tgt from ccache");
		//setOptionsUsingCreds(kcontext, &my_creds, &opts);
		*tgtEndtime = my_creds.times.endtime;
		krb5_free_cred_contents(kcontext, &my_creds);
	}
	else
	{
		qDebug("TGT expired");
		*tgtEndtime = 0;
	}

	retval = krb5_cc_default(kcontext, &ccache);
	if (retval)
		return retval;

	retval = krb5_get_renewed_creds(kcontext, &my_creds, kprincipal, ccache,
	                                NULL);
	
	qDebug("krb5_get_renewed_creds returned: %d", retval);
	if (retval)
		goto out;
	
	retval = krb5_cc_initialize(kcontext, ccache, kprincipal);
	if (retval)
		goto out;
	
	retval = krb5_cc_store_cred(kcontext, ccache, &my_creds);
	if (retval)
		goto out;
	
	*tgtEndtime = my_creds.times.endtime;
	
out:
	krb5_free_cred_contents(kcontext, &my_creds);
	krb5_cc_close (kcontext, ccache);
	
	return retval;
}

int
v5::initCredential(krb5_context kcontext, krb5_principal kprincipal,
                    krb5_get_init_creds_opt *opts, const QString& password,
                    krb5_timestamp *tgtEndtime)
{
	krb5_error_code retval;
	krb5_creds      my_creds;
	krb5_ccache     ccache;
	
	qDebug("call initCredential");

	retval = krb5_get_init_creds_password(kcontext, &my_creds, kprincipal,
	                                      password.toUtf8().data(), NULL, NULL,
	                                      0, NULL, opts);
	if (retval)
	{
		return retval;
	}
	
	retval = krb5_cc_default(kcontext, &ccache);
	if (retval)
	{
		goto out;
	}
	
	retval = krb5_cc_initialize(kcontext, ccache, kprincipal);
	if (retval)
		goto out;
	
	retval = krb5_cc_store_cred(kcontext, ccache, &my_creds);
	if (retval)
		goto out;
	
	*tgtEndtime = my_creds.times.endtime;
	
out:
	krb5_free_cred_contents (kcontext, &my_creds);
	krb5_cc_close (kcontext, ccache);
	
	return retval;
}

int
v5::destroyCcache(krb5_context kcontext)
{
	int code = 0;
	krb5_ccache cache = NULL;

	qDebug("destroyCcache called");
	
	code = krb5_cc_default(kcontext, &cache);
	if (code)
	{
		return code;
	}

	code = krb5_cc_destroy (kcontext, cache);
	if (code != 0  && code != KRB5_FCC_NOFILE)
	{
		return code;
	}

	return 0;
}

int
v5::getCredForwardable(krb5_creds *creds)
{
        return creds->ticket_flags & TKT_FLG_FORWARDABLE;
}

int
v5::getCredForwarded(krb5_creds *creds)
{
        return creds->ticket_flags & TKT_FLG_FORWARDED;
}

int
v5::getCredProxiable(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_PROXIABLE;
}

int
v5::getCredProxy(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_PROXY;
}

int
v5::getCredMayPostdate(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_MAY_POSTDATE;
}

int
v5::getCredPostdated(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_POSTDATED;
}

int
v5::getCredInvalid(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_INVALID;
}

int
v5::getCredRenewable(krb5_creds *creds)
{
        return creds->ticket_flags & TKT_FLG_RENEWABLE;
}

int
v5::getCredInitial(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_INITIAL;
}

int
v5::getCredHwAuth(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_HW_AUTH;
}

int
v5::getCredPreAuth(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_PRE_AUTH;
}

int
v5::getCredTransitPolicyChecked(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_TRANSIT_POLICY_CHECKED;
}

int
v5::getCredOkAsDelegate(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_OK_AS_DELEGATE;
}

int
v5::getCredAnonymous(krb5_creds *creds)
{
	return creds->ticket_flags & TKT_FLG_ANONYMOUS;
}

krb5_timestamp
v5::getNow(krb5_context kcontext)
{
	krb5_timestamp now;
	int e = krb5_timeofday(kcontext, &now);
	if(e != 0)
	{
		qWarning("Cannot get current time: %s", strerror(e));
		return 0;
	}
	return now;
}

/*
QStringList
v5::getRealms(krb5_context kcontext)
{
	krb5_error_code retval;
	char *realm;
	void *iter;
	QStringList list;
	char *r = NULL;
	
	krb5_get_default_realm(kcontext, &r);
	QString defRealm(r);
	krb5_free_default_realm(kcontext, r);

	if ((retval = krb5_realm_iterator_create(kcontext, &iter)))
	{
		qWarning("krb5_realm_iterator_create failed: %d", retval);
		return list;
	}
	while (iter)
	{
		if ((retval = krb5_realm_iterator(kcontext, &iter, &realm)))
		{
			qWarning("krb5_realm_iterator failed: %d", retval);
			krb5_realm_iterator_free(kcontext, &iter);
			return list;
		}
		if (realm)
		{
			if(list.contains(realm) == 0)
			{
				if(defRealm == realm)
				{
					list.push_front(defRealm);
				}
				else
				{
					list.push_back(realm);
				}
			}
			krb5_free_realm_string(kcontext, realm);
		}
	}
	krb5_realm_iterator_free(kcontext, &iter);
	return list;
}
*/

size_t
v5::getPrincipalRealmLength(krb5_principal p)
{
	return p->realm.length;
}

const char *
v5::getPrincipalRealmData(krb5_principal p)
{
	return p->realm.data;
}

QString
v5::etype2String(krb5_enctype enctype)
{
	static char buf[100];
	krb5_error_code retval;
	
	if((retval = krb5_enctype_to_string(enctype, buf, sizeof(buf))))
	{
		/* XXX if there's an error != EINVAL, I should probably report it */
		sprintf(buf, "etype %d", enctype);
	}

	return buf;
}

QString
v5::getKrb5ErrorMessage(krb5_context kcontext, krb5_error_code code)
{
	const char *message = krb5_get_error_message(kcontext, code);
	QString msg(message);
	krb5_free_error_message(kcontext, message);
	return msg;
}
	
