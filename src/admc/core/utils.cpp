/*
 * ADMC - AD Management Center
 *
 * This file contains common utilities for the ADMC core functionality.
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include <QList>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QStandardItem>
#include <QString>
#include <QVariant>

#include "adldap.h"
#include "ad_object.h"
#include "core/ad.h"
#include "core/globals.h"
#include "core/settings.h"
#include "core/utils.h"

/**
 * Convert a string list to a variant list, return the newly created list.
 */
QList<QVariant> string_list_to_variant_list(const QList<QString> &string_list) {
    QList<QVariant> out;

    for (const QString &string : string_list) {
        const QVariant variant = QVariant(string);
        out.append(variant);
    }

    return out;
}

QList<QString> variant_list_to_string_list(const QList<QVariant> &variant_list) {
    QList<QString> out;

    for (const QVariant &variant : variant_list) {
        const QString string = variant.toString();
        out.append(string);
    }

    return out;
}

QString generate_new_name(const QList<QString> &existing_name_list, const QString &name_base) {
    const QString first = name_base;
    if (!existing_name_list.contains(first)) {
        return first;
    }

    int n = 2;

    auto get_name = [&]() {
        return QString("%1 (%2)").arg(name_base).arg(n);
    };

    while (existing_name_list.contains(get_name())) {
        n++;

        // NOTE: new name caps out at 1000 as a reasonable
        // limit, not a bug
        if (n > 1000) {
            break;
        }
    }

    return get_name();
}

bool string_contains_bad_chars(const QString &string, const QString &bad_chars) {
    const QString bad_chars_escaped = QRegularExpression::escape(bad_chars);
    const QString regexp_string = QString("[%1]").arg(bad_chars_escaped);
    const QRegularExpression regexp = QRegularExpression(regexp_string);

    const bool out = string.contains(regexp);

    return out;

}

// NOTE: not included in base lib, so had to copypaste. Maybe find some other
// more popular implementation and use that (with appropriate license).
// Preferrably something that automatically pads the result (leading 0's).
// -- Dmitry Degtyarev
/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char *itoa(int value, char *result, int base) {
    // check that the base is valid
    if (base < 2 || base > 36) {
        *result = '\0';
        return result;
    }

    char *ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0)
        *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

/**
 * Make a list of QStandardItem instances of the specified size.
 *
 * @param count Number of items to create.
 * @return A list of items.
 */
QList<QStandardItem *> make_item_row(const int count) {
    QList<QStandardItem *> row;

    for (int i = 0; i < count; i++) {
        const auto item = new QStandardItem();
        row.append(item);
    }

    return row;
}

void set_data_for_row(const QList<QStandardItem *> &row,
                      const QVariant &data,
                      const int role) {
    for (QStandardItem *item : row) {
        item->setData(data, role);
    }
}

QList<QPersistentModelIndex> persistent_index_list(
    const QList<QModelIndex> &indexes)
{
    QList<QPersistentModelIndex> out;

    for (const QModelIndex &index : indexes) {
        out.append(QPersistentModelIndex(index));
    }

    return out;
}

QList<QModelIndex> normal_index_list(
    const QList<QPersistentModelIndex> &indexes)
{
    QList<QModelIndex> out;

    for (const QPersistentModelIndex &index : indexes) {
        out.append(QModelIndex(index));
    }

    return out;
}

/**
 * Make a validator which ensures that a string contains only decimal numbers.
 *
 * @return A new validator instance.
 */
QRegularExpressionValidator* make_decimal_numbers_validator(QObject *parent) {
    return new QRegularExpressionValidator(QRegularExpression("[0-9]*"),
                                           parent);
}

QString get_classes_filter(const QList<QString> &class_list) {
    QList<QString> class_filters;
    for (const QString &object_class : class_list) {
        const QString class_filter = filter_CONDITION(Condition_Equals,
                                                      ATTRIBUTE_OBJECT_CLASS,
                                                      object_class);
        class_filters.append(class_filter);
    }

    const QString out = filter_OR(class_filters);

    return out;
}

// Hide advanced view only" objects if advanced view setting
// is off
QString advanced_features_filter(const QString &filter) {
    const bool advanced_features_OFF =
        (! settings_get_bool(SETTING_advanced_features));

    if (advanced_features_OFF) {
        const QString advanced_features = filter_CONDITION(
            Condition_NotEquals,
            ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY,
            LDAP_BOOL_TRUE);
        const QString out = filter_AND({ filter, advanced_features });

        return out;
    } else {
        return filter;
    }
}

QString is_container_filter() {
    const QList<QString> accepted_classes = g_adconfig->get_filter_containers();
    const QString out = get_classes_filter(accepted_classes);
    return out;
}

QString gpo_status_from_int(int status) {
    switch (status) {
    case 0:
        return QObject::tr("Enabled");
    case 1:
        return QObject::tr("User configuration disabled");
    case 2:
        return QObject::tr("Computer configuration disabled");
    case 3:
        return QObject::tr("Disabled");
    default:
        return QObject::tr("Undefined GPO status");
    }
}

QList<QString> index_list_to_dn_list(const QList<QModelIndex> &index_list,
                                     const int dn_role) {
    QList<QString> out;

    for (const QModelIndex &index : index_list) {
        const QString dn = index.data(dn_role).toString();
        out.append(dn);
    }

    return out;
}

// NOTE: configuration and schema objects are hidden so that
// they don't show up in regular searches. Have to use
// search_object() and manually add them to search results.
void dev_mode_search_results(QHash<QString, AdObject> &results,
                             AdInterface &ad,
                             const QString &base) {
    const bool dev_mode = settings_get_bool(SETTING_feature_dev_mode);
    if (! dev_mode) {
        return;
    }

    const QString domain_dn = g_adconfig->domain_dn();
    const QString configuration_dn = g_adconfig->configuration_dn();
    const QString schema_dn = g_adconfig->schema_dn();

    if (base == domain_dn) {
        results[configuration_dn] = ad.search_object(configuration_dn);
    } else if (base == configuration_dn) {
        results[schema_dn] = ad.search_object(schema_dn);
    }
}

/**
 * Helper procedure that helps to configure table columns.
 *
 * Name mapping should be passed as the seconds parameter:
 *   column-number => label
 *
 * Columns that are not in the mapping get empty labels.
 */
void set_horizontal_header_labels_from_map(
    QStandardItemModel *model,
    const QMap<int, QString> &labels_map)
{
    for (int col = 0; col < model->columnCount(); col++) {
        QString label;
        if (labels_map.contains(col)) {
            label = labels_map[col];
        } else {
            label = QString();
        }

        model->setHorizontalHeaderItem(col, new QStandardItem(label));
    }
}

void limit_plain_text_edit(QPlainTextEdit *edit, const QString &attribute) {
    const int range_upper = ad_get_range_upper(attribute);

    if (range_upper > 0) {
        QObject::connect(
            edit, &QPlainTextEdit::textChanged,
            edit, [edit, range_upper]() {
                const QString text = edit->toPlainText();

                if (text.length() > range_upper) {
                    edit->setPlainText(text.left(range_upper));
                }
            });
    }
}
