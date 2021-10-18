/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

class ObjectImpl;
class ConsoleFilterDialog;
class QLabel;

namespace Ui {
    class MainWindow;
}

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    Ui::MainWindow *ui;

    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:
    ObjectImpl *object_impl;
    ConsoleFilterDialog *filter_dialog;
    QLabel *client_user_label;
    
    void connect_to_server();
    void refresh_object_tree();
    void on_show_non_containers(bool checked);
    void on_dev_mode(bool checked);
    void on_advanced_features(bool checked);
    void on_filter_dialog_accepted();
    void on_log_searches_changed();
    void on_show_client_user();
    void load_connection_options();
};

#endif /* MAIN_WINDOW_H */
