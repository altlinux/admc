/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef GPLINK_H
#define GPLINK_H

#include <QHash>
#include <QList>
#include <QString>

enum GplinkOption {
    GplinkOption_NoOption,
    GplinkOption_Disabled,
    GplinkOption_Enforced
};

/**
 * Class to store a gplink attribute for easy manipulation.
 * Gplink attribute primer: an ordered list of GPO container
 * DN's with each GPO being assigned an "option" value.
 * Options specify whether policy is disabled and/or
 * enforced.
 */

class Gplink {
public:
    Gplink();
    Gplink(const Gplink &other);
    Gplink(const QString &gplink_string);

    Gplink &operator=(const Gplink &other);

    QString to_string() const;
    bool contains(const QString &gpo) const;

    // NOTE: this returns a list of GPO DN's with correct
    // letter case. Gplink string contains DN's in a
    // different letter case that is not appropriate for
    // LDAP operations.
    QList<QString> get_gpo_list() const;

    void add(const QString &gpo_case);
    void remove(const QString &gpo_case);
    void move_up(const QString &gpo_case);
    void move_down(const QString &gpo_case);
    void move(int from_order, int to_order);

    bool get_option(const QString &gpo, const GplinkOption option) const;
    void set_option(const QString &gpo, const GplinkOption option, const bool value);

    bool equals(const Gplink &other) const;

    int get_gpo_order(const QString &gpo) const;
    int get_max_order() const;

    QStringList enforced_gpo_dn_list() const;
    QStringList disabled_gpo_dn_list() const;

private:
    QList<QString> gpo_list;
    QHash<QString, int> options;
};

#endif /* GPLINK_H */
