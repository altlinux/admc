/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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
#include <QPoint>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>

void message_box_generic(const QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent);

QList<QStandardItem *> make_item_row(const int count) {
    QList<QStandardItem *> row;

    for (int i = 0; i < count; i++) {
        const auto item = new QStandardItem();
        row.append(item);
    }

    return row;
}

void exec_menu_from_view(QMenu *menu, const QAbstractItemView *view, const QPoint &pos) {
    const QPoint global_pos = view->mapToGlobal(pos);
    menu->exec(global_pos);
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

void show_only_in_dev_mode(QWidget *widget) {
    const BoolSettingSignal *dev_mode_signal = g_settings->get_bool_signal(BoolSetting_DevMode);

    const auto do_it = [widget]() {
        const bool dev_mode = g_settings->get_bool(BoolSetting_DevMode);
        widget->setVisible(dev_mode);
    };
    do_it();

    QObject::connect(
        dev_mode_signal, &BoolSettingSignal::changed,
        [do_it]() {
            do_it();
        });
}

void set_line_edit_to_numbers_only(QLineEdit *edit) {
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

void resize_columns(QTreeView *view, const QHash<int, double> widths) {
    for (const int col : widths.keys()) {
        const double width_ratio = widths[col];
        const int width = (int) (view->width() * width_ratio);

        view->setColumnWidth(col, width);
    }
}

void show_busy_indicator() {
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void hide_busy_indicator() {
    QGuiApplication::restoreOverrideCursor();
}

bool confirmation_dialog(const QString &text, QWidget *parent) {
    const bool confirm_actions = g_settings->get_bool(BoolSetting_ConfirmActions);
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

bool ad_connected_base(const AdInterface &ad) {
    if (!ad.is_connected()) {
        ad_error_log(ad, nullptr);
    }

    return ad.is_connected();
}

bool ad_connected(const AdInterface &ad) {
    return ad_connected_base(ad);
}

bool ad_failed(const AdInterface &ad) {
    return !ad_connected_base(ad);
}

QString is_container_filter() {
    const QList<QString> accepted_classes = g_adconfig->get_filter_containers();

    QList<QString> class_filters;
    for (const QString &object_class : accepted_classes) {
        const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
        class_filters.append(class_filter);
    }

    return filter_OR(class_filters);
}

void limit_edit(QLineEdit *edit, const QString &attribute) {
    const int range_upper = g_adconfig->get_attribute_range_upper(attribute);
    if (range_upper > 0) {
        edit->setMaxLength(range_upper);
    }
}

QIcon get_object_icon(const AdObject &object) {
    static const QMap<QString, QString> class_to_icon = {
        {CLASS_DOMAIN, "network-server"},
        {CLASS_CONTAINER, "folder"},
        {CLASS_OU, "folder-documents"},
        {CLASS_GROUP, "application-x-smb-workgroup"},
        {CLASS_PERSON, "avatar-default"},
        {CLASS_COMPUTER, "computer"},
        {CLASS_GP_CONTAINER, "folder-templates"},

        // Some custom icons for one-off objects
        {"builtinDomain", "emblem-system"},
        {"configuration", "emblem-system"},
        {"lostAndFound", "emblem-system"},
    };

    // Iterate over object classes in reverse, starting from most inherited class
    QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    std::reverse(object_classes.begin(), object_classes.end());

    const QString icon_name = [object_classes]() -> QString {
        for (auto object_class : object_classes) {
            if (class_to_icon.contains(object_class)) {
                return class_to_icon[object_class];
            }
        }

        return "dialog-question";
    }();

    const QIcon icon = QIcon::fromTheme(icon_name);

    return icon;
}

QList<QPersistentModelIndex> persistent_index_list(const QList<QModelIndex> &indexes) {
    QList<QPersistentModelIndex> out;

    for (const QModelIndex &index : indexes) {
        out.append(QPersistentModelIndex(index));
    }

    return out;
}

// Hide advanced view only" objects if advanced view setting
// is off
void advanced_features_filter(QString &filter) {
    const bool advanced_features_OFF = !g_settings->get_bool(BoolSetting_AdvancedFeatures);
    if (advanced_features_OFF) {
        const QString advanced_features = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_SHOW_IN_ADVANCED_VIEW_ONLY, "true");
        filter = filter_OR({filter, advanced_features});
    }
}

// OR filter with some dev mode object classes, so that they
// show up no matter what when dev mode is on
void dev_mode_filter(QString &filter) {
    const bool dev_mode = g_settings->get_bool(BoolSetting_DevMode);
    if (!dev_mode) {
        return;
    }

    const QList<QString> schema_classes = {
        "classSchema",
        "attributeSchema",
        "displaySpecifier",
    };

    QList<QString> class_filters;
    for (const QString &object_class : schema_classes) {
        const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
        class_filters.append(class_filter);
    }

    filter = filter_OR({filter, filter_OR(class_filters)});
}

// NOTE: configuration and schema objects are hidden so that
// they don't show up in regular searches. Have to use
// search_object() and manually add them to search results.
void dev_mode_search_results(QHash<QString, AdObject> &results, AdInterface &ad, const QString &base) {
    const bool dev_mode = g_settings->get_bool(BoolSetting_DevMode);
    if (!dev_mode) {
        return;
    }

    const QString domain_head = g_adconfig->domain_head();
    const QString configuration_dn = g_adconfig->configuration_dn();
    const QString schema_dn = g_adconfig->schema_dn();

    if (base == domain_head) {
        results[configuration_dn] = ad.search_object(configuration_dn);
    } else if (base == configuration_dn) {
        results[schema_dn] = ad.search_object(schema_dn);
    }
}

void message_box_generic(const QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent) {
    auto message_box = new QMessageBox(parent);
    message_box->setAttribute(Qt::WA_DeleteOnClose);
    message_box->setStandardButtons(QMessageBox::Ok);
    message_box->setWindowTitle(title);
    message_box->setText(title);
    message_box->setIcon(icon);

    message_box->open();
}

void message_box_critical(QWidget *parent, const QString &title, const QString &text) {
    message_box_generic(QMessageBox::Critical, title, text, parent);
}

void message_box_information(QWidget *parent, const QString &title, const QString &text) {
    message_box_generic(QMessageBox::Information, title, text, parent);
}

void message_box_question(QWidget *parent, const QString &title, const QString &text) {
    message_box_generic(QMessageBox::Question, title, text, parent);
}

void message_box_warning(QWidget *parent, const QString &title, const QString &text) {
    message_box_generic(QMessageBox::Warning, title, text, parent);
}
