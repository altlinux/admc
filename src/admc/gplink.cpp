
#include "gplink.h"
#include "adldap.h"

#include <QObject>

#define LDAP_PREFIX "LDAP://"

// TODO: confirm that input gplink is valid. Do sanity checks?

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

        QString gpo = part_split[0];
        gpo.replace(LDAP_PREFIX, "", Qt::CaseSensitive);

        const QString option_string = part_split[1];
        const int option = option_string.toInt();

        gpos_in_order.append(gpo);
        options[gpo] = option;
    }
}

QString Gplink::to_string() const {
    QString gplink_string;

    for (auto gpo : gpos_in_order) {
        const int option = options[gpo];
        const QString option_string = QString::number(option);

        const QString part = QString("[%1%2;%3]").arg(LDAP_PREFIX, gpo, option_string);
        
        gplink_string += part;
    }

    return gplink_string;
}

QList<QString> Gplink::get_gpos() const {
    return gpos_in_order;
}

bool Gplink::contains(const QString &gpo) const {
    return gpos_in_order.contains(gpo);
}

void Gplink::add(const QString &gpo) {
    gpos_in_order.append(gpo);
    options[gpo] = 0;
}

void Gplink::remove(const QString &gpo) {
    gpos_in_order.removeAll(gpo);
    options.remove(gpo);
}

void Gplink::move_up(const QString &gpo) {
    const int current_index = gpos_in_order.indexOf(gpo);

    if (current_index > 0) {
        const int new_index = current_index - 1;
        gpos_in_order.move(current_index, new_index);
    }
}

void Gplink::move_down(const QString &gpo) {
    const int current_index = gpos_in_order.indexOf(gpo);
    
    if (current_index < gpos_in_order.size() - 1) {
        const int new_index = current_index + 1;

        gpos_in_order.move(current_index, new_index);
    }
}

bool Gplink::get_option(const QString &gpo, const GplinkOption option) const {
    const int option_bits = options[gpo];
    const bool is_set = bit_is_set(option_bits, (int) option);

    return is_set;
}

void Gplink::set_option(const QString &gpo, const GplinkOption option, const bool value) {
    const int option_bits = options[gpo];
    const int option_bits_new = bit_set(option_bits, (int) option, value);
    options[gpo] = option_bits_new;
}
