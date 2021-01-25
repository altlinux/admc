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

#ifndef OBJECT_MENU_H
#define OBJECT_MENU_H

/**
 * Adds object-related actions to a menu. 
 */

class QMenu;
class QWidget;
class QAbstractItemView;

void add_object_actions_to_menu(QMenu *menu, QAbstractItemView *view, QWidget *parent);

#endif /* OBJECT_MENU_H */
