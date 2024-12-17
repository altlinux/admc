/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AD_INTERFACE_P_H
#define AD_INTERFACE_P_H

#include <QCoreApplication>
#include <QList>
#include <QMutex>

#include "samba/smb_context.h"

class AdInterface;
class AdConfig;
class QString;
typedef struct ldap LDAP;

class AdInterfacePrivate {
    Q_DECLARE_TR_FUNCTIONS(AdInterfacePrivate)

    friend AdInterface;
    static QMutex mutex;

public:
    AdInterfacePrivate(AdInterface *q);

    LDAP *ld;
    bool is_connected;
    QString domain;
    QString dc;
    QString client_user;
    QList<AdMessage> messages;

    void success_message(const QString &msg, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    void error_message(const QString &context, const QString &error, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    void error_message_plain(const QString &text, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    QString default_error() const;
    int get_ldap_result() const;
    bool search_paged_internal(const char *base, const int scope, const char *filter, char **attributes, QHash<QString, AdObject> *results, AdCookie *cookie, const bool get_sacl);
    bool connect_via_ldap(const char *uri);
    bool delete_gpt(const QString &parent_path);
    bool smb_path_is_dir(const QString &path, bool *ok);

    // Returns GPT contents including the root path, in
    // order of increasing depth, so root path is first
    QList<QString> gpo_get_gpt_contents(const QString &gpt_root_path, bool *ok);

private:
    static AdConfig *adconfig;
    static bool s_log_searches;
    static QString s_dc;
    static void *s_sasl_nocanon;
    static int s_port;
    static bool s_domain_is_default;
    static QString s_custom_domain;
    static CertStrategy s_cert_strat;
    static SMBContext s_smb_context;

    AdInterface *q;
};

#endif /* AD_INTERFACE_P_H */
