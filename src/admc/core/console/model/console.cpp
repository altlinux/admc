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
 * @file consle.cpp
 * @brief Console model index implementation.
 */

#include "core/console/console.h"
#include "core/console/model/console.h"

ConsoleModelIndex::ConsoleModelIndex(const QModelIndex& index)
    : QModelIndex(index) {
    // Do nothing.
}

QString ConsoleModelIndex::get_name() const {
    return data(Qt::DisplayRole).toString();
}

int ConsoleModelIndex::get_type() const {
    return data(ConsoleRole_Type).toInt();
}

int ConsoleModelIndex::get_sort_index() const {
    return data(ConsoleRole_SortIndex).toInt();
}

bool ConsoleModelIndex::was_fetched() const {
    return data(ConsoleRole_WasFetched).toBool();
}

bool ConsoleModelIndex::is_scope() const {
    return data(ConsoleRole_IsScope).toBool();
}

bool ConsoleModelIndex::is_hidden() const {
    return data(ConsoleRole_IsHidden).toBool();
}

