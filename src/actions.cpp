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

#include "actions.h"

QAction action_advanced_view("Advanced view");
QAction action_toggle_dn("Show DN");
QAction action_attributes("Attributes");
QAction action_delete_entry("Delete");
QAction action_new_user("New User");
QAction action_new_computer("New Computer");
QAction action_new_group("New Group");
QAction action_new_ou("New OU");
QAction action_edit_policy("Edit policy");

void actions_init() {
    action_advanced_view.setCheckable(true);
    action_toggle_dn.setCheckable(true);
}
