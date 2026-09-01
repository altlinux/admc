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

#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QRegularExpressionValidator>
#include <QStandardItem>
#include <QString>

class AdInterface;
class QPlainTextEdit;

QList<QVariant> string_list_to_variant_list(const QList<QString> &string_list);
QList<QString> variant_list_to_string_list(const QList<QVariant> &variant_list);

QString generate_new_name(const QList<QString> &existing_name_list,
                          const QString &name_base);
bool string_contains_bad_chars(const QString &string, const QString &bad_chars);
char *itoa(int value, char *result, int base);
QList<QStandardItem *> make_item_row(const int count);
void set_data_for_row(const QList<QStandardItem *> &row,
                      const QVariant &data,
                      const int role);
QList<QPersistentModelIndex> persistent_index_list(
    const QList<QModelIndex> &indexes);
QList<QModelIndex> normal_index_list(
    const QList<QPersistentModelIndex> &indexes);
QRegularExpressionValidator* make_decimal_numbers_validator(QObject *parent);

// Filter that accepts only given classes
QString get_classes_filter(const QList<QString> &class_list);

QString advanced_features_filter(const QString &filter);

// Filter that accepts only container classes
QString is_container_filter();

QString gpo_status_from_int(int status);

QList<QString> index_list_to_dn_list(const QList<QModelIndex> &index_list,
                                     const int dn_role);

void dev_mode_search_results(QHash<QString, AdObject> &results,
                             AdInterface &ad,
                             const QString &base);

void set_horizontal_header_labels_from_map(
    QStandardItemModel *model,
    const QMap<int, QString> &labels_map);

void limit_plain_text_edit(QPlainTextEdit *edit, const QString &attribute);
int count_non_empty_containers(const QList<QModelIndex> &list);

#endif
