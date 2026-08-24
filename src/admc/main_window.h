/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <memory>
#include "krb5client.h"

#include "ui/console/all_policies_folder_impl.h"
#include "console_impls/object_impl/object_impl.h"
#include "ui/console/policy_impl.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_impls/query_folder_impl.h"
#include "console_impls/query_item_impl.h"
#include "core/console_item_type.h"

class AdInterface;
class QLabel;
class ObjectImpl;
class AuthDialogBase;
class DomainInfoImpl;

namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    Ui::MainWindow *ui;

    MainWindow(AdInterface &ad, Krb5Client &krb5_client_arg, QWidget *parent = nullptr);
    ~MainWindow();

    void show_changelog_on_update();
    void open_auth_dialog();

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);

private:
    QLabel *login_label;
    AuthDialogBase *auth_dialog;
    bool inited = false;
    Krb5Client *krb5_client = nullptr;
    QActionGroup *theme_action_group;
    DomainInfoImpl *domain_info_impl = nullptr;
    ObjectImpl *object_impl = nullptr;
    PolicyRootImpl *policy_root_impl = nullptr;
    AllPoliciesFolderImpl *all_policies_folder_impl = nullptr;
    PolicyOUImpl *policy_ou_impl = nullptr;
    PolicyImpl *policy_impl = nullptr;
    QueryItemImpl *query_item_impl = nullptr;
    QueryFolderImpl *query_folder_impl = nullptr;
    bool is_language_changed = false;

    void retranslate_themes_menu();
    void on_log_searches_changed();
    void on_show_login_changed();
    void open_manual();
    void open_connection_options();
    void open_changelog();
    void open_about();
    void edit_fsmo_roles();
    void reload_console_tree();
    void setup_themes();
    void setup_languages();
    void setup_simple_settings();
    void setup_complex_settings(ObjectImpl *obj_impl);
    void init_globals();
    void setup_console_actions();
    void setup_main_window_actions();
    void restore_console_widget_state();
    void restore_main_window_state();
    void setup_status_bar(const AdInterface &ad);
    void init_on_connect(AdInterface &ad);
    void setup_authentication_dialog();
    void on_change_user();
    void on_logout();
    void disable_actions_on_logout(bool disable);
    void init_krb5_client();
    void resize_status_message();

    friend class MainWindowConnectionError;
};

#endif /* MAIN_WINDOW_H */
