/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

#include "ad_defines.h"
#include "ad_object.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QDateTime>

// Interface to AD/LDAP
// Emits modified() signal after performing an action that
// modifies data on the AD server
// Reload GUI state when that signal is emitted

enum ConnectResult {
    ConnectResult_Success,
    ConnectResult_FailedToFindHosts,
    ConnectResult_FailedToConnect,
};

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    SearchScope_Descendants,
    SearchScope_All,
};

// TODO: follow same logic as ldap f-n, "If no attrs are listed, all user attributes are returned. If only 1.1 is listed, no attributes will be returned."
#define SEARCH_ALL_ATTRIBUTES "SEARCH_ALL_ATTRIBUTES"

typedef struct ldap LDAP;
typedef struct _SMBCCTX SMBCCTX;
class AdConfig;

class AdInterface final : public QObject {
Q_OBJECT

private:
    enum DoStatusMsg {
        DoStatusMsg_Yes,
        DoStatusMsg_No
    };

public:
    static AdInterface *instance();

    ConnectResult connect();

    void refresh();

    // Use this if you are doing a series of AD modifications.
    // During the batch, modified() signals won't be emitted
    // and once the batch is complete one modified() signal
    // is emitted, so that GUI is reloaded only once.
    void start_batch();
    void end_batch();

    AdConfig *config() const;
    QString domain() const;
    QString domain_head() const;
    QString host() const;
    QString configuration_dn() const;
    QString schema_dn() const;

    // NOTE: search f-ns need to communicate with the AD server, 
    // so use them only for infrequent calls. Also try to ask
    // only for attributes that you need.
    QHash<QString, AdObject> search(const QString &filter, const QList<QString> &attributes, const SearchScope scope_enum, const QString &search_base = QString());
    AdObject search_object(const QString &dn, const QList<QString> &attributes = {SEARCH_ALL_ATTRIBUTES});

    bool attribute_add(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    
    bool attribute_add_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);

    bool attribute_replace_int(const QString &dn, const QString &attribute, const int value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime);

    bool object_add(const QString &dn, const QString &object_class);
    bool object_delete(const QString &dn);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);

    bool group_add_user(const QString &group_dn, const QString &user_dn);
    bool group_remove_user(const QString &group_dn, const QString &user_dn);
    bool group_set_primary_for_user(const QString &group_dn, const QString &user_dn);
    bool group_set_scope(const QString &dn, GroupScope scope);
    bool group_set_type(const QString &dn, GroupType type);

    bool user_set_pass(const QString &dn, const QString &password);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);

    bool object_can_drop(const QString &dn, const QString &target_dn);
    void object_drop(const QString &dn, const QString &target_dn);

    bool create_gpo(const QString &name);
    bool delete_gpo(const QString &dn);

    QString sysvol_path_to_smb(const QString &sysvol_path) const;

signals:
    void modified();

private:
    LDAP *ld;
    SMBCCTX *smbc;
    AdConfig *m_config;
    QString m_domain;
    QString m_domain_head;
    QString m_configuration_dn;
    QString m_schema_dn;
    QString m_host;

    bool batch_in_progress = false;
        
    AdInterface();

    void emit_modified();
    void success_status_message(const QString &msg, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    void error_status_message(const QString &context, const QString &error, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    QString default_error() const;

    AdInterface(const AdInterface&) = delete;
    AdInterface& operator=(const AdInterface&) = delete;
    AdInterface(AdInterface&&) = delete;
    AdInterface& operator=(AdInterface&&) = delete;
};

AdInterface *AD();


#endif /* AD_INTERFACE_H */
