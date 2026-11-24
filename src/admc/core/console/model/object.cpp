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

////////////////////////////////////////////////////////////////////////////////
// Constants.

const QList<QString> RENAMABLE_CLASS_LIST = {
    CLASS_USER,
    CLASS_GROUP,
    CLASS_OU,
    CLASS_SITE,
    CLASS_PSO
};

const QList<QString> NON_MOVABLE_OBJECT_CLASSES = {
    CLASS_PSO,
    CLASS_SITE,
    CLASS_SUBNET,
    CLASS_SITE_LINK,
    CLASS_SITE_LINK_BRIDGE,
    CLASS_SITES_CONTAINER,
    CLASS_INTER_SITE_TRANSPORT,
    CLASS_INTER_SITE_TRANSPORT_CONTAINER,
    CLASS_SUBNET_CONTAINER
};

////////////////////////////////////////////////////////////////////////////////

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

bool ConsoleObjectModelIndex::has_non_moveable_class() const {
    return NON_MOVABLE_OBJECT_CLASSES.contains(get_main_class());
}

bool ConsoleObjectModelIndex::has_renameable_class() const {
    return RENAMABLE_CLASS_LIST.contains(get_main_class());
}

/**
 * @brief Check if an object is moveable.
 *
 * An object is considered moveable if it does * not have "CannotMove" parameter
 * set to "true" and if it does not belong to a * non-moveable class.
 *
 * @return True if the object is moveable, false otherwise.
 */
bool ConsoleObjectModelIndex::is_moveable() const {
    return ((data(ObjectRole_CannotMove).toBool() == false) &&
            (! has_non_moveable_class()));
}

/**
 * Get unique classes for model index list.
 *
 * @param indices A list of Qt model indices.
 * @return A list of unique classes.
 */
QList<QString> ConsoleObjectModelIndex::get_unique_classes(
    const QList<QModelIndex> &indices)
{
    QSet<QString> set;
    for (const QModelIndex &index : indices) {
        const ConsoleObjectModelIndex item(index);
        set.insert(item.get_main_class());
    }
    return QList<QString>(set.begin(), set.end());
}
