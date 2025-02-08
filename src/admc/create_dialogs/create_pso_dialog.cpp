/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2024 BaseALT Ltd.
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

#include "create_pso_dialog.h"
#include "ui_create_pso_dialog.h"
#include "ad_interface.h"
#include "utils.h"
#include "ad_utils.h"
#include "status.h"
#include "globals.h"
#include "attribute_edits/protect_deletion_edit.h"

#include <QPushButton>
#include <QLineEdit>

CreatePSODialog::CreatePSODialog(const QString &parent_dn_arg, QWidget *parent) :
    CreateObjectDialog(parent),
    ui(new Ui::CreatePSODialog),
    parent_dn(parent_dn_arg) {

    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    deletion_edit = new ProtectDeletionEdit(ui->protect_deletion_checkbox, this);

    connect(ui->pso_edit_widget->name_line_edit(), &QLineEdit::textEdited,
            [this](const QString &text) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(text.isEmpty());
    });
}

CreatePSODialog::~CreatePSODialog() {
    delete ui;
}

void CreatePSODialog::accept() {
    const QString name = ui->pso_edit_widget->name_line_edit()->text().trimmed();
    bool name_verified = verify_object_name(name, this);
    if (!name_verified) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    if (ui->pso_edit_widget->settings_are_default()) {
        message_box_warning(this, "", tr("At least one password setting (except precedence) should not be default"));
        return;
    }

    QHash<QString, QList<QString>> pso_string_settings = ui->pso_edit_widget->pso_settings_string_values();
    QHash<QString, QList<QString>> attrs_map = {
        {ATTRIBUTE_OBJECT_CLASS, {CLASS_PSO}},
    };
    for (const auto& attribute : pso_string_settings.keys()) {
        attrs_map[attribute] = pso_string_settings[attribute];
    }

    const QString dn = get_created_dn();

    const bool add_success = ad.object_add(dn, attrs_map);
    g_status->display_ad_messages(ad, this);
    if (!add_success) {
        g_status->add_message(tr("Failed to create password settings object %1").arg(name), StatusType_Error);
        return;
    }

    deletion_edit->apply(ad, dn);

    g_status->add_message(tr("Password settings object %1 has been successfully created.").arg(name), StatusType_Success);
    QDialog::accept();
}

QString CreatePSODialog::get_created_dn() const {
    const QString name = ui->pso_edit_widget->name_line_edit()->text().trimmed();
    const QString dn = dn_from_name_and_parent(name, parent_dn, CLASS_PSO);
    return dn;
}


