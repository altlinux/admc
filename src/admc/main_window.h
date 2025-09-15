/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QLabel *login_label;
    AuthDialogBase *auth_dialog;
    bool inited = false;
    Krb5Client *krb5_client = nullptr;

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
    void show_changelog_on_update();
    void setup_status_bar(const AdInterface &ad);
    void init_on_connect(AdInterface &ad);
    void setup_authentication_dialog();
    void on_change_user();
    void on_logout();
    void disable_actions_on_logout(bool disable);
    void init_krb5_client();
    void resize_status_message();
};

#endif /* MAIN_WINDOW_H */
