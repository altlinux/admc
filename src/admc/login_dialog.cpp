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
#include "utils.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QGridLayout>
#include <QApplication>
#include <QCheckBox>

LoginDialog::LoginDialog(QWidget *parent)
: QDialog(parent)
{
    setWindowTitle(tr("Login"));
    setAttribute(Qt::WA_DeleteOnClose);

    const auto domain_edit_label = new QLabel(tr("Domain: "), this);
    domain_edit = new QLineEdit(this);

    const auto site_edit_label = new QLabel(tr("Site: "), this);
    site_edit = new QLineEdit(this);

    const auto login_button = new QPushButton(tr("Login"), this);
    login_button->setAutoDefault(false);

    autologin_check = new QCheckBox(tr("Login using saved session at startup"));

    const auto layout = new QGridLayout(this);
    append_to_grid_layout_with_label(layout, domain_edit_label, domain_edit);
    append_to_grid_layout_with_label(layout, site_edit_label, site_edit);
    layout->addWidget(autologin_check, layout->rowCount(), 0, 1, layout->columnCount());
    layout->addWidget(login_button, layout->rowCount(), 0, 1, layout->columnCount());

    // Load session values from settings
    const QString domain = SETTINGS()->get_variant(VariantSetting_Domain).toString();
    const QString site = SETTINGS()->get_variant(VariantSetting_Site).toString();
    domain_edit->setText(domain);
    site_edit->setText(site);

    connect(
        login_button, &QAbstractButton::clicked,
        this, &LoginDialog::on_login_button);
    connect(
        this, &QDialog::rejected,
        this, &LoginDialog::on_rejected);
}

void LoginDialog::on_login_button() {
    const QString domain = domain_edit->text();
    const QString site = site_edit->text();

    const bool login_success = AD()->login(domain, site);

    if (login_success) {
        SETTINGS()->set_variant(VariantSetting_Domain, domain);
        SETTINGS()->set_variant(VariantSetting_Site, site);

        const bool autologin_checked = checkbox_is_checked(autologin_check);
        SETTINGS()->set_bool(BoolSetting_AutoLogin, autologin_checked);

        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to login!"));
    }
}

void LoginDialog::on_rejected() {
    QApplication::closeAllWindows();
    QApplication::quit();
}
