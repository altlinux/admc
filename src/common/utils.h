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

#ifndef UTILS_H
#define UTILS_H

#include <QModelIndex>
#include <QList>
#include <QPoint>

class QAbstractItemModel;
class QAbstractItemView;
class QAbstractProxyModel;
class QString;
class QCheckBox;
class QGridLayout;
class QLabel;
class QStandardItem;
class QStandardItemModel;
class QMenu;
class QTreeView;

QString get_dn_from_index(const QModelIndex &index, int dn_column);
QString get_dn_from_pos(const QPoint &pos, const QAbstractItemView *view, int dn_column);
void setup_model_chain(QAbstractItemView *view, QAbstractItemModel *source_model, QList<QAbstractProxyModel *> proxies);
bool checkbox_is_checked(const QCheckBox *checkbox);
void checkbox_set_checked(QCheckBox *checkbox, bool checked);
bool check_item_is_checked(const QStandardItem *check_item);
void check_item_set_checked(QStandardItem *checkbox, bool checked);
void append_to_grid_layout_with_label(QGridLayout *layout, QLabel *label, QWidget *widget);
QString set_changed_marker(const QString &text, bool changed);
QList<QStandardItem *> make_item_row(const int count);

int bit_set(int bitmask, int bit, bool set);
bool bit_is_set(int bitmask, int bit);

void exec_menu_from_view(QMenu *menu, const QAbstractItemView *view, const QPoint &pos);
// NOTE: view must have header items and model before this is called
void setup_column_toggle_menu(const QTreeView *view, const QStandardItemModel *model, const QList<int> &initially_visible_columns);

#endif /* UTILS_H */
