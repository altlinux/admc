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
#include <QIcon>
#include <QMap>

class QAbstractItemModel;
class QAbstractItemView;
class QAbstractProxyModel;
class QString;
class QCheckBox;
class QGridLayout;
class QLineEdit;
class AccountOptionEdit;

QModelIndex convert_to_source(const QModelIndex &index);
QString get_dn_from_index(const QModelIndex &base_row_index, int dn_column);
QIcon get_object_icon(const QString &dn);
void set_root_to_head(QAbstractItemView *view);
void setup_model_chain(QAbstractItemView *view, QAbstractItemModel *source_model, QList<QAbstractProxyModel *> proxies);
bool checkbox_is_checked(const QCheckBox *checkbox);
void append_to_grid_layout_with_label(QGridLayout *layout, const QString &label_text, QWidget *widget);
void autofill_full_name(QLineEdit *full_name_edit, QLineEdit *first_name_edit, QLineEdit *last_name_edit);

#endif /* UTILS_H */
