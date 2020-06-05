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

// Convenience function to get AdInterface member of the MainWindow instance
AdInterface *AD();

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(const bool auto_login);

    static QAction *action_advanced_view;
    static QAction *action_toggle_dn;
    static QAction *action_details;
    static QAction *action_delete_entry;
    static QAction *action_new_user;
    static QAction *action_new_computer;
    static QAction *action_new_group;
    static QAction *action_new_ou;
    static QAction *action_edit_policy;

    AdInterface *ad_interface = nullptr;
    static MainWindow *instance;

private slots:
    void on_action_details();
    void on_action_delete_entry();
    void on_action_new_user();
    void on_action_new_computer();
    void on_action_new_group();
    void on_action_new_ou();
    void on_containers_clicked_dn(const QString &dn);
    void on_contents_clicked_dn(const QString &dn);
    void on_action_edit_policy();
    void on_action_login();
    void on_action_exit();
    void on_ad_interface_login_complete(const QString &base, const QString &head);

private:
    QString get_selected_dn() const;
    void on_action_new_entry_generic(NewEntryType type);
    void set_enabled_for_ad_actions(bool enabled);

    AdModel *ad_model = nullptr;
    ContainersWidget *containers_widget = nullptr;
    ContentsWidget *contents_widget = nullptr;
    DetailsWidget *details_widget = nullptr;

    QAction *action_containers_click_attributes = nullptr;
    QAction *action_contents_click_attributes = nullptr;
    QAction *action_login = nullptr;
    QAction *action_exit = nullptr;
};

#endif /* MAIN_WINDOW_H */
