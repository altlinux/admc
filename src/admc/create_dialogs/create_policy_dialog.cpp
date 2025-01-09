/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "ui_create_policy_dialog.h"

#include "adldap.h"
#include "console_impls/policy_impl.h"
#include "console_widget/console_widget.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QPushButton>

CreatePolicyDialog::CreatePolicyDialog(AdInterface &ad, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::CreatePolicyDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    const QString default_name = [&]() {
        const QList<QString> existing_name_list = [&]() {
            const QString base = g_adconfig->domain_dn();
            const SearchScope scope = SearchScope_All;
            const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
            const QList<QString> attributes = {ATTRIBUTE_DISPLAY_NAME};
            const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

            QList<QString> out;

            for (const AdObject &object : results.values()) {
                const QString name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
                out.append(name);
            }

            return out;
        }();

        const QString out = generate_new_name(existing_name_list, tr("New Group Policy Object"));

        connect(ui->name_edit, &QLineEdit::textChanged, this, &CreatePolicyDialog::on_edited);
        on_edited();

        return out;
    }();

    ui->name_edit->setText(default_name);
    ui->name_edit->selectAll();
    limit_edit(ui->name_edit, ATTRIBUTE_DISPLAY_NAME);

    settings_setup_dialog_geometry(SETTING_create_policy_dialog_geometry, this);
}

CreatePolicyDialog::~CreatePolicyDialog() {
    delete ui;
}

QString CreatePolicyDialog::get_created_dn() const {
    return created_dn;
}

void CreatePolicyDialog::accept() {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const QString name = ui->name_edit->text().trimmed();

    // NOTE: since this is *display name*, not just name,
    // have to manually check for conflict. Server wouldn't
    // catch this.
    const bool name_conflict = [&]() {
        const QString base = g_adconfig->domain_dn();
        const SearchScope scope = SearchScope_All;
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DISPLAY_NAME, name);
        const QList<QString> attributes = QList<QString>();
        const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

        return !results.isEmpty();
    }();

    if (name_conflict) {
        message_box_warning(this, tr("Error"), tr("Group Policy Object with this name already exists."));

        return;
    }

    const bool success = ad.gpo_add(name, created_dn);

    hide_busy_indicator();

    g_status->display_ad_messages(ad, this);

    if (success) {
        QDialog::accept();
    }
}

void CreatePolicyDialog::on_edited() {
    QRegExp reg_exp_spaces("^\\s*$");
    if (ui->name_edit->text().isEmpty() || ui->name_edit->text().contains(reg_exp_spaces)) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}
