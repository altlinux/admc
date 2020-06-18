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

#include "config.h"

#include "main_window.h"
#include "containers_widget.h"
#include "contents_widget.h"
#include "members_model.h"
#include "members_widget.h"
#include "details_widget.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "status.h"
#include "entry_widget.h"
#include "settings.h"

#include <QApplication>
#include <QString>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeView>
#include <QDir>
#include <QProcess>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>

MainWindow::MainWindow(const bool auto_login)
: QMainWindow()
{
    // TODO: setting width to 1600+ fullscreens the window, no idea why
    resize(1500, 1000);
    setWindowTitle("MainWindow");

    // Menubar
    {
        QMenuBar *menubar = menuBar();
        QMenu *menubar_file = menubar->addMenu("File");
        menubar_file->addAction("Login", [this]() {
            on_action_login();
        });
        menubar_file->addAction("Exit", [this]() {
            on_action_exit();
        });

        QMenu *menubar_view = menubar->addMenu("View");
        menubar_view->addAction(SETTINGS()->toggle_advanced_view);
        menubar_view->addAction(SETTINGS()->toggle_show_dn_column);
        menubar_view->addAction(SETTINGS()->toggle_show_status_log);

        QMenu *menubar_preferences = menubar->addMenu("Preferences");
        menubar_preferences->addAction(SETTINGS()->details_on_containers_click);
        menubar_preferences->addAction(SETTINGS()->details_on_contents_click);
        menubar_preferences->addAction(SETTINGS()->confirm_actions);
    }

    // Widgets
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    auto ad_model = new AdModel(this);
    auto containers_widget = new ContainersWidget(ad_model, this);
    auto contents_widget = new ContentsWidget(ad_model, this);

    auto members_model = new MembersModel(this);
    auto members_widget = new MembersWidget(members_model, this);
    
    auto details_widget = new DetailsWidget(members_widget, this);

    auto status_log = new QTextEdit(this);
    status_log->setReadOnly(true);

    new Status(statusBar(), status_log, this);

    // Layout
    {
        auto horiz_splitter = new QSplitter(Qt::Horizontal);
        horiz_splitter->addWidget(containers_widget);
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

    // Connect signals
    {
        connect(
            containers_widget, &ContainersWidget::selected_container_changed,
            contents_widget, &ContentsWidget::on_selected_container_changed);
        connect(
            containers_widget, &EntryWidget::clicked_dn,
            details_widget, &DetailsWidget::on_containers_clicked_dn);
        connect(
            contents_widget, &EntryWidget::clicked_dn,
            details_widget, &DetailsWidget::on_contents_clicked_dn);
        
        connect_entry_widget(containers_widget, details_widget);
        connect_entry_widget(contents_widget, details_widget);
        connect_entry_widget(members_widget, details_widget);
    }

    if (auto_login) {
        on_action_login();
    }
}

void MainWindow::on_action_login() {
    AD()->ad_interface_login(SEARCH_BASE, HEAD_DN);
}

void MainWindow::on_action_exit() {
    const QString text = QString("Are you sure you want to exit?");
    const bool confirmed = confirmation_dialog(text);

    if (confirmed) {
        QApplication::quit();
    }   
}

bool MainWindow::confirmation_dialog(const QString &text) {
    const bool confirm_actions = SETTINGS()->confirm_actions->isChecked();
    if (!confirm_actions) {
        return true;
    }

    const QString title = "ADMC";
    const QMessageBox::StandardButton reply = QMessageBox::question(this, title, text, QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::connect_entry_widget(EntryWidget *widget, DetailsWidget *details_widget) {
    connect(
        widget, &EntryWidget::context_menu_details,
        details_widget, &DetailsWidget::on_context_menu_details);
    connect(
        widget, &EntryWidget::context_menu_rename,
        this, &MainWindow::on_context_menu_rename);
    connect(
        widget, &EntryWidget::context_menu_delete,
        this, &MainWindow::on_context_menu_delete);
    connect(
        widget, &EntryWidget::context_menu_new_user,
        this, &MainWindow::on_context_menu_new_user);
    connect(
        widget, &EntryWidget::context_menu_new_computer,
        this, &MainWindow::on_context_menu_new_computer);
    connect(
        widget, &EntryWidget::context_menu_new_group,
        this, &MainWindow::on_context_menu_new_group);
    connect(
        widget, &EntryWidget::context_menu_new_ou,
        this, &MainWindow::on_context_menu_new_ou);
    connect(
        widget, &EntryWidget::context_menu_edit_policy,
        this, &MainWindow::on_context_menu_edit_policy);
}

void MainWindow::on_context_menu_delete(const QString &dn) {
    const QString name = AD()->get_attribute(dn, "name");
    const QString text = QString("Are you sure you want to delete \"%1\"?").arg(name);
    const bool confirmed = confirmation_dialog(text);

    if (confirmed) {
        AD()->delete_entry(dn);
    }    
}

void MainWindow::new_entry_dialog(const QString &parent_dn, NewEntryType type) {
    QString type_string = new_entry_type_to_string[type];
    QString dialog_title = "New " + type_string;
    QString input_label = type_string + " name";

    bool ok;
    QString name = QInputDialog::getText(nullptr, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created entry?

    // Create user once dialog is complete
    if (ok && !name.isEmpty()) {
        // Attempt to create user in AD

        const QMap<NewEntryType, QString> new_entry_type_to_suffix = {
            {NewEntryType::User, "CN"},
            {NewEntryType::Computer, "CN"},
            {NewEntryType::OU, "OU"},
            {NewEntryType::Group, "CN"},
        };
        QString suffix = new_entry_type_to_suffix[type];

        const QString dn = suffix + "=" + name + "," + parent_dn;

        AD()->create_entry(name, dn, type);
    }
}

void MainWindow::on_context_menu_new_user(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::User);
}

void MainWindow::on_context_menu_new_computer(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::Computer);
}

void MainWindow::on_context_menu_new_group(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::Group);
}

void MainWindow::on_context_menu_new_ou(const QString &dn) {
    new_entry_dialog(dn, NewEntryType::OU);
}

void MainWindow::on_context_menu_rename(const QString &dn) {
    // Get new name from input box
    QString dialog_title = "Rename";
    QString input_label = "New name:";
    bool ok;
    QString new_name = QInputDialog::getText(nullptr, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    if (ok && !new_name.isEmpty()) {
        AD()->rename(dn, new_name);
    }
}

void MainWindow::on_context_menu_edit_policy(const QString &dn) {
    // Start policy edit process
    const auto process = new QProcess(this);

    const QString program_name = "../gpgui";
    process->setProgram(QDir::currentPath() + program_name);

    const char *uri = "ldap://dc0.domain.alt";

    const QString path = AD()->get_attribute(dn, "gPCFileSysPath");

    QStringList args;
    args << uri;
    args << path;
    process->setArguments(args);

    printf("on_action_edit_policy\ndn=%s\npath=%s\n", qPrintable(dn), qPrintable(path));
    printf("execute command: %s %s %s\n", qPrintable(program_name), qPrintable(uri), qPrintable(path));

    process->start();
}
