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
#include "ad/ad_interface.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QModelIndex>
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
    const BoolSettingSignal *dev_mode_signal = SETTINGS()->get_bool_signal(BoolSetting_DevMode);

    const auto do_it =
    [widget]() {
        const bool dev_mode = SETTINGS()->get_bool(BoolSetting_DevMode);
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
    const bool confirm_actions = SETTINGS()->get_bool(BoolSetting_ConfirmActions);
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
