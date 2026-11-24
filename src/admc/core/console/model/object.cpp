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
 * @file object.cpp
 * @brief Console object model index implementation.
 */

#include "core/console/console.h"
#include "core/console/model/object.h"

ConsoleObjectModelIndex::ConsoleObjectModelIndex(const QModelIndex& index)
    : QModelIndex(index) {
    // Do nothing.
}

QStringList ConsoleObjectModelIndex::get_all_classes() const {
    return data(ObjectRole_ObjectClasses).toStringList();
}

QString ConsoleObjectModelIndex::get_main_class() const {
    return data(ObjectRole_ObjectClasses).toStringList().last();
}

/**
 * Get the distinguished name (DN) of the object.
 * @return The distinguished name of the object.
 */
QString ConsoleObjectModelIndex::get_dn() const {
    return data(ObjectRole_DN).toString();
}

/**
 * Get the object category.
 * @return The object category.
 */
QString ConsoleObjectModelIndex::get_category() const {
    return data(ObjectRole_ObjectCategory).toString();
}

/**
 * Get the object parent.
 * @return The object parent.
 */
QPersistentModelIndex ConsoleObjectModelIndex::get_parent() const {
    return parent();
}

bool ConsoleObjectModelIndex::is_parent_valid() const {
    return parent().isValid();
}

bool ConsoleObjectModelIndex::is_renameable() const {
    return data(ObjectRole_CannotRename).toBool() == false;
}

bool ConsoleObjectModelIndex::is_deleteable() const {
    return data(ObjectRole_CannotDelete).toBool() == false;
}

bool ConsoleObjectModelIndex::is_moveable() const {
    return data(ObjectRole_CannotMove).toBool() == false;
}
