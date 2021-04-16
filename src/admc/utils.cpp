/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "settings.h"
#include "adldap.h"
#include "globals.h"
#include "console_widget/console_widget.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QCheckBox>
#include <QStandardItem>
#include <QMenu>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QGuiApplication>
#include <QCursor>
#include <QList>
#include <QPoint>
#include <QMap>
#include <QHash>
#include <QMessageBox>

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
        const QString label =
        [=]() {
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

    const auto do_it =
    [widget]() {
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

    auto do_it =
    [widget, selection_model]() {
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
    const QMessageBox::StandardButton reply = QMessageBox::question(parent, title, text, QMessageBox::Yes|QMessageBox::No);

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
        const QString title = QObject::tr("Connection error");
        const QString text = QObject::tr("Failed to connect to server.");

        // TODO: would want a valid parent widget for
        // message box but this f-n can be called from
        // places where there isn't one available,
        // console_drag_model for example. Good news is that
        // the messagebox appears to be modal even without a
        // parent.
        QMessageBox::critical(nullptr, title, text);
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
    // TODO: change to custom, good icons, add those icons to installation?
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

    const QString icon_name =
    [object_classes]() -> QString {
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

QList<QPersistentModelIndex> get_persistent_indexes(const QList<QModelIndex> &indexes) {
    QList<QPersistentModelIndex> out;

    for (const QModelIndex &index : indexes) {
        out.append(QPersistentModelIndex(index));
    }

    return out;
}

QModelIndex get_selected_scope_index(ConsoleWidget *console) {
    const QList<QModelIndex> selected_indexes = console->get_selected_items();
    
    if (selected_indexes.size() == 1) {
        const QModelIndex index = selected_indexes[0];
        const QModelIndex scope_index = console->convert_to_scope_index(index);

        return scope_index;
    } else {
        return QModelIndex();
    }
}
