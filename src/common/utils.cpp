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
#include <QLabel>
#include <QGridLayout>
#include <QStandardItem>
#include <QMenu>
#include <QTreeView>
#include <QHeaderView>
#include <QtGlobal>
#include <QStandardItemModel>

// Index can be an index of any column in target row and of any proxy in the proxy chain
QString get_dn_from_index(const QModelIndex &index, int dn_column) {
    if (!index.isValid()) {
        return QString();
    }

    // Convert to source index
    const QModelIndex source_index =
    [index]() {
        const QAbstractItemModel *current_model = index.model();
        QModelIndex current_index = index;
        while (true) {
            const QSortFilterProxyModel *current_model_as_proxy = qobject_cast<const QSortFilterProxyModel *>(current_model);

            if (current_model_as_proxy != nullptr) {
                current_index = current_model_as_proxy->mapToSource(current_index);

                current_model = current_model_as_proxy->sourceModel();
            } else {
                return current_index;
            }
        }
    }();
    
    const QModelIndex dn_index = source_index.siblingAtColumn(dn_column);
    const QString dn = dn_index.data().toString();

    return dn;
}

QString get_dn_from_pos(const QPoint &pos, const QAbstractItemView *view, int dn_column) {
    const QModelIndex base_index = view->indexAt(pos);
    const QString dn = get_dn_from_index(base_index, dn_column);

    return dn;
}

bool checkbox_is_checked(const QCheckBox *checkbox) {
    return (checkbox->checkState() == Qt::Checked);
}

void checkbox_set_checked(QCheckBox *checkbox, bool checked) {
    const Qt::CheckState check_state =
    [checked]() {
        if (checked) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }();
    
    checkbox->setCheckState(check_state);
}

bool check_item_is_checked(const QStandardItem *item) {
    return (item->checkState() == Qt::Checked);
}

void check_item_set_checked(QStandardItem *item, bool checked) {
    const Qt::CheckState check_state =
    [checked]() {
        if (checked) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }();
    
    item->setCheckState(check_state);
}

void append_to_grid_layout_with_label(QGridLayout *layout,QLabel *label, QWidget *widget) {
    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(widget, row, 1);
}

// If changed, return text with asterisk at the end
// If not changed, return text without asterisk
QString set_changed_marker(const QString &text, bool changed) {
    const int asterisk_index = text.indexOf("*");
    const bool has_asterisk = (asterisk_index != -1);

    QString new_text = text;
    if (changed && !has_asterisk) {
        new_text = text + "*";
    } else if (!changed && has_asterisk) {
        new_text = text;
        new_text.remove(asterisk_index, 1);
    }

    return new_text;
}

QList<QStandardItem *> make_item_row(const int count) {
    QList<QStandardItem *> row;

    for (int i = 0; i < count; i++) {
        const auto item = new QStandardItem();
        row.push_back(item);
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

QList<QString> byte_arrays_to_strings(const QList<QByteArray> &byte_arrays) {
    QList<QString> strings;

    for (const auto byte_array : byte_arrays) {
        const QString string = QString::fromUtf8(byte_array);
        strings.append(string);
    }

    return strings;
}

// "CN=foo,CN=bar,DC=domain,DC=com"
// =>
// "foo"
QString dn_get_rdn(const QString &dn) {
    int equals_i = dn.indexOf('=') + 1;
    int comma_i = dn.indexOf(',');
    int segment_length = comma_i - equals_i;

    QString name = dn.mid(equals_i, segment_length);

    return name;
}

// "CN=foo,CN=bar,CN=xd,DC=domain,DC=com"
// =>
// "domain.com/xd/bar"
// NOTE: direction is reversed to match how it looks in the tree
QString dn_get_parent(const QString &dn) {
    const int comma_i = dn.indexOf(',');
    const QString parent_dn = dn.mid(comma_i + 1);

    QString parent;

    const QList<QString> parent_dn_split = parent_dn.split(",");
    for (int i = 0; i < parent_dn_split.size(); i++) {
        const QString raw_part = parent_dn_split[i];
        const int equals_i = raw_part.indexOf('=');
        const QString part = raw_part.mid(equals_i + 1);
        
        const QString separator =
        [parent_dn_split, i]() {
            if (i == parent_dn_split.size() - 1) {
                return ".";
            } else if (i > 0) {
                return "/";
            }
            return "";
        }();

        parent = part + separator + parent;
    }

    return parent;
}
