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

#include "create_policy_dialog.h"

#include "adldap.h"
#include "status.h"
#include "globals.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>

// TODO: implement checkbox for account option "User cannot change password". Can't just do it through UAC attribute bits.

// TODO: not sure about how required_edits are done, maybe
// just do this through verify()? Had to remove upnedit from
// required_edits because that's a list of stringedits. Now upnedit checks that it's not empty in verify();

CreatePolicyDialog::CreatePolicyDialog(QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(400);

    const auto title = QString(tr("Create GPO"));
    setWindowTitle(title);

    name_edit = new QLineEdit();
    name_edit->setText("New Group Policy Object");

    const auto edits_layout = new QFormLayout();
    edits_layout->addRow(tr("Name"), name_edit);

    auto create_button = new QPushButton(tr("Create"));

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(edits_layout);
    layout->addWidget(create_button);

    connect(
        create_button, &QAbstractButton::clicked,
        this, &CreatePolicyDialog::accept);
}

QString CreatePolicyDialog::get_created_dn() const {
    return created_dn;
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
    const bool name_conflict =
    [&]() {
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DISPLAY_NAME, name);
        const QHash<QString, AdObject> results = ad.search(filter, QList<QString>(), SearchScope_All);

        return !results.isEmpty();
    }();

    if (name_conflict) {
        QMessageBox::warning(this, tr("Error"), tr("Group Policy Object with this name already exists."));

        return;
    }

    const bool success = ad.create_gpo(name, created_dn);

    hide_busy_indicator();

    g_status->display_ad_messages(ad, this);

    if (success) {
        QDialog::accept();
    }
}
