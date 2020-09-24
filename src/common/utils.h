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

class QAbstractItemModel;
class QAbstractItemView;
class QAbstractProxyModel;
class QString;
class QCheckBox;
class QGridLayout;
class QLabel;
class QByteArray;
class QStandardItem;

QModelIndex convert_to_source(const QModelIndex &index);
QString get_dn_from_index(const QModelIndex &base_row_index, int dn_column);
void set_root_to_head(QAbstractItemView *view);
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

#endif /* UTILS_H */
