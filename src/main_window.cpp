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
#include "members_widget.h"
#include "details_widget.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"
#include "status_bar.h"
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

MainWindow::MainWindow(const bool auto_login)
: QMainWindow()
{
    action_login = new QAction("Login");
    action_exit = new QAction("Exit");

    // TODO: setting width to 1600+ fullscreens the window, no idea why
    resize(1500, 1000);
    setWindowTitle("MainWindow");

    const auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);
    central_widget->setLayout(new QVBoxLayout());
    central_widget->layout()->setContentsMargins(0, 0, 0, 0);
    central_widget->layout()->setSpacing(0);

    const auto status_bar = new StatusBar();
    setStatusBar(status_bar);

    const auto menubar = new QMenuBar(this);
    setMenuBar(menubar);
    
    const auto menubar_file = menubar->addMenu("File");
    menubar_file->addAction(action_login);
    menubar_file->addAction(action_exit);

    const auto menubar_view = menubar->addMenu("View");
    menubar_view->addAction(SETTINGS()->toggle_advanced_view);
    menubar_view->addAction(SETTINGS()->toggle_show_dn_column);

    const auto menubar_preferences = menubar->addMenu("Preferences");
    menubar_preferences->addAction(SETTINGS()->details_on_containers_click);
    menubar_preferences->addAction(SETTINGS()->details_on_contents_click);
    menubar_preferences->addAction(SETTINGS()->confirm_actions);

    const auto splitter = new QSplitter();
    splitter->setOrientation(Qt::Horizontal);
    central_widget->layout()->addWidget(splitter);

    ad_model = new AdModel(this);

    containers_widget = new ContainersWidget(ad_model);
    contents_widget = new ContentsWidget(ad_model);

    MembersWidget *members_widget = MembersWidget::make();
    details_widget = new DetailsWidget(members_widget);

    splitter->addWidget(containers_widget);
    splitter->addWidget(contents_widget);
    splitter->addWidget(details_widget);

    // When window is resized, make containers widget half as big 
    // as others
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 2);
    
    connect(
        AD(), &AdInterface::ad_interface_login_complete,
        this, &MainWindow::on_ad_interface_login_complete);

    // Set root index of contents view to selection of containers view
    connect(
        containers_widget, &ContainersWidget::selected_container_changed,
        contents_widget, &ContentsWidget::on_selected_container_changed);

    connect(
        containers_widget, &EntryWidget::clicked_dn,
        details_widget, &DetailsWidget::on_containers_clicked_dn);

    connect(
        contents_widget, &EntryWidget::clicked_dn,
        details_widget, &DetailsWidget::on_contents_clicked_dn);

    connect(
        action_login, &QAction::triggered,
        this, &MainWindow::on_action_login);
    connect(
        action_exit, &QAction::triggered,
        this, &MainWindow::on_action_exit);

    connect_entry_widget(*containers_widget);
    connect_entry_widget(*contents_widget);
    connect_entry_widget(*members_widget);

    // Disable widgets until logged in
    set_enabled_for_widgets(false);

    if (auto_login) {
        on_action_login();
    }
}

void MainWindow::on_action_login() {
    AD()->ad_interface_login(SEARCH_BASE, HEAD_DN);
}

void MainWindow::set_enabled_for_widgets(bool enabled) {
    QList<QWidget *> widgets = {
        containers_widget,
        contents_widget,
        details_widget
    };

    for (auto e : widgets) {
        e->setEnabled(enabled);
    }
}

void MainWindow::on_ad_interface_login_complete(const QString &base, const QString &head) {
    set_enabled_for_widgets(true);
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

void MainWindow::connect_entry_widget(const EntryWidget &widget) {
    connect(
        &widget, &EntryWidget::request_details,
        details_widget, &DetailsWidget::on_context_menu_details);
    connect(
        &widget, &EntryWidget::request_rename,
        this, &MainWindow::on_request_rename);
    connect(
        &widget, &EntryWidget::request_delete,
        this, &MainWindow::on_request_delete);
    connect(
        &widget, &EntryWidget::request_new_user,
        this, &MainWindow::on_request_new_user);
    connect(
        &widget, &EntryWidget::request_new_computer,
        this, &MainWindow::on_request_new_computer);
    connect(
        &widget, &EntryWidget::request_new_group,
        this, &MainWindow::on_request_new_group);
    connect(
        &widget, &EntryWidget::request_new_ou,
        this, &MainWindow::on_request_new_ou);
    connect(
        &widget, &EntryWidget::request_edit_policy,
        this, &MainWindow::on_request_edit_policy);
}

void MainWindow::on_request_delete(const QString &dn) {
    const QString name = AD()->get_attribute(dn, "name");
    const QString text = QString("Are you sure you want to delete \"%1\"?").arg(name);
    const bool confirmed = confirmation_dialog(text);

    if (confirmed) {
        AD()->delete_entry(dn);
    }    

    contents_widget->setEnabled(false);
}

void MainWindow::on_request_new_entry_generic(const QString &dn, NewEntryType type) {
    create_entry_dialog(type, dn);
}

void MainWindow::on_request_new_user(const QString &dn) {
    on_request_new_entry_generic(dn, NewEntryType::User);
}

void MainWindow::on_request_new_computer(const QString &dn) {
    on_request_new_entry_generic(dn, NewEntryType::Computer);
}

void MainWindow::on_request_new_group(const QString &dn) {
    on_request_new_entry_generic(dn, NewEntryType::Group);
}

void MainWindow::on_request_new_ou(const QString &dn) {
    on_request_new_entry_generic(dn, NewEntryType::OU);
}

void MainWindow::on_request_rename(const QString &dn) {
    // Get new name from input box
    QString dialog_title = "Rename";
    QString input_label = "New name:";
    bool ok;
    QString new_name = QInputDialog::getText(nullptr, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    if (ok && !new_name.isEmpty()) {
        AD()->rename(dn, new_name);
    }
}

void MainWindow::on_request_edit_policy(const QString &dn) {
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
