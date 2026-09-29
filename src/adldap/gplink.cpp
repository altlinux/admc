/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
 * Copyright (C) 2023-2024 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QObject>

#include "gplink.h"
#include "adldap.h"

#define LDAP_PREFIX "LDAP://"

Gplink::Gplink() {
}

Gplink::Gplink(const Gplink &other)
    : gpo_list(other.gpo_list), options(other.options) {
    // Do nothing.
}

/**
 * Remove LDAP prefix and make the string lower-case:
 *
 *   "LDAP://cn={UUID},cn=something,DC=a,DC=b"
 *   =>
 *   "cn={uuid},cn=something,dc=a,dc=b"
*/
static QString ldap_to_gpo(const QString &in) {
    QString out = in;
    out.remove(LDAP_PREFIX);
    out = out.toLower();
    return out;
}

Gplink::Gplink(const QString &gplink_string) {
    if (gplink_string.isEmpty()) {
        return;
    }

    // "[gpo_1;option_1][gpo_2;option_2][gpo_3;option_3]..."
    // =>
    // {"gpo_1;option_1", "gpo_2;option_2", "gpo_3;option_3"}
    QString gplink_string_without_brackets = gplink_string;
    gplink_string_without_brackets.replace("[", "");
    const QList<QString> gplink_string_split =
        gplink_string_without_brackets.split(']');

    for (auto part : gplink_string_split) {
        if (part.isEmpty()) {
            continue;
        }

        // "gpo;option"
        // =>
        // gpo and option
        const QList<QString> part_split = part.split(';');

        if (part_split.size() != 2) {
            continue;
        }

        const QString gpo = ldap_to_gpo(part_split[0]);
        const QString option_string = part_split[1];
        const int option = option_string.toInt();

        gpo_list.prepend(gpo);
        options[gpo] = option;
    }
}

Gplink &Gplink::operator=(const Gplink &other) {
    if (this == &other) {
        return *this;
    }

    gpo_list = other.gpo_list;
    options = other.options;
    return *this;
}

/**
 * Convert gpo dn from lower case to gplink case format
 */
static QString gpo_dn_to_gplink_case(const QList<QString> &rdn_list) {
    QList<QString> rdn_list_case;
    for (const QString &rdn : rdn_list) {
        const QList<QString> attribute_value = rdn.split("=");

        // Do no processing if data is malformed
        if (attribute_value.size() != 2) {
            return rdn;
        }

        const QString attribute = attribute_value[0];
        const QString value = attribute_value[1];

        QString attribute_case;
        // "DC" attribute is upper-cased
        if (attribute == "dc") {
            attribute_case = attribute.toUpper();
        } else {
            attribute_case = attribute;
        }

        QString value_case;
        // uuid (the first rdn) is upper-cased
        if (rdn_list.indexOf(rdn) == 0) {
            value_case = value.toUpper();
        } else {
            value_case = value;
        }

        const QList<QString> attribute_value_case = {
            attribute_case,
            value_case,
        };

        const QString rdn_case = attribute_value_case.join("=");

        rdn_list_case.append(rdn_case);
    }

    return rdn_list_case.join(",");
}

// Transform into gplink format. Have to uppercase some
// parts of the output.
QString Gplink::to_string() const {
    QList<QString> part_list;

    QList<QString>::const_reverse_iterator i;
    for (i = gpo_list.rbegin(); i != gpo_list.rend(); ++i) {
        const QString gpo_case = gpo_dn_to_gplink_case(i->split(","));
        const int option = options[*i];
        const QString option_string = QString::number(option);
        const QString part =
            QString("[%1%2;%3]").arg(LDAP_PREFIX, gpo_case, option_string);

        part_list.append(part);
    }

    const QString out = part_list.join("");

    return out;
}

bool Gplink::contains(const QString &gpo_case) const {
    const QString gpo = gpo_case.toLower();

    return options.contains(gpo);
}

static QString get_gpo_case(const QString &gpo) {
    QList<QString> rdn_list = gpo.split(",");
    const bool rdn_list_is_malformed = rdn_list.isEmpty();
    if (rdn_list_is_malformed) {
        return gpo;
    }

    const QString guid_rdn = rdn_list[0];
    rdn_list[0] = guid_rdn.toUpper();
    const uint32_t LIST_SIZE = rdn_list.size();
    for (uint32_t i = 1; i < LIST_SIZE; i++) {
        const QString rdn = rdn_list[i];
        QList<QString> rdn_split = rdn.split("=");

        const bool rdn_is_malformed = (rdn_split.size() != 2);
        if (rdn_is_malformed) {
            continue;
        }

        // Uppercase all rdn left halves
        rdn_split[0] = rdn_split[0].toUpper();

        // Modify some right halves
        if (rdn_split[1] == "system") {
            rdn_split[1] = "System";
        } else if (rdn_split[1] == "policies") {
            rdn_split[1] = "Policies";
        }

        rdn_list[i] = rdn_split.join("=");
    }

    return rdn_list.join(",");
}

QList<QString> Gplink::get_gpo_list() const {
    QList<QString> gpo_list_case;
    for (auto gpo : gpo_list) {
        gpo_list_case.append(get_gpo_case(gpo));
    }
    return gpo_list_case;
}

void Gplink::add(const QString &gpo_case) {
    const QString gpo = gpo_case.toLower();

    const bool gpo_already_in_link = contains(gpo);
    if (gpo_already_in_link) {
        return;
    }

    gpo_list.append(gpo);
    options[gpo] = 0;
}

void Gplink::remove(const QString &gpo_case) {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return;
    }

    gpo_list.removeAll(gpo);
    options.remove(gpo);
}

void Gplink::move_up(const QString &gpo_case) {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return;
    }

    const int current_index = gpo_list.indexOf(gpo);

    if (current_index > 0) {
        const int new_index = current_index - 1;
        gpo_list.move(current_index, new_index);
    }
}

void Gplink::move_down(const QString &gpo_case) {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return;
    }

    const int current_index = gpo_list.indexOf(gpo);

    if (current_index < gpo_list.size() - 1) {
        const int new_index = current_index + 1;

        gpo_list.move(current_index, new_index);
    }
}

void Gplink::move(int from_order, int to_order) {
    if (from_order > (int)gpo_list.size() || to_order > (int)gpo_list.size() ||
            from_order < 1 || to_order < 1) {
        return;
    }

    gpo_list.move(from_order - 1, to_order - 1);
}

bool Gplink::get_option(const QString &gpo_case,
                        const GplinkOption option) const {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return false;
    }

    const int option_bits = options[gpo];
    const bool is_set = bitmask_is_set(option_bits, (int) option);

    return is_set;
}

void Gplink::set_option(const QString &gpo_case,
                        const GplinkOption option,
                        const bool value) {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return;
    }

    const int option_bits = options[gpo];
    const int option_bits_new = bitmask_set(option_bits, (int) option, value);
    options[gpo] = option_bits_new;
}

bool Gplink::equals(const Gplink &other) const {
    return (to_string() == other.to_string());
}

int Gplink::get_gpo_order(const QString &gpo_case) const {
    const QString gpo = gpo_case.toLower();
    const int out = gpo_list.indexOf(gpo) + 1;

    return out;
}

int Gplink::get_max_order() const {
    return gpo_list.size();
}

QStringList Gplink::enforced_gpo_dn_list() const
{
    QStringList enforced_dn_list;
    for (QString gpo_dn : get_gpo_list()) {
        if (get_option(gpo_dn, GplinkOption_Enforced))
            enforced_dn_list.append(gpo_dn);
    }
    return enforced_dn_list;
}

QStringList Gplink::disabled_gpo_dn_list() const
{
    QStringList disabled_dn_list;
    for (QString gpo_dn : get_gpo_list()) {
        if (get_option(gpo_dn, GplinkOption_Disabled))
            disabled_dn_list.append(gpo_dn);
    }
    return disabled_dn_list;
}
