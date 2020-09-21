
#include "gplink.h"

#include <QObject>

#define LDAP_PREFIX "LDAP://"

// NOTE: DN == GPO. But sticking to GPO terminology in this case

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
        QString gpo = part_split[0];
        gpo.replace(LDAP_PREFIX, "", Qt::CaseSensitive);
        const QString option = part_split[1];

        gpos_in_order.append(gpo);
        options[gpo] = option;
    }
}

QString Gplink::to_string() const {
    QString gplink_string;

    for (auto gpo : gpos_in_order) {
        const QString option = options[gpo];

        const QString part = QString("[%1%2;%3]").arg(LDAP_PREFIX, gpo, option);
        
        gplink_string += part;
    }

    return gplink_string;
}

QList<QString> Gplink::get_gpos() const {
    return gpos_in_order;
}

QString Gplink::get_option(const QString &gpo) const {
    if (options.contains(gpo)) {
        return options[gpo];
    } else {
        printf("WARNING: Gplink::get_option() given unknown gpo");
        return "-1";
    }
}

void Gplink::add(const QString &gpo) {
    gpos_in_order.append(gpo);
    options[gpo] = GPLINK_OPTION_NONE;
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

void Gplink::set_option(const QString &gpo, const QString &option) {
    if (options.contains(gpo)) {
        options[gpo] = option;
    }
}
