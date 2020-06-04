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
#include "containers_widget.h"
#include "contents_widget.h"
#include "details_widget.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry_dialog.h"
#include "actions.h"
#include "status_bar.h"

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

MainWindow::MainWindow()
: QMainWindow()
{  
    //
    // Setup widgets
    //
    actions_init();

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
    
    const auto menubar_new = menubar->addMenu("New");
    menubar_new->addAction(&action_new_user);
    menubar_new->addAction(&action_new_computer);
    menubar_new->addAction(&action_new_group);
    menubar_new->addAction(&action_new_ou);

    const auto menubar_view = menubar->addMenu("View");
    menubar_view->addAction(&action_advanced_view);
    menubar_view->addAction(&action_toggle_dn);

    const auto menubar_preferences = menubar->addMenu("Preferences");
    action_containers_click_attributes = menubar_preferences->addAction("Open attributes on left click in Containers window");
    action_containers_click_attributes->setCheckable(true);
    action_contents_click_attributes = menubar_preferences->addAction("Open attributes on left click in Contents window");
    action_contents_click_attributes->setCheckable(true);

    const auto splitter = new QSplitter();
    splitter->setOrientation(Qt::Horizontal);
    central_widget->layout()->addWidget(splitter);

    ad_model = new AdModel(this);

    containers_widget = new ContainersWidget(ad_model);
    contents_widget = new ContentsWidget(ad_model);
    details_widget = new DetailsWidget();

    splitter->addWidget(containers_widget);
    splitter->addWidget(contents_widget);
    splitter->addWidget(details_widget);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 2);
    
    //
    // Connect actions
    //
    connect(
        &action_details, &QAction::triggered,
        this, &MainWindow::on_action_details);
    connect(
        &action_delete_entry, &QAction::triggered,
        this, &MainWindow::on_action_delete_entry);
    connect(
        &action_new_user, &QAction::triggered,
        this, &MainWindow::on_action_new_user);
    connect(
        &action_new_computer, &QAction::triggered,
        this, &MainWindow::on_action_new_computer);
    connect(
        &action_new_group, &QAction::triggered,
        this, &MainWindow::on_action_new_group);
    connect(
        &action_new_ou, &QAction::triggered,
        this, &MainWindow::on_action_new_ou);

    connect(
        &action_edit_policy, &QAction::triggered,
        this, &MainWindow::on_action_edit_policy);

    // Set root index of contents view to selection of containers view
    connect(
        containers_widget, &ContainersWidget::selected_container_changed,
        contents_widget, &ContentsWidget::on_selected_container_changed);

    connect(
        containers_widget, &EntryWidget::clicked_dn,
        this, &MainWindow::on_containers_clicked_dn);

    connect(
        contents_widget, &EntryWidget::clicked_dn,
        this, &MainWindow::on_contents_clicked_dn);
}

void MainWindow::on_action_details() {
    QString dn = EntryWidget::get_selected_dn();
    details_widget->change_target(dn);
}

void MainWindow::on_containers_clicked_dn(const QString &dn) {
    if (action_containers_click_attributes->isChecked()) {
        details_widget->change_target(dn);
    }
}

void MainWindow::on_contents_clicked_dn(const QString &dn) {
    if (action_containers_click_attributes->isChecked()) {
        details_widget->change_target(dn);
    }
}

void MainWindow::on_action_delete_entry() {
    QString dn = EntryWidget::get_selected_dn();
    delete_entry(dn);
}

void MainWindow::on_action_new_entry_generic(NewEntryType type) {
    QString dn = EntryWidget::get_selected_dn();
    create_entry_dialog(type, dn);
}

void MainWindow::on_action_new_user() {
    on_action_new_entry_generic(NewEntryType::User);
}

void MainWindow::on_action_new_computer() {
    on_action_new_entry_generic(NewEntryType::Computer);
}

void MainWindow::on_action_new_group() {
    on_action_new_entry_generic(NewEntryType::Group);
}

void MainWindow::on_action_new_ou() {
    on_action_new_entry_generic(NewEntryType::OU);
}

void MainWindow::on_action_edit_policy() {
    // Start policy edit process
    const auto process = new QProcess(this);

    const QString program_name = "../gpgui";
    process->setProgram(QDir::currentPath() + program_name);

    const char *uri = "ldap://dc0.domain.alt";

    const QString dn = EntryWidget::get_selected_dn();
    const QString path = get_attribute(dn, "gPCFileSysPath");

    QStringList args;
    args << uri;
    args << path;
    process->setArguments(args);

    printf("on_action_edit_policy\ndn=%s\npath=%s\n", qPrintable(dn), qPrintable(path));
    printf("execute command: %s %s %s\n", qPrintable(program_name), qPrintable(uri), qPrintable(path));

    process->start();
}
