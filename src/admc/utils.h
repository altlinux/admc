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
class QPoint;
class QWidget;
class QModelIndex;
class QVariant;
class AdInterface;
template <typename T> class QList;
template <typename K, typename T> class QMap;
template <typename K, typename T> class QHash;

#define debug_print(a, args...) printf("%s(%s:%d) " a,  __func__,__FILE__, __LINE__, ##args)
#define trace(a, args...) debug_print(a "\n", ##args)

QList<QStandardItem *> make_item_row(const int count);

void exec_menu_from_view(QMenu *menu, const QAbstractItemView *view, const QPoint &pos);

// Convenience f-n so that you can pass a mapping of
// column => label
// Columns not in the map get empty labels
void set_horizontal_header_labels_from_map(QStandardItemModel *model, const QMap<int, QString> &labels_map);

void show_only_in_dev_mode(QWidget *widget);

// Prohibits leading zeroes
void set_line_edit_to_numbers_only(QLineEdit *edit);

void enable_widget_on_selection(QWidget *widget, QAbstractItemView *view);

// Provide a mapping of columns to widths as ratio of total
// view width. For example: {{column 1 => 0.5}, {column 2 =>
// 0.25}} would give first column 50% of total width and
// second 25% of total width. Omit columns if you don't care
// about their max width.
void resize_columns(QTreeView *view, const QHash<int, double> widths);

void show_busy_indicator();
void hide_busy_indicator();

bool confirmation_dialog(const QString &text, QWidget *parent);

void set_data_for_row(const QList<QStandardItem *> &row, const QVariant &data, const int role);

// Wrappers over is_connected() that also open an error
// messagebox if failed to connect. You should generally use
// these in GUI code instead of is_connected().
bool ad_connected(const AdInterface &ad);
bool ad_failed(const AdInterface &ad);

#endif /* UTILS_H */
