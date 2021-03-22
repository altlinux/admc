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

#include "main_window.h"
#include "menubar.h"
#include "status.h"
#include "settings.h"
#include "ad/ad_interface.h"
#include "ad/ad_config.h"
#include "console.h"
#include "console_widget/console_widget.h"
#include "toggle_widgets_dialog.h"

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QAction>
#include <QDesktopWidget>

MainWindow::MainWindow()
: QMainWindow()
{
    if (SETTINGS()->contains_variant(VariantSetting_MainWindowGeometry)) {
        SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);
    } else {
        // Make window 70% of desktop size for first startup
        resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
    }

    QStatusBar *status_bar = STATUS()->status_bar;
    setStatusBar(status_bar);

    STATUS()->status_bar->showMessage(tr("Ready"));
    SETTINGS()->connect_toggle_widget(STATUS()->status_log, BoolSetting_ShowStatusLog);

    console = new Console();

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(STATUS()->status_log);
    vert_splitter->addWidget(console);
    vert_splitter->setStretchFactor(0, 1);
    vert_splitter->setStretchFactor(1, 3);

    setCentralWidget(vert_splitter);

    connect_action = new QAction(tr("&Connect"));

    auto menubar = new MenuBar(this, console->console_widget);
    setMenuBar(menubar);

    connect(
        connect_action, &QAction::triggered,
        this, &MainWindow::connect_to_server);

    connect_to_server();
}

QAction *MainWindow::get_connect_action() const {
    return connect_action;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);

    QApplication::quit();
}

void MainWindow::connect_to_server() {
    AdInterface ad;
    if (ad_connected(ad)) {
        // TODO: check for load failure
        const QLocale locale = SETTINGS()->get_variant(VariantSetting_Locale).toLocale();
        ADCONFIG()->load(ad, locale);

        STATUS()->display_ad_messages(ad, this);

        console->go_online(ad);
        connect_action->setEnabled(false);
    }
}
