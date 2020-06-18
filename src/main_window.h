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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ad_interface.h"

#include <QMainWindow>

class QAction;
class QString;
class AdModel;
class ContainersWidget;
class ContentsWidget;
class DetailsWidget;
class EntryWidget;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(const bool auto_login);

private slots:
    void on_context_menu_rename(const QString &dn);
    void on_context_menu_delete(const QString &dn);
    void on_context_menu_new_user(const QString &dn);
    void on_context_menu_new_computer(const QString &dn);
    void on_context_menu_new_group(const QString &dn);
    void on_context_menu_new_ou(const QString &dn);
    void on_context_menu_edit_policy(const QString &dn);
    void on_action_login();
    void on_action_exit();

private:
    void new_entry_dialog(const QString &parent_dn, NewEntryType type);
    void set_enabled_for_widgets(bool enabled);
    bool confirmation_dialog(const QString &text);
    void connect_entry_widget(EntryWidget *widget, DetailsWidget *details_widget);
};

#endif /* MAIN_WINDOW_H */
