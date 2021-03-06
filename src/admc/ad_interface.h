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
 * modify objects . Success and error messages resulting
 * from operations are sent to Status.
 */

#include <QObject>
#include <QCoreApplication>

#include "ad_defines.h"
#include "utils.h"

enum SearchScope {
    SearchScope_Object,
    SearchScope_Children,
    // NOTE: Descendants scope appears to not work, from the ldap_search manual: "Note that the latter requires the server support the LDAP Subordinates Search Scope extension."
    SearchScope_Descendants,
    SearchScope_All,
};

class QString;
class QByteArray;
class QDateTime;
class AdObject;
template <typename T> class QList;
typedef struct ldap LDAP;
typedef struct _SMBCCTX SMBCCTX;

class AdCookie {
public:
    AdCookie();
    ~AdCookie();

    bool more_pages() const;

private:
    struct berval *cookie;

    friend class AdInterface;

    DISABLE_COPY_MOVE(AdCookie);
};


// TODO: have to put signals in a singleton since
// adinterface stopped being a singleton. Console needs to
// connect to these signals forever. Will probably change
// this to something better, might just remove these
// signals.
class AdSignals final : public QObject {
Q_OBJECT

public:
    using QObject::QObject;

signals:
    // These signals are for ObjectModel
    void object_added(const QString &dn);
    void object_deleted(const QString &dn);
    void object_changed(const QString &dn);
};

AdSignals *ADSIGNALS();

enum AdMessageType {
    AdMessageType_Success,
    AdMessageType_Error
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

private:
    // Some f-ns in this class reuse other f-ns and this
    // enum is used to turn off status messages of child
    // f-ns which are otherwise displayed by default.
    enum DoStatusMsg {
        DoStatusMsg_Yes,
        DoStatusMsg_No
    };

public:
    AdInterface();
    ~AdInterface();

    bool is_connected() const;
    QList<AdMessage> messages() const;
    bool any_error_messages() const;
    void clear_messages();

    // NOTE: If request attributes list is empty, all attributes are returned

    // This is a simplified version that searches all pages
    // in one go
    QHash<QString, AdObject> search(const QString &filter, const QList<QString> &attributes, const SearchScope scope, const QString &search_base = QString());

    // This is a more complicated version of search() which
    // separates the search process by pages as they arrive
    // from the server. In general you can use the simpler
    // search(). This version is specifically for cases
    // where you need to do something between pages, like
    // processing UI events so it doesn't freeze.
    bool search_paged(const QString &filter_arg, const QList<QString> &attributes_arg, const SearchScope scope_arg, const QString &search_base_arg, AdCookie *cookie, QHash<QString, AdObject> *out);

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

    bool object_add(const QString &dn, const QString &object_class);
    bool object_delete(const QString &dn);
    bool object_move(const QString &dn, const QString &new_container);
    bool object_rename(const QString &dn, const QString &new_name);

    bool group_add_member(const QString &group_dn, const QString &user_dn);
    bool group_remove_member(const QString &group_dn, const QString &user_dn);
    bool group_set_scope(const QString &dn, GroupScope scope);
    bool group_set_type(const QString &dn, GroupType type);

    bool user_set_primary_group(const QString &group_dn, const QString &user_dn);
    bool user_set_pass(const QString &dn, const QString &password);
    bool user_set_account_option(const QString &dn, AccountOption option, bool set);
    bool user_unlock(const QString &dn);

    bool create_gpo(const QString &name);
    bool delete_gpo(const QString &dn);

    QString sysvol_path_to_smb(const QString &sysvol_path) const;

private:
    LDAP *ld;
    SMBCCTX *smbc;
    bool m_is_connected;
    QString domain;
    QString domain_head;
    QString host;
    QList<AdMessage> m_messages;

    void success_status_message(const QString &msg, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    void error_status_message(const QString &context, const QString &error, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    QString default_error() const;
    int get_ldap_result() const;
    bool search_paged_internal(const char *filter, char **attributes, const int scope, const char *search_base, QHash<QString, AdObject> *out, AdCookie *cookie);

    DISABLE_COPY_MOVE(AdInterface);
};

// TODO: haven't found a better place for ad_is_connected()
// f-n yet. Should move it somewhere i think because
// adinterface needs to be separate from GUI.

// Wrappers over is_connected() that also open an error
// messagebox if failed to connect. You should generally use
// these in GUI code instead of is_connected().
bool ad_connected(const AdInterface &ad);
bool ad_failed(const AdInterface &ad);

#endif /* AD_INTERFACE_H */
