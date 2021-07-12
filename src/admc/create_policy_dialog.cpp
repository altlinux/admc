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

#include "create_policy_dialog.h"

#include "adldap.h"
#include "console_types/console_policy.h"
#include "console_widget/console_widget.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

CreatePolicyDialog::CreatePolicyDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    console = console_arg;

    setMinimumWidth(400);

    const auto title = QString(tr("Create GPO"));
    setWindowTitle(title);

    name_edit = new QLineEdit();
    name_edit->setText("New Group Policy Object");

    const auto edits_layout = new QFormLayout();
    edits_layout->addRow(tr("Name"), name_edit);

    auto button_box = new QDialogButtonBox();
    button_box->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    button_box->addButton(QDialogButtonBox::Cancel);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edits_layout);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
}

void CreatePolicyDialog::open() {
    name_edit->setText("");

    QDialog::open();
}

void CreatePolicyDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    show_busy_indicator();

    const QString name = name_edit->text();

    // NOTE: since this is *display name*, not just name,
    // have to manually check for conflict. Server wouldn't
    // catch this.
    const bool name_conflict = [&]() {
        const QString base = g_adconfig->domain_head();
        const SearchScope scope = SearchScope_All;
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DISPLAY_NAME, name);
        const QList<QString> attributes = QList<QString>();
        const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

        return !results.isEmpty();
    }();

    if (name_conflict) {
        QMessageBox::warning(this, tr("Error"), tr("Group Policy Object with this name already exists."));

        return;
    }

    QString created_dn;
    const bool success = ad.create_gpo(name, created_dn);

    hide_busy_indicator();

    g_status()->display_ad_messages(ad, this);

    if (success) {
        // Create policy in console
        const QString base = created_dn;
        const SearchScope scope = SearchScope_Object;
        const QString filter = QString();
        const QList<QString> attributes = console_policy_search_attributes();
        const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

        const AdObject object = results[created_dn];

        console_policy_create(console, object);

        // NOTE: not adding policy object to the domain
        // tree, but i think it's ok?

        QDialog::accept();
    }
}
