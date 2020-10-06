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
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
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

RenameDialog::RenameDialog(const QString &target_arg)
: QDialog()
{
    target = target_arg;
    const AdObject object = AD()->request_all(target);

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    const auto title_label = new QLabel(QString(tr("Rename dialog")), this);

    const auto edits_layout = new QGridLayout();

    QList<QString> string_attributes;
    QString objectClass;
    if (object.is_user()) {
        string_attributes = {
            ATTRIBUTE_NAME,
            ATTRIBUTE_FIRST_NAME,
            ATTRIBUTE_LAST_NAME,
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_USER_PRINCIPAL_NAME,
            ATTRIBUTE_SAMACCOUNT_NAME
        };
        objectClass = CLASS_USER;
    } else if (object.is_group()) {
        string_attributes = {
            ATTRIBUTE_NAME,
            ATTRIBUTE_SAMACCOUNT_NAME
        };
        objectClass = CLASS_GROUP;
    } else if (object.is_policy()) {
        string_attributes = {
            ATTRIBUTE_DISPLAY_NAME
        };
        // TODO: there's no display has no display name for displayName(heh). No group policy display specifier either.
        objectClass = CLASS_CONTAINER;
    }

    make_string_edits(object, string_attributes, objectClass, &string_edits, &all_edits, this);
    setup_string_edit_autofills(string_edits);

    layout_attribute_edits(all_edits, edits_layout);

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
}

void RenameDialog::accept() {
    const bool verify_success = verify_attribute_edits(all_edits, this);
    if (!verify_success) {
        return;
    }

    const int errors_index = Status::instance()->get_errors_size();
    AD()->start_batch();
    {
        // NOTE: apply attribute changes before renaming so that attribute changes can complete on old DN
        const bool apply_success = apply_attribute_edits(all_edits, target);

        auto get_attribute_from_edit =
        [this](const QString &attribute) -> QString {
            const StringEdit *edit = string_edits[attribute];
            const QString value = edit->edit->text();

            return value;
        };

        const bool rename_success =
        [this, get_attribute_from_edit]() {
            if (string_edits.contains(ATTRIBUTE_NAME)) {
                const QString new_name = get_attribute_from_edit(ATTRIBUTE_NAME);

                return AD()->object_rename(target, new_name);
            } else {
                // NOTE: for policies, object is never actually renamed, only the display name changes
                return true;
            }
        }();

        const QString name_for_message =
        [this, get_attribute_from_edit]() {
            if (string_edits.contains(ATTRIBUTE_NAME)) {
                return get_attribute_from_edit(ATTRIBUTE_NAME);
            } else {
                return get_attribute_from_edit(ATTRIBUTE_DISPLAY_NAME);
            }
        }();

        if (apply_success && rename_success) {
            const QString message = QString(tr("Renamed object - \"%1\"")).arg(name_for_message);
            Status::instance()->message(message, StatusType_Success);

            QDialog::accept();
        } else {
            const QString message = QString(tr("Failed to rename object - \"%1\"")).arg(name_for_message);
            Status::instance()->message(message, StatusType_Error);
        }
    }
    AD()->end_batch();
    Status::instance()->show_errors_popup(errors_index);
}
