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

QString get_dn_from_index(const QModelIndex &index, int dn_column) {
    if (!index.isValid()) {
        return QString();
    }

    const QModelIndex dn_index = index.siblingAtColumn(dn_column);
    const QString dn = dn_index.data().toString();

    return dn;
}

QString get_dn_from_pos(const QPoint &pos, const QAbstractItemView *view, int dn_column) {
    const QModelIndex base_index = view->indexAt(pos);
    const QString dn = get_dn_from_index(base_index, dn_column);

    return dn;
}

QList<QStandardItem *> make_item_row(const int count) {
    QList<QStandardItem *> row;

    for (int i = 0; i < count; i++) {
        const auto item = new QStandardItem();
        row.append(item);
    }

    return row;
}

int bit_set(int bitmask, int bit, bool set) {
    if (set) {
        return bitmask | bit;
    } else {
        return bitmask & ~bit;
    }
}

bool bit_is_set(int bitmask, int bit) {
    return ((bitmask & bit) != 0);
}

void exec_menu_from_view(QMenu *menu, const QAbstractItemView *view, const QPoint &pos) {
    const QPoint global_pos = view->mapToGlobal(pos);
    menu->exec(global_pos);
}

void setup_column_toggle_menu(const QTreeView *view, const QStandardItemModel *model, const QList<int> &initially_visible_columns) {
    Q_CHECK_PTR(view);
    Q_CHECK_PTR(model);
    Q_CHECK_PTR(view->header());

    QHeaderView *header = view->header();
    header->setContextMenuPolicy(Qt::CustomContextMenu);

    // Hide all columns except the ones that are supposed to be
    // visible
    for (int i = 0; i < header->count(); i++) {
        const bool hidden = !initially_visible_columns.contains(i);
        header->setSectionHidden(i, hidden);
    }

    QObject::connect(
        header, &QHeaderView::customContextMenuRequested,
        [view, header, model](const QPoint pos) {
            QMenu menu;
            for (int i = 0; i < header->count(); i++) {
                const auto header_item = model->horizontalHeaderItem(i);
                const QString section_name = header_item->text();

                QAction *action = menu.addAction(section_name);
                action->setCheckable(true);
                const bool currently_hidden = header->isSectionHidden(i);
                action->setChecked(!currently_hidden);

                QObject::connect(action, &QAction::triggered,
                    [header, i, action]() {
                        const bool was_hidden = header->isSectionHidden(i);
                        const bool hidden = !was_hidden;

                        header->setSectionHidden(i, hidden);
                    });
            }
            exec_menu_from_view(&menu, header, pos);
        });
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

const char *cstr(const QString &qstr) {
    static QList<QByteArray> buffer;

    const QByteArray bytes = qstr.toUtf8();
    buffer.append(bytes);

    // Limit buffer to 100 strings
    if (buffer.size() > 100) {
        buffer.removeAt(0);
    }

    // NOTE: return data of bytes in buffer NOT the temp local bytes
    return buffer.last().constData();
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
