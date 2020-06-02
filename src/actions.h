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

#ifndef ACTIONS_H
#define ACTIONS_H

#include <QAction>

extern QAction action_advanced_view;
extern QAction action_toggle_dn;
extern QAction action_details;
extern QAction action_delete_entry;
extern QAction action_new_user;
extern QAction action_new_computer;
extern QAction action_new_group;
extern QAction action_new_ou;
extern QAction action_edit_policy;

void actions_init();

#endif /* ACTIONS_H */
