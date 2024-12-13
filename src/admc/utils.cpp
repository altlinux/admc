/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "utils.h"

#include "adldap.h"
#include "console_widget/console_widget.h"
#include "globals.h"
#include "settings.h"
#include "status.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QCursor>
#include <QGuiApplication>
#include <QHash>
#include <QHeaderView>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QPlainTextEdit>
#include <QPoint>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>

QMessageBox *message_box_generic(const QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent);
int get_range_upper(const QString &attribute);

QList<QStandardItem *> make_item_row(const int count) {
    QList<QStandardItem *> row;

    for (int i = 0; i < count; i++) {
        const auto item = new QStandardItem();
        row.append(item);
    }

    return row;
}

void set_horizontal_header_labels_from_map(QStandardItemModel *model, const QMap<int, QString> &labels_map) {
    for (int col = 0; col < model->columnCount(); col++) {
        const QString label = [=]() {
            if (labels_map.contains(col)) {
                return labels_map[col];
            } else {
                return QString();
            }
        }();

        model->setHorizontalHeaderItem(col, new QStandardItem(label));
    }
}

void set_line_edit_to_decimal_numbers_only(QLineEdit *edit) {
    edit->setValidator(new QRegExpValidator(QRegExp("[0-9]*"), edit));
}

void enable_widget_on_selection(QWidget *widget, QAbstractItemView *view) {
    auto selection_model = view->selectionModel();

    auto do_it = [widget, selection_model]() {
        const bool has_selection = selection_model->hasSelection();
        widget->setEnabled(has_selection);
    };

    QObject::connect(
        selection_model, &QItemSelectionModel::selectionChanged,
        do_it);
    do_it();
}

void show_busy_indicator() {
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void hide_busy_indicator() {
    QGuiApplication::restoreOverrideCursor();
}

bool confirmation_dialog(const QString &text, QWidget *parent) {
    const bool confirm_actions = settings_get_variant(SETTING_confirm_actions).toBool();
    if (!confirm_actions) {
        return true;
    }

    const QString title = QObject::tr("Confirm action");
    const QMessageBox::StandardButton reply = QMessageBox::question(parent, title, text, QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        return true;
    } else {
        return false;
    }
}

void set_data_for_row(const QList<QStandardItem *> &row, const QVariant &data, const int role) {
    for (QStandardItem *item : row) {
        item->setData(data, role);
    }
}

bool ad_connected_base(const AdInterface &ad, QWidget *parent) {
    if (!ad.is_connected()) {
        ad_error_log(ad, parent);
    }

    return ad.is_connected();
}

bool ad_connected(const AdInterface &ad, QWidget *parent) {
    return ad_connected_base(ad, parent);
}

bool ad_failed(const AdInterface &ad, QWidget *parent) {
    return !ad_connected_base(ad, parent);
}

QString get_classes_filter(const QList<QString> &class_list) {
    QList<QString> class_filters;
    for (const QString &object_class : class_list) {
        const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
        class_filters.append(class_filter);
    }

    const QString out = filter_OR(class_filters);

    return out;
}

QString is_container_filter() {
    const QList<QString> accepted_classes = g_adconfig->get_filter_containers();

    const QString out = get_classes_filter(accepted_classes);

    return out;
}

void limit_edit(QLineEdit *edit, const QString &attribute) {
    const int range_upper = get_range_upper(attribute);

    if (range_upper > 0) {
        edit->setMaxLength(range_upper);
    }
}

void limit_plain_text_edit(QPlainTextEdit *edit, const QString &attribute) {
    const int range_upper = get_range_upper(attribute);

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

QList<QPersistentModelIndex> persistent_index_list(const QList<QModelIndex> &indexes) {
    QList<QPersistentModelIndex> out;

    for (const QModelIndex &index : indexes) {
        out.append(QPersistentModelIndex(index));
    }

    return out;
}

QList<QModelIndex> normal_index_list(const QList<QPersistentModelIndex> &indexes) {
    QList<QModelIndex> out;

    for (const QPersistentModelIndex &index : indexes) {
        out.append(QModelIndex(index));
    }

    return out;
}

// Hide advanced view only" objects if advanced view setting
// is off
QString advanced_features_filter(const QString &filter) {
    const bool advanced_features_OFF = !settings_get_variant(SETTING_advanced_features).toBool();

    if (advanced_features_OFF) {
        const QString advanced_features = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, LDAP_BOOL_TRUE);
        const QString out = filter_AND({filter, advanced_features});

        return out;
    } else {
        return filter;
    }
}

// NOTE: configuration and schema objects are hidden so that
// they don't show up in regular searches. Have to use
// search_object() and manually add them to search results.
void dev_mode_search_results(QHash<QString, AdObject> &results, AdInterface &ad, const QString &base) {
    const bool dev_mode = settings_get_variant(SETTING_feature_dev_mode).toBool();
    if (!dev_mode) {
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

QMessageBox *message_box_generic(const QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent) {
    auto message_box = new QMessageBox(parent);
    message_box->setAttribute(Qt::WA_DeleteOnClose);
    message_box->setStandardButtons(QMessageBox::Ok);
    message_box->setWindowTitle(title);
    message_box->setText(text);
    message_box->setIcon(icon);

    message_box->open();

    return message_box;
}

QMessageBox *message_box_critical(QWidget *parent, const QString &title, const QString &text) {
    return message_box_generic(QMessageBox::Critical, title, text, parent);
}

QMessageBox *message_box_information(QWidget *parent, const QString &title, const QString &text) {
    return message_box_generic(QMessageBox::Information, title, text, parent);
}

QMessageBox *message_box_question(QWidget *parent, const QString &title, const QString &text) {
    return message_box_generic(QMessageBox::Question, title, text, parent);
}

QMessageBox *message_box_warning(QWidget *parent, const QString &title, const QString &text) {
    return message_box_generic(QMessageBox::Warning, title, text, parent);
}

QList<QString> index_list_to_dn_list(const QList<QModelIndex> &index_list, const int dn_role) {
    QList<QString> out;

    for (const QModelIndex &index : index_list) {
        const QString dn = index.data(dn_role).toString();
        out.append(dn);
    }

    return out;
}

QList<QString> get_selected_dn_list(ConsoleWidget *console, const int type, const int dn_role) {
    const QList<QModelIndex> indexes = console->get_selected_items(type);
    const QList<QString> out = index_list_to_dn_list(indexes, dn_role);

    return out;
}

QString get_selected_target_dn(ConsoleWidget *console, const int type, const int dn_role) {
    const QList<QString> dn_list = get_selected_dn_list(console, type, dn_role);

    if (!dn_list.isEmpty()) {
        return dn_list[0];
    } else {
        return QString();
    }
}

void center_widget(QWidget *widget) {
    QScreen *primary_screen = QGuiApplication::primaryScreen();

    if (primary_screen != nullptr) {
        widget->move(primary_screen->geometry().center() - widget->frameGeometry().center());
    }
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

QList<QString> variant_list_to_string_list(const QList<QVariant> &variant_list) {
    QList<QString> out;

    for (const QVariant &variant : variant_list) {
        const QString string = variant.toString();
        out.append(string);
    }

    return out;
}

QList<QVariant> string_list_to_variant_list(const QList<QString> &string_list) {
    QList<QVariant> out;

    for (const QString &string : string_list) {
        const QVariant variant = QVariant(string);
        out.append(variant);
    }

    return out;
}

bool string_contains_bad_chars(const QString &string, const QString &bad_chars) {
    const QRegularExpression regexp = [&]() {
        const QString bad_chars_escaped = QRegularExpression::escape(bad_chars);
        const QString regexp_string = QString("[%1]").arg(bad_chars_escaped);
        const QRegularExpression out = QRegularExpression(regexp_string);

        return out;
    }();

    const bool out = string.contains(regexp);

    return out;
}

bool verify_object_name(const QString &name, QWidget *parent) {
    const bool contains_bad_chars = [&]() {
        const bool some_bad_chars = string_contains_bad_chars(name, NAME_BAD_CHARS);
        const bool starts_with_space = name.startsWith(" ");
        const bool ends_with_space = name.endsWith(" ");
        const bool starts_with_question_mark = name.startsWith("?");

        const bool out = (some_bad_chars || starts_with_space || ends_with_space || starts_with_question_mark);

        return out;
    }();

    if (contains_bad_chars) {
        const QString error_text = QString(QCoreApplication::translate("utils.cpp", "Input field for Name contains one or more of the following illegal characters: # , + \" \\ < > ; = (leading space) (trailing space) (leading question mark)"));
        message_box_warning(parent, QCoreApplication::translate("utils.cpp", "Error"), error_text);

        return false;
    }

    return true;
}

void setup_lineedit_autofill(QLineEdit *src, QLineEdit *dest) {
    QObject::connect(
        src, &QLineEdit::textChanged,
        [src, dest]() {
            const QString src_input = src->text();
            dest->setText(src_input);
        });
}

void setup_full_name_autofill(QLineEdit *first_name_edit, QLineEdit *last_name_edit, QLineEdit *middle_name_edit, QLineEdit *full_name_edit) {
    auto autofill_full_name = [=]() {
        const QString full_name_value = [=]() {
            const QString first_name = first_name_edit->text().trimmed();
            const QString last_name = last_name_edit->text().trimmed();
            const QString middle_name = middle_name_edit->text().trimmed();

            const bool last_name_first = settings_get_variant(SETTING_last_name_before_first_name).toBool();
            if (!first_name.isEmpty() && !last_name.isEmpty()) {
                if (last_name_first) {
                    return last_name + " " + first_name + " " + middle_name;
                } else {
                    return first_name + " " + middle_name + " " + last_name;
                }
            } else if (!first_name.isEmpty()) {
                return first_name + middle_name;
            } else if (!last_name.isEmpty()) {
                return middle_name + last_name;
            } else if (!middle_name.isEmpty()) {
                return middle_name;
            } else {
                return QString();
            }
        }();

        full_name_edit->setText(full_name_value);
    };

    QObject::connect(
        first_name_edit, &QLineEdit::textChanged,
        first_name_edit, autofill_full_name);
    QObject::connect(
        last_name_edit, &QLineEdit::textChanged,
        last_name_edit, autofill_full_name);
    QObject::connect(
        middle_name_edit, &QLineEdit::textChanged,
        middle_name_edit, autofill_full_name);
}

int get_range_upper(const QString &attribute) {
    if (attribute == ATTRIBUTE_UPN_SUFFIXES) {
        // NOTE: schema doesn't define a max length for
        // "upn suffixes", but we do need a limit. Use
        // half of total max length of upn as a good
        // estimate.
        const int upn_suffix_max_length = [&]() {
            const int upn_max_length = g_adconfig->get_attribute_range_upper(ATTRIBUTE_USER_PRINCIPAL_NAME);
            const int out = upn_max_length / 2;

            return out;
        }();

        return upn_suffix_max_length;
    } else {
        const int out = g_adconfig->get_attribute_range_upper(attribute);

        return out;
    }
}

void set_line_edit_to_hex_numbers_only(QLineEdit *edit) {
    edit->setValidator(new QRegExpValidator(QRegExp("[0-9a-f]*"), edit));
}

void set_line_edit_to_time_span_format(QLineEdit *edit) {
    QRegExp time_span_reg_exp("([0-9]{1,4}:[0-2][0-3]:[0-5][0-9]:[0-5][0-9])|^\\(never\\)&|^\\(none\\)&");
    edit->setValidator(new QRegExpValidator(time_span_reg_exp));
}

QString gpo_status_from_int(int status) {
    switch (status) {
    case 0: return QObject::tr("Enabled");
    case 1: return QObject::tr("User configuration disabled");
    case 2: return QObject::tr("Computer configuration disabled");
    case 3: return QObject::tr("Disabled");
    default: return QObject::tr("Undefined GPO status");
    }
}

QString current_dc_dns_host_name(AdInterface &ad)
{
    const AdObject rootDSE = ad.search_object("");
    const QString server_name = rootDSE.get_string(ATTRIBUTE_SERVER_NAME);
    const AdObject server = ad.search_object(server_name);
    const QString out = server.get_string(ATTRIBUTE_DNS_HOST_NAME);

    return out;
}
