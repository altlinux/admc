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

#include "login_dialog.h"
#include "ad_interface.h"
#include "settings.h"
#include "confirmation_dialog.h"
#include "dn_column_proxy.h"
#include "utils.h"
#include "settings.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QListWidget>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QAction>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QCheckBox>

LoginDialog::LoginDialog(QAction *login_action, QWidget *parent)
: QDialog(parent)
{
    resize(500, 300);

    const auto label = new QLabel("Login dialog");

    const auto domain_edit_label = new QLabel("Domain: ");
    domain_edit = new QLineEdit(this);

    const auto site_edit_label = new QLabel("Site: ");
    site_edit = new QLineEdit(this);

    const auto login_button = new QPushButton("Login", this);
    const auto cancel_button = new QPushButton("Cancel", this);

    const auto hosts_list_label = new QLabel("Select host:");
    hosts_list = new QListWidget(this);
    hosts_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

    save_session_checkbox = new QCheckBox("Save this session", this);

    const auto layout = new QGridLayout(this);
    layout->addWidget(label, 0, 0);
    layout->addWidget(domain_edit_label, 1, 0);
    layout->addWidget(domain_edit, 1, 1);
    layout->addWidget(site_edit_label, 1, 3);
    layout->addWidget(site_edit, 1, 4);
    layout->addWidget(hosts_list_label, 3, 0);
    layout->addWidget(hosts_list, 4, 0, 1, 5);
    layout->addWidget(save_session_checkbox, 5, 4, Qt::AlignRight);
    layout->addWidget(cancel_button, 6, 0, Qt::AlignLeft);
    layout->addWidget(login_button, 6, 4, Qt::AlignRight);

    connect(
        domain_edit, &QLineEdit::editingFinished,
        this, &LoginDialog::load_hosts);
    connect(
        site_edit, &QLineEdit::editingFinished,
        this, &LoginDialog::load_hosts);
    connect(
        hosts_list, &QListWidget::itemDoubleClicked,
        this, &LoginDialog::on_host_double_clicked);
    connect(
        login_button, &QAbstractButton::clicked,
        this, &LoginDialog::on_login_button);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &LoginDialog::on_cancel_button);
    connect(
        this, &QDialog::finished,
        this, &LoginDialog::on_finished);
    connect(
        login_action, &QAction::triggered,
        this, &LoginDialog::show);
}

void LoginDialog::show() {
    // Load session values from settings
    const QString domain = SETTINGS()->get_string(SettingString_Domain);
    const QString site = SETTINGS()->get_string(SettingString_Site);
    const QString host = SETTINGS()->get_string(SettingString_Host);
    
    domain_edit->setText(domain);
    site_edit->setText(site);

    load_hosts();

    // Select saved session host in hosts list
    QList<QListWidgetItem *> found_hosts = hosts_list->findItems(host, Qt::MatchExactly);
    if (!found_hosts.isEmpty()) {
        QListWidgetItem *host_item = found_hosts[0];
        hosts_list->setCurrentItem(host_item);
    }

    QDialog::open();

    if (found_hosts.isEmpty() && host != "") {
        QMessageBox::warning(this, "Warning", "Failed to find saved session's host");
    }
}

void LoginDialog::on_host_double_clicked(QListWidgetItem *item) {
    const QString host = item->text();

    complete(host);
}

void LoginDialog::on_login_button(bool) {
    // NOTE: listwidget has to have focus to properly return current item...
    hosts_list->setFocus();
    QListWidgetItem *current_item = hosts_list->currentItem();

    if (current_item == nullptr) {
        QMessageBox::warning(this, "Error", "Need to select a host to login.");
    } else {
        const QString host = current_item->text();

        complete(host);
    } 
}

void LoginDialog::on_cancel_button(bool) {
    done(QDialog::Rejected);
}

void LoginDialog::on_finished() {
    hosts_list->clear();
}

void LoginDialog::load_hosts() {
    const QString domain = domain_edit->text();
    const QString site = site_edit->text();

    QList<QString> hosts = AdInterface::get_domain_hosts(domain, site);

    hosts_list->clear();
    for (auto h : hosts) {
        new QListWidgetItem(h, hosts_list);
    }
}

void LoginDialog::complete(const QString &host) {
    const QString domain = domain_edit->text();

    AD()->login(host, domain);

    if (AD()->is_connected()) {
        const bool save_session = save_session_checkbox->isChecked();
        if (save_session) {
            const QString site = site_edit->text();
            SETTINGS()->set_string(SettingString_Domain, domain);
            SETTINGS()->set_string(SettingString_Site, site);
            SETTINGS()->set_string(SettingString_Host, host);
        }

        done(QDialog::Accepted);
    } else {
        QMessageBox::critical(this, "Error", "Failed to login!");
    }
}
