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

#include "main_window_connection_error.h"
#include "ui_main_window_connection_error.h"

#include "adldap.h"
#include "connection_options_dialog.h"
#include "globals.h"
#include "main_window.h"
#include "settings.h"
#include "utils.h"

MainWindowConnectionError::MainWindowConnectionError()
: QMainWindow() {
    ui = new Ui::MainWindowConnectionError();
    ui->setupUi(this);

    center_widget(this);

    connect(
        ui->retry_button, &QAbstractButton::clicked,
        this, &MainWindowConnectionError::on_retry_button);
    connect(
        ui->quit_button, &QAbstractButton::clicked,
        this, &MainWindowConnectionError::close);
    connect(
        ui->options_button, &QAbstractButton::clicked,
        this, &MainWindowConnectionError::open_connection_options);
}

MainWindowConnectionError::~MainWindowConnectionError() {
    delete ui;
}

void MainWindowConnectionError::on_retry_button() {
    AdInterface ad;

    if (ad_connected(ad, this)) {
        load_g_adconfig(ad);

        MainWindow *real_main_window = new MainWindow(ad, this);
        real_main_window->show();

        QMainWindow::hide();
    }
}

void MainWindowConnectionError::open_connection_options() {
    auto dialog = new ConnectionOptionsDialog(this);
    dialog->open();
}
