/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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

/**
 * @file query.h
 * @brief Console query model index implementation.
 */

#ifndef CONSOLE_QUERY_MODEL_H
#define CONSOLE_QUERY_MODEL_H

#include <QList>
#include <QModelIndex>
#include <QString>

/**
 * @brief Console query model index implementation.
 *
 * This class is used for query folder operations in the console tree.
 */
class ConsoleQueryModelIndex : public QModelIndex {
public:
    ConsoleQueryModelIndex(const QModelIndex& index);
    QString get_description() const;
    QString get_filter() const;
    QByteArray get_filter_state() const;
    QString get_base() const;
    QString get_name() const;
    bool is_children_scope() const;
    static QStringList get_sibling_name_list(
        const QModelIndex &parent_idx,
        const QModelIndex &index_to_omit = QModelIndex());
};

#endif  // ifndef CONSOLE_QUERY_MODEL_H
