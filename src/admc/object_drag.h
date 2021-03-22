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

#ifndef OBJECT_DRAG_H
#define OBJECT_DRAG_H

class QModelIndex;

enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

bool object_can_drop(const QModelIndex &dropped, const QModelIndex &target);
DropType get_drop_type(const QModelIndex &dropped, const QModelIndex &target);

#endif /* OBJECT_DRAG_H */
