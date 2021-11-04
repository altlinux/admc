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

#include "main_window_connection_error.h"
#include "ui_main_window_connection_error.h"

#include "adldap.h"
#include "main_window.h"
#include "settings.h"
#include "utils.h"
#include "globals.h"
#include "connection_options_dialog.h"

MainWindowConnectionError::MainWindowConnectionError()
: QMainWindow() {
    ui = new Ui::MainWindowConnectionError();
    ui->setupUi(this);

    center_widget(this);

    auto connection_options_dialog = new ConnectionOptionsDialog(this);

    connect(
        ui->retry_button, &QAbstractButton::clicked,
        this, &MainWindowConnectionError::on_retry_button);
    connect(
        ui->quit_button, &QAbstractButton::clicked,
        this, &MainWindow::close);
    connect(
        ui->options_button, &QAbstractButton::clicked,
        connection_options_dialog, &QDialog::open);
    connect(
        connection_options_dialog, &QDialog::accepted,
        load_connection_options);
}

MainWindowConnectionError::~MainWindowConnectionError() {
    delete ui;
}

void MainWindowConnectionError::on_retry_button() {
    AdInterface ad;

    if (ad_connected(ad)) {
        load_g_adconfig(ad);

        MainWindow *real_main_window = new MainWindow(ad, this);
        real_main_window->show();

        QMainWindow::hide();
    }
}
