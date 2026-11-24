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
 * @file object.h
 * @brief Console object model index implementation.
 */

#ifndef CONSOLE_OBJECT_MODEL_H
#define CONSOLE_OBJECT_MODEL_H

#include <QList>
#include <QModelIndex>
#include <QString>

#include "ad_object.h"

////////////////////////////////////////////////////////////////////////////////
// Constants.

extern const QList<QString> RENAMABLE_CLASS_LIST;
extern const QList<QString> NON_MOVABLE_OBJECT_CLASSES;

////////////////////////////////////////////////////////////////////////////////
// Classes.

/**
 * @class ConsoleObjectItem
 * @brief ADMC console object item.
 */
class ConsoleObjectModelIndex : public QModelIndex {
public:
    ConsoleObjectModelIndex(const QModelIndex& index);
    QStringList get_all_classes() const;
    QString get_main_class() const;
    QString get_dn() const;
    QString get_category() const;
    QPersistentModelIndex get_parent() const;
    bool is_parent_valid() const;
    bool is_renameable() const;
    bool has_non_moveable_class() const;
    bool has_renameable_class() const;
    bool is_moveable() const;
    bool is_deleteable() const;
};

#endif  // ifndef CONSOLE_OBJECT_MODEL_H
