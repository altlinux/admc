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

#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

/**
 * Interface to AD server. Provides a way to search and
 * modify objects.
 */

#include <QCoreApplication>
#include <QHash>
#include <QSet>

#include "ad_defines.h"

class AdInterfacePrivate;
class QString;
class QByteArray;
class QDateTime;
class AdObject;
class AdConfig;
template <typename T>
class QList;
typedef void TALLOC_CTX;

enum AdMessageType {
    AdMessageType_Success,
    AdMessageType_Error
};

// Some f-ns in this class reuse other f-ns and this
// enum is used to turn off status messages of child
// f-ns which are otherwise displayed by default.
enum DoStatusMsg {
    DoStatusMsg_Yes,
    DoStatusMsg_No
};

class AdCookie {
public:
    AdCookie();
    ~AdCookie();

    bool more_pages() const;

private:
    struct berval *cookie;

    friend class AdInterface;
    friend class AdInterfacePrivate;
};

class AdMessage {

public:
    AdMessage(const QString &text, const AdMessageType &type);

    QString text() const;
    AdMessageType type() const;

private:
    QString m_text;
    AdMessageType m_type;
};

class AdInterface {
    Q_DECLARE_TR_FUNCTIONS(AdInterface)

public:
    AdInterface();
    ~AdInterface();

    /**
     * Set this config instance to be used for all
     * adinterface's going forward. If adconfig is unset,
     * AdInterface defaults to outputting raw attribute
     * values. Note that AdInterface is not responsible for
     * deleting AdConfig instance.
     */
    static void set_config(AdConfig *config);

    static void set_log_searches(const bool enabled);

    static void set_dc(const QString &dc);
    static void set_sasl_nocanon(const bool is_on);
    static void set_port(const int port);
    static void set_cert_strategy(const CertStrategy strategy);
    static void set_domain_is_default(const bool is_default);
    static void set_custom_domain(const QString &domain);

    bool is_connected() const;
    QList<AdMessage> messages() const;
    bool any_error_messages() const;
    void clear_messages();
    AdConfig *adconfig() const;
    QString client_user() const;
    bool logged_in_as_domain_admin();
    QString get_dc() const;
    QString get_domain() const;

    // NOTE: Updates dc for AdInterface instance from static AdInterfacePrivate::s_dc.
    // It is needed when DC changes after AdInterface object was constructed.
    void update_dc();

    // NOTE: If request attributes list is empty, all
    // attributes are returned

    // This is a simplified version that searches all pages
    // in one go
    QHash<QString, AdObject> search(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes, const bool get_sacl = false);

    // This is a more complicated version of search() which
    // separates the search process by pages as they arrive
    // from the server. In general you can use the simpler
    // search(). This version is for cases where you want to
    // display search results as they come in instead of all
    // at once.
    bool search_paged(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes, QHash<QString, AdObject> *results, AdCookie *cookie, const bool get_sacl = false);

    // Simplest search f-n that only searches for attributes
    // of one object
    AdObject search_object(const QString &dn, const QList<QString> &attributes = QList<QString>(), const bool get_sacl = false);

    bool attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg = DoStatusMsg_Yes, const bool set_dacl = false);

    bool attribute_replace_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes, const bool set_dacl = false);
    bool attribute_add_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);

    bool attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_int(const QString &dn, const QString &attribute, const int value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime);

    // NOTE: attrs_map should contain attribute values
    // that will be added to the newly created object.
    // Note that it *must* contain a valid value for
    // objectClass attribute.
    bool object_add(const QString &dn, const QHash<QString, QList<QString>> &attrs_map);
    // Simplified version that only only adds one
    // objectClass value
    bool object_add(const QString &dn, const QString &object_class);

    bool object_delete(const QString &dn, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);

    bool group_add_member(const QString &group_dn, const QString &user_dn);
    bool group_remove_member(const QString &group_dn, const QString &user_dn);
    bool group_set_scope(const QString &dn, GroupScope scope, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool group_set_type(const QString &dn, GroupType type);

    bool user_set_primary_group(const QString &group_dn, const QString &user_dn);
    bool user_set_pass(const QString &dn, const QString &password, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);

    bool computer_reset_account(const QString &dn);

    // "dn_out" is set to the dn of created gpo
    bool gpo_add(const QString &name, QString &dn_out);
    bool gpo_delete(const QString &dn, bool *deleted_object);
    bool gpo_check_perms(const QString &gpo, bool *ok);
    bool gpo_sync_perms(const QString &gpo);
    bool gpo_get_sysvol_version(const AdObject &gpc_object, int *version);

    QString filesys_path_to_smb_path(const QString &filesys_path) const;

private:
    AdInterfacePrivate *d;

    bool ldap_init();
    void ldap_free();
};

QList<QString> get_domain_hosts(const QString &domain, const QString &site);

#endif /* AD_INTERFACE_H */
