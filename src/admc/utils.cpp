/*
 * ADMC - AD Management Center
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

#include "utils.h"
#include "core/utils.h"

#include "adldap.h"
#include "ui/widget/console/console_widget.h"
#include "core/ad.h"
#include "core/globals.h"
#include "core/search_thread.h"
#include "core/settings.h"
#include "ui/message_box.h"
#include "ui/status.h"

#include <QAbstractItemView>
#include <QCursor>
#include <QGuiApplication>
#include <QHash>
#include <QHeaderView>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QPlainTextEdit>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>

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

QList<QString> get_selected_dn_list(ConsoleWidget *console,
                                    const int type,
                                    const int dn_role) {
    const QList<QModelIndex> indexes = console->get_selected_items(type);
    return index_list_to_dn_list(indexes, dn_role);
}

QString get_selected_target_dn(ConsoleWidget *console,
                               const int type,
                               const int dn_role) {
    const QList<QString> dn_list = get_selected_dn_list(console, type, dn_role);
    return (! dn_list.isEmpty()) ? dn_list[0] : QString();
}

void center_widget(QWidget *widget) {
    QScreen *primary_screen = QGuiApplication::primaryScreen();

    if (primary_screen != nullptr) {
        widget->move(primary_screen->geometry().center() -
                     widget->frameGeometry().center());
    }
}

bool verify_object_name(const QString &name, QWidget *parent) {
    const bool some_bad_chars = string_contains_bad_chars(name, NAME_BAD_CHARS);
    const bool starts_with_space = name.startsWith(" ");
    const bool ends_with_space = name.endsWith(" ");
    const bool starts_with_question_mark = name.startsWith("?");

    const bool contains_bad_chars =
        (some_bad_chars || starts_with_space
         || ends_with_space || starts_with_question_mark);

    if (contains_bad_chars) {
        const QString error_text = QCoreApplication::translate(
            "utils.cpp",
            "Input field for Name contains one or more of the following"
            " illegal characters: # , + \" \\ < > ; = (leading space)"
            " (trailing space) (leading question mark)");
        message_box_warning(
            parent,
            QCoreApplication::translate("utils.cpp", "Error"),
            error_text);

        return false;
    }

    return true;
}

void search_thread_display_errors(SearchThread *thread, QWidget *parent) {
    if (thread->failed_to_connect()) {
        error_log(
            { QCoreApplication::translate(
                    "object_impl.cpp",
                    "Failed to connect to server while searching for objects.") },
            parent);
    } else if (thread->hit_object_display_limit()) {
        error_log(
            { QCoreApplication::translate(
                    "object_impl.cpp",
                    "Could not load all objects. Increase object display limit in Filter Options or reduce number of objects by applying a filter. Filter Options is accessible from main window's menubar via the \"View\" menu.") },
            parent);
    }
}
