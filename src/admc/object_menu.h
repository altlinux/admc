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
class QAction;
class QString;
class QModelIndex;
template <typename T> class QList;

// Construct object actions of the menu based on current target(s) in the view. If any more custom actions need to be added to this menu, insert them before the action returned from this f-n.
QAction *add_object_actions_to_menu(QMenu *menu, const QList<QModelIndex> &targets, QWidget *parent);

// The following f-ns are declared in header so that tests
// can access them
void delete_object(const QList<QString> targets, QWidget *parent);
void move(const QList<QString> targets, QWidget *parent);
void add_to_group(const QList<QString> targets, QWidget *parent);
void enable_account(const QList<QString> targets, QWidget *parent);
void disable_account(const QList<QString> targets, QWidget *parent);

void properties(const QString &target, QWidget *parent);
void rename(const QString &target, QWidget *parent);
void create(const QString &target, const QString &object_class, QWidget *parent);
void find(const QString &target, QWidget *parent);
void reset_password(const QString &target, QWidget *parent);

#endif /* OBJECT_MENU_H */
