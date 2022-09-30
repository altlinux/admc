#ifndef  KRB5_TW_GETTEXT_H
#define  KRB5_TW_GETTEXT_H

#include <libintl.h>

#define ki18n(s, ...) QString::fromUtf8(dgettext("krb5-ticket-watcher", s))

#endif

