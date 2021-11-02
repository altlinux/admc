
#include "gplink.h"
#include "adldap.h"

#include <QObject>

#define LDAP_PREFIX "LDAP://"

Gplink::Gplink() {
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
    const QList<QString> gplink_string_split = gplink_string_without_brackets.split(']');

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

        // "LDAP://cn={UUID},cn=something,DC=a,DC=b"
        // =>
        // "cn={uuid},cn=something,dc=a,dc=b"
        const QString gpo = [&]() {
            QString out;

            out = part_split[0];
            out.remove(LDAP_PREFIX);
            out = out.toLower();

            return out;
        }();

        const int option = [&]() {
            const QString option_string = part_split[1];
            const int option_int = option_string.toInt();

            return option_int;
        }();

        gpo_list.append(gpo);
        options[gpo] = option;
    }
}

// Transform into gplink format. Have to uppercase some
// parts of the output.
QString Gplink::to_string() const {
    QList<QString> part_list;

    for (auto gpo : gpo_list) {
        // Convert gpo dn from lower case to gplink case
        // format
        const QString gpo_case = [&]() {
            const QList<QString> rdn_list = gpo.split(",");

            QList<QString> rdn_list_case;

            for (const QString &rdn : rdn_list) {
                const QString rdn_case = [&]() {
                    const QList<QString> attribute_value = rdn.split("=");

                    // Do no processing if data is malformed
                    if (attribute_value.size() != 2) {
                        return rdn;
                    }

                    const QString attribute = attribute_value[0];
                    const QString value = attribute_value[1];

                    const QString attribute_case = [&]() {
                        // "DC" attribute is upper-cased
                        if (attribute == "dc") {
                            return attribute.toUpper();
                        } else {
                            return attribute;
                        }
                    }();

                    const QString value_case = [&]() {
                        // uuid (the first rdn) is upper-cased
                        if (rdn_list.indexOf(rdn) == 0) {
                            return value.toUpper();
                        } else {
                            return value;
                        }
                    }();

                    const QList<QString> attribute_value_case = {
                        attribute_case,
                        value_case,
                    };

                    const QString out = attribute_value_case.join("=");

                    return out;
                }();

                rdn_list_case.append(rdn_case);
            }

            const QString out = rdn_list_case.join(",");

            return out;
        }();
        const int option = options[gpo];
        const QString option_string = QString::number(option);

        const QString part = QString("[%1%2;%3]").arg(LDAP_PREFIX, gpo_case, option_string);

        part_list.append(part);
    }

    const QString out = part_list.join("");

    return out;
}

bool Gplink::contains(const QString &gpo_case) const {
    const QString gpo = gpo_case.toLower();

    return options.contains(gpo);
}

QList<QString> Gplink::get_gpo_list() const {
    return gpo_list;
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

bool Gplink::get_option(const QString &gpo_case, const GplinkOption option) const {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return false;
    }

    const int option_bits = options[gpo];
    const bool is_set = bit_is_set(option_bits, (int) option);

    return is_set;
}

void Gplink::set_option(const QString &gpo_case, const GplinkOption option, const bool value) {
    const QString gpo = gpo_case.toLower();

    if (!contains(gpo)) {
        return;
    }

    const int option_bits = options[gpo];
    const int option_bits_new = bit_set(option_bits, (int) option, value);
    options[gpo] = option_bits_new;
}

bool Gplink::equals(const Gplink &other) const {
    return (to_string() == other.to_string());
}
