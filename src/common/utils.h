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
class QStandardItem;
class QStandardItemModel;
class QMenu;
class QTreeView;
class QLineEdit;

QString get_dn_from_index(const QModelIndex &index, int dn_column);
QString get_dn_from_pos(const QPoint &pos, const QAbstractItemView *view, int dn_column);
QList<QStandardItem *> make_item_row(const int count);

int bit_set(int bitmask, int bit, bool set);
bool bit_is_set(int bitmask, int bit);

void exec_menu_from_view(QMenu *menu, const QAbstractItemView *view, const QPoint &pos);
// NOTE: view must have header items and model before this is called
void setup_column_toggle_menu(const QTreeView *view, const QStandardItemModel *model, const QList<int> &initially_visible_columns);

// Convenience f-n so that you can pass a mapping of
// column => label
// Columns not in the map get empty labels
void set_horizontal_header_labels_from_map(QStandardItemModel *model, const QMap<int, QString> &labels_map);

void show_only_in_dev_mode(QWidget *widget);

// Prohibits leading zeroes
void set_line_edit_to_numbers_only(QLineEdit *edit);

void enable_widget_on_selection(QWidget *widget, QAbstractItemView *view);

// NOTE: uses a buffer that is capped at 100 strings, so pointers returned from this become invalid after 99 more calls. Only use this to give cstr args to C routines in the same scope. Keep this far away from any recursion.
const char *cstr(const QString &qstr);

// Provide a mapping of columns to widths as ratio of total
// view width. For example: {{column 1 => 0.5}, {column 2 =>
// 0.25}} would give first column 50% of total width and
// second 25% of total width. Omit columns if you don't care
// about their max width.
void resize_columns(QTreeView *view, const QHash<int, double> widths);

#endif /* UTILS_H */
