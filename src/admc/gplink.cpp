
#include "gplink.h"

#include <QObject>

#define LDAP_PREFIX "LDAP://"

const QHash<GplinkOption, int> gplink_option_to_int_map = {
    {GplinkOption_None,     0},
    {GplinkOption_Disable,  1},
    {GplinkOption_Enforce,  2},
    {GplinkOption_COUNT,    -1},
};

GplinkOption gplink_option_from_string(const QString &option_string) {
    const int option_int = option_string.toInt();
    const QList<GplinkOption> keys = gplink_option_to_int_map.keys(option_int);
    const GplinkOption option = keys[0];

    return option;
};

QString gplink_option_to_string(const GplinkOption option) {
    const int option_int = gplink_option_to_int_map[option];
    const QString option_string = QString::number(option_int);

    return option_string;
}

QString gplink_option_to_display_string(const GplinkOption option) {
    switch (option) {
        case GplinkOption_None: return QObject::tr("None");
        case GplinkOption_Disable: return QObject::tr("Disable");
        case GplinkOption_Enforce: return QObject::tr("Enforce");
        case GplinkOption_COUNT: {};
    }

    return QObject::tr("UNKNOWN OPTION");
};

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
        const QString option_string = part_split[1];
        const GplinkOption option = gplink_option_from_string(option_string);

        gpos_in_order.append(gpo);
        options[gpo] = option;
    }
}

QString Gplink::to_string() const {
    QString gplink_string;

    for (auto gpo : gpos_in_order) {
        const GplinkOption option = options[gpo];
        const QString option_string = gplink_option_to_string(option);

        const QString part = QString("[%1%2;%3]").arg(LDAP_PREFIX, gpo, option_string);
        
        gplink_string += part;
    }

    return gplink_string;
}

void Gplink::add(const QString &gpo) {
    gpos_in_order.append(gpo);
    options[gpo] = GplinkOption_None;
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
