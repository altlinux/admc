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

LoginDialog::LoginDialog(QWidget *parent)
: QDialog(parent)
{
    resize(600, 600);

    const auto label = new QLabel("Login dialog");

    const auto domain_edit_label = new QLabel("Domain: ");
    domain_edit = new QLineEdit(this);

    const auto site_edit_label = new QLabel("Site: ");
    site_edit = new QLineEdit(this);

    const auto login_button = new QPushButton("Login", this);
    const auto cancel_button = new QPushButton("Cancel", this);

    const auto hosts_list_label = new QLabel("Select host: ");
    hosts_list = new QListWidget(this);
    hosts_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const auto layout = new QGridLayout(this);
    layout->addWidget(label, 0, 0);
    layout->addWidget(domain_edit_label, 1, 0);
    layout->addWidget(domain_edit, 1, 1);
    layout->addWidget(site_edit_label, 1, 2);
    layout->addWidget(site_edit, 1, 3);
    layout->addWidget(hosts_list_label, 3, 0);
    layout->addWidget(hosts_list, 4, 0, 1, 4);
    layout->addWidget(cancel_button, 5, 0, Qt::AlignLeft);
    layout->addWidget(login_button, 5, 4, Qt::AlignRight);

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
    const QString uri = "ldap://" + host;

    // TODO: don't have to pass head_dn
    // but need to get head_dn programmatically via ldap
    // think listing tree with 1 lvl should do it
    AD()->ad_interface_login(uri, "DC=domain,DC=alt");

    if (AD()->is_connected()) {
        done(QDialog::Accepted);
    } else {
        QMessageBox::critical(this, "Error", "Failed to login!");
    }
}

void LoginDialog::on_host_double_clicked(QListWidgetItem *item) {
    const QString host = item->text();

    complete(host);
}

void LoginDialog::on_login_button(bool) {
    QList<QListWidgetItem *> selected_items = hosts_list->selectedItems();

    if (!selected_items.isEmpty()) {
        QListWidgetItem *item = selected_items[0];
        const QString host = item->text();

        complete(host);
    } 
}

void LoginDialog::on_cancel_button(bool) {
    done(QDialog::Rejected);
}

void LoginDialog::open_xd() {
    domain_edit->setText("");
    site_edit->setText("");
    hosts_list->clear();

    load_hosts();

    open();
}

void LoginDialog::on_finished() {
    hosts_list->clear();
}
