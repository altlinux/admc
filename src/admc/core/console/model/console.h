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
 * @file console.h
 * @brief Console model index implementation.
 */

#ifndef CONSOLE_MODEL_H
#define CONSOLE_MODEL_H

#include <QList>
#include <QModelIndex>
#include <QString>

/**
 * @brief Console model index implementation.
 *
 * This class is used for console operations.
 */
class ConsoleModelIndex : public QModelIndex {
public:
    ConsoleModelIndex() = default;
    explicit ConsoleModelIndex(const QModelIndex& index);
    QString get_name() const;
    int get_type() const;
    int get_sort_index() const;
    bool was_fetched() const;
    bool is_scope() const;
    bool is_hidden() const;
};

#endif  // ifndef CONSOLE_MODEL_H
