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
    void on_request_details(const QString &dn);
    void on_request_rename(const QString &dn);
    void on_request_delete(const QString &dn);
    void on_request_new_user(const QString &dn);
    void on_request_new_computer(const QString &dn);
    void on_request_new_group(const QString &dn);
    void on_request_new_ou(const QString &dn);
    void on_request_edit_policy(const QString &dn);
    void on_containers_clicked_dn(const QString &dn);
    void on_contents_clicked_dn(const QString &dn);
    void on_action_login();
    void on_action_exit();
    void on_ad_interface_login_complete(const QString &base, const QString &head);

private:
    QString get_selected_dn() const;
    void on_request_new_entry_generic(const QString &dn, NewEntryType type);
    void set_enabled_for_ad_actions(bool enabled);
    bool confirmation_dialog(const QString &text);
    void connect_entry_widget(const EntryWidget &widget);

    AdModel *ad_model = nullptr;
    ContainersWidget *containers_widget = nullptr;
    ContentsWidget *contents_widget = nullptr;
    DetailsWidget *details_widget = nullptr;

    QAction *action_login = nullptr;
    QAction *action_exit = nullptr;
};

#endif /* MAIN_WINDOW_H */
