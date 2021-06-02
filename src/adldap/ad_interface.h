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
class SecurityACE;
class SecurityDescriptor;
template <typename T> class QList;
typedef void TALLOC_CTX;
struct security_descriptor;
struct security_ace;
struct dom_sid;

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    // NOTE: Descendants scope appears to not work, from the
    // ldap_search manual: "Note that the latter requires
    // the server support the LDAP Subordinates Search Scope
    // extension."
    SearchScope_Descendants,
    SearchScope_All,
};

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
    /**
     * Pass an loaded AdConfig instance to ctor to enable
     * attribute display values in messages. Without an
     * AdConfig instance AdInterface defaults to outputting
     * raw attribute values. Note that AdInterface is not
     * responsible for deleting AdConfig instance.
     */
    AdInterface(AdConfig *adconfig = nullptr);
    ~AdInterface();

    /**
     * If you wish to use the same AdConfig instance for all
     * connections, you can setup a permanent one here. Note
     * that if you pass another adconfig to ctor that will
     * override the permanent adconfig.
     */
    static void set_permanent_adconfig(AdConfig *adconfig);
    
    static void set_log_searches(const bool enabled);

    static void set_dc(const QString &dc);
    static QString get_dc();

    bool is_connected() const;
    QList<AdMessage> messages() const;
    bool any_error_messages() const;
    void clear_messages();

    // NOTE: If request attributes list is empty, all attributes are returned

    // This is a simplified version that searches all pages
    // in one go
    QHash<QString, AdObject> search(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes);

    // This is a more complicated version of search() which
    // separates the search process by pages as they arrive
    // from the server. In general you can use the simpler
    // search(). This version is specifically for cases
    // where you need to do something between pages, like
    // processing UI events so it doesn't freeze.
    bool search_paged(const QString &base, const SearchScope scope, const QString &filter, const QList<QString> &attributes, QHash<QString, AdObject> *results, AdCookie *cookie);

    // Simplest search f-n that only searches for attributes
    // of one object
    AdObject search_object(const QString &dn, const QList<QString> &attributes = QList<QString>());

    bool attribute_replace_values(const QString &dn, const QString &attribute, const QList<QByteArray> &values, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    
    bool attribute_replace_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_add_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_delete_value(const QString &dn, const QString &attribute, const QByteArray &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    
    bool attribute_replace_string(const QString &dn, const QString &attribute, const QString &value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_int(const QString &dn, const QString &attribute, const int value, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool attribute_replace_datetime(const QString &dn, const QString &attribute, const QDateTime &datetime);
    bool attribute_replace_security_descriptor(const QString &dn, const QHash<QByteArray, QHash<AcePermission, PermissionState>> &descriptor_state);

    bool object_add(const QString &dn, const QString &object_class);
    bool object_delete(const QString &dn);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);

    bool group_add_member(const QString &group_dn, const QString &user_dn);
    bool group_remove_member(const QString &group_dn, const QString &user_dn);
    bool group_set_scope(const QString &dn, GroupScope scope, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    bool group_set_type(const QString &dn, GroupType type);

    bool user_set_primary_group(const QString &group_dn, const QString &user_dn);
    bool user_set_pass(const QString &dn, const QString &password);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);

    // "dn_out" is set to the dn of created gpo
    bool create_gpo(const QString &name, QString &dn_out);
    bool delete_gpo(const QString &dn);

    QString sysvol_path_to_smb(const QString &sysvol_path) const;

    QString get_trustee_name(const QByteArray &trustee);

private:
    AdInterfacePrivate *d;
};

class SecurityDescriptor {

public:
    struct security_descriptor *data;

    SecurityDescriptor(const QByteArray &descriptor_bytes);
    ~SecurityDescriptor();

    QList<QByteArray> get_trustee_list() const;
    QList<security_ace *> dacl() const;
    QList<security_ace *> get_ace_list(const QByteArray &trustee) const;
    void print_acl(const QByteArray &trustee) const;

private:
    TALLOC_CTX *mem_ctx;

};

extern const QList<uint32_t> sec_masks;
extern const QHash<AcePermission, uint32_t> ace_permission_to_mask_map;
extern const QHash<AcePermission, QString> ace_permission_to_type_map;

QList<QString> get_domain_hosts(const QString &domain, const QString &site);
QByteArray dom_sid_to_bytes(const dom_sid &sid);

extern const QList<AcePermission> all_permissions_list;
extern const QSet<AcePermission> all_permissions;
extern const QSet<AcePermission> read_prop_permissions;
extern const QSet<AcePermission> write_prop_permissions;

#endif /* AD_INTERFACE_H */
