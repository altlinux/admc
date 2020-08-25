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

#include "rename_dialog.h"
#include "ad_interface.h"
#include "attribute_edit.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>

// TODO: figure out what can and can't be renamed and disable renaming for exceptions (computers can't for example)

RenameDialog::RenameDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent)
{
    target = target_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    const QString name = AdInterface::instance()->attribute_get(target, ATTRIBUTE_NAME);
    const auto title_label = new QLabel(QString(tr("Renaming \"%1\":")).arg(name), this);

    const auto edits_layout = new QGridLayout();

    name_edit = new QLineEdit();
    append_to_grid_layout_with_label(edits_layout, tr("Name"), name_edit);

    const bool is_user = AdInterface::instance()->is_user(target);
    const bool is_group = AdInterface::instance()->is_group(target);
    QList<QString> string_attributes;
    if (is_user) {
        string_attributes = {
            ATTRIBUTE_FIRST_NAME,
            ATTRIBUTE_LAST_NAME,
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_USER_PRINCIPAL_NAME,
            ATTRIBUTE_SAMACCOUNT_NAME
        };

    } else if (is_group) {
        string_attributes = {
            ATTRIBUTE_SAMACCOUNT_NAME
        };
    }

    QMap<QString, StringEdit *> string_edits;
    make_string_edits(string_attributes, &string_edits);

    if (is_user) {
        autofill_full_name(string_edits);
    }

    if (string_attributes.contains(ATTRIBUTE_SAMACCOUNT_NAME)) {
        QLineEdit *sama_name_edit = string_edits[ATTRIBUTE_SAMACCOUNT_NAME]->edit;
        autofill_edit_from_other_edit(name_edit, sama_name_edit);
    }

    for (auto attribute : string_attributes) {
        all_edits.append(string_edits[attribute]);
    }
    layout_attribute_edits(all_edits, edits_layout, this);

    const auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel, this);
    connect(
        button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &QDialog::reject);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(button_box);

    for (auto edit : all_edits) {
        edit->load(target);
    }
}

void RenameDialog::accept() {
    const QString new_name = name_edit->text();

    const bool verify_success = verify_attribute_edits(all_edits, this);
    if (!verify_success) {
        return;
    }

    const int errors_index = Status::instance()->get_errors_size();
    AdInterface::instance()->start_batch();
    {

        // NOTE: rename last so that other attribute changes can complete on old DN
        const bool result_apply = apply_attribute_edits(all_edits, target, this);
        const AdResult result_rename = AdInterface::instance()->object_rename(target, new_name);

        if (result_apply && result_rename.success) {
            const QString message = QString(tr("Renamed object - \"%1\"")).arg(new_name);
            Status::instance()->message(message, StatusType_Success);

            QDialog::accept();
        } else {
            const QString message = QString(tr("Failed to rename object - \"%1\"")).arg(new_name);
            Status::instance()->message(message, StatusType_Error);
        }
    }
    AdInterface::instance()->end_batch();
    Status::instance()->show_errors_popup(errors_index);
}
