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
#include "containers_widget.h"
#include "contents_widget.h"
#include "details_widget.h"
#include "status.h"
#include "settings.h"
#include "confirmation_dialog.h"
#include "policies_widget.h"
#include "login_dialog.h"
#include "ad_interface.h"

#include <QApplication>
#include <QString>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QMessageBox>

MainWindow::MainWindow()
: QMainWindow()
{
    SETTINGS()->restore_geometry(this, VariantSetting_MainWindowGeometry);

    const auto open_login_dialog =
    [this]() {
        const auto login_dialog = new LoginDialog(this);
        login_dialog->open();

        QObject::connect(
            login_dialog, &QDialog::accepted,
            [this] () {
                finish_init();
            });
    };

    const bool auto_login = SETTINGS()->get_bool(BoolSetting_AutoLogin);
    if (auto_login) {
        const QString host = SETTINGS()->get_variant(VariantSetting_Host).toString();
        const QString domain = SETTINGS()->get_variant(VariantSetting_Domain).toString();

        const bool login_success = AD()->login(host, domain);

        if (login_success) {
            finish_init();
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("Failed to login using saved login info"));

            open_login_dialog();
        }
    } else {
        open_login_dialog();
    }
}

void MainWindow::finish_init() {
    auto status_log = new QTextEdit(this);
    status_log->setReadOnly(true);
    QStatusBar *status_bar = statusBar();
    Status::instance()->init(status_bar, status_log);

    auto menubar = new MenuBar(this);
    setMenuBar(menubar);

    // Widgets
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    auto containers_widget = new ContainersWidget(this);
    auto contents_widget = new ContentsWidget(containers_widget, this);
    auto details_widget = DetailsWidget::docked_instance();
    auto policies_widget = new PoliciesWidget();

    // Layout
    const auto containers_policies_splitter = new QSplitter(Qt::Vertical);
    containers_policies_splitter->addWidget(containers_widget);
    containers_policies_splitter->addWidget(policies_widget);
    containers_policies_splitter->setStretchFactor(0, 2);
    containers_policies_splitter->setStretchFactor(1, 1);

    auto horiz_splitter = new QSplitter(Qt::Horizontal);
    horiz_splitter->addWidget(containers_policies_splitter);
    horiz_splitter->addWidget(contents_widget);
    horiz_splitter->addWidget(details_widget);
    horiz_splitter->setStretchFactor(0, 1);
    horiz_splitter->setStretchFactor(1, 2);
    horiz_splitter->setStretchFactor(2, 2);

    auto vert_splitter = new QSplitter(Qt::Vertical);
    vert_splitter->addWidget(status_log);
    vert_splitter->addWidget(horiz_splitter);
    vert_splitter->setStretchFactor(0, 1);
    vert_splitter->setStretchFactor(1, 3);

    auto central_layout = new QVBoxLayout();
    central_layout->addWidget(vert_splitter);
    central_layout->setContentsMargins(0, 0, 0, 0);
    central_layout->setSpacing(0);

    central_widget->setLayout(central_layout);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    SETTINGS()->save_geometry(this, VariantSetting_MainWindowGeometry);
}
