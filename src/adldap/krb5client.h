/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#ifndef KRB5CLIENT_H
#define KRB5CLIENT_H

#include <memory>
#include <QDateTime>

enum Krb5TgtState {
    Krb5TgtState_Active, // Ticket (TGT) is valid and a up-to-date
    Krb5TgtState_Expired, // TGT is expired, but can be refreshed
    Krb5TgtState_Outdated, // TGT lifetime is out of date and can't be refreshed
    Krb5TgtState_Invalid // TGT is not valid for some reason
};

struct Krb5TGTData {
    Krb5TgtState state = Krb5TgtState_Invalid;
    QString principal;
    QString realm;
    QDateTime starts;
    QDateTime expires;
    QDateTime renew_until;
};

class Krb5Client final {
public:
    explicit Krb5Client();
    ~Krb5Client();

    void authenticate(const QString &principal, const QString &password);
    void set_current_principal(const QString &principal);
    void refresh_tgt(const QString &principal);
    Krb5TGTData tgt_data(const QString &principal) const;
    QString current_principal() const;
    QString system_principal() const;
    bool principal_has_cache(const QString &principal) const;
    QStringList available_principals() const;
    QStringList active_tgt_principals() const;

private:
    class Krb5ClientImpl;
    std::unique_ptr<Krb5ClientImpl> impl;
};

#endif // KRB5CLIENT_H
