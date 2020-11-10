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
    const AdObject object = AD()->search_object(target);

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    const auto title_label = new QLabel(QString(tr("Rename dialog")), this);

    const auto edits_layout = new QGridLayout();

    const QString object_class = object.get_string(ATTRIBUTE_OBJECT_CLASS);

    old_name_for_message =
    [object]() {
        if (object.is_class(CLASS_GP_CONTAINER)) {
            return object.get_string(ATTRIBUTE_DISPLAY_NAME);
        } else {
            return object.get_string(ATTRIBUTE_NAME);
        }
    }();

    name_edit = nullptr;

    if (object.is_class(CLASS_USER)) {
        name_edit = new StringEdit(ATTRIBUTE_NAME, object_class, this, &all_edits);

        const QList<QString> attributes = {
            ATTRIBUTE_FIRST_NAME,
            ATTRIBUTE_LAST_NAME,
            ATTRIBUTE_DISPLAY_NAME,
            ATTRIBUTE_USER_PRINCIPAL_NAME,
            ATTRIBUTE_SAMACCOUNT_NAME
        };
        make_string_edits(attributes, object_class, this, &all_edits);
    } else if (object.is_class(CLASS_GROUP)) {
        name_edit = new StringEdit(ATTRIBUTE_NAME, object_class, this, &all_edits);
        new StringEdit(ATTRIBUTE_SAMACCOUNT_NAME, object_class, this, &all_edits);
    } else if (object.is_class(CLASS_GP_CONTAINER)) {
        // TODO: no display specifier for "displayName" for "policy" class, hardcode it?
        new StringEdit(ATTRIBUTE_DISPLAY_NAME, object_class, this, &all_edits);
    }

    edits_add_to_layout(all_edits, edits_layout);
    edits_load(all_edits, object);

    auto button_box = new QDialogButtonBox();
    ok_button = button_box->addButton(QDialogButtonBox::Ok);
    reset_button = button_box->addButton(QDialogButtonBox::Reset);
    connect(
        ok_button, &QPushButton::clicked,
        this, &QDialog::accept);
    connect(
        reset_button, &QPushButton::clicked,
        this, &RenameDialog::reset);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(button_box);

    for (auto edit : all_edits) {
        connect(
            edit, &AttributeEdit::edited,
            this, &RenameDialog::on_edited);
    }
    on_edited();
}

void RenameDialog::accept() {
    const bool verify_success = edits_verify(all_edits);
    if (!verify_success) {
        return;
    }

    const int errors_index = Status::instance()->get_errors_size();
    AD()->start_batch();
    {
        // NOTE: apply attribute changes before renaming so that attribute changes can complete on old DN
        const bool apply_success = edits_apply(all_edits, target);

        const bool rename_success =
        [this]() {
            if (name_edit != nullptr) {
                const QString new_name = name_edit->get_input();
                
                return AD()->object_rename(target, new_name);
            } else {
                // NOTE: for policies, object is never actually renamed, only the display name changes
                return true;
            }
        }();

        if (apply_success && rename_success) {
            const QString message = QString(tr("Renamed object - \"%1\"")).arg(old_name_for_message);
            Status::instance()->message(message, StatusType_Success);
        } else {
            const QString message = QString(tr("Failed to rename object - \"%1\"")).arg(old_name_for_message);
            Status::instance()->message(message, StatusType_Error);
        }
    }
    AD()->end_batch();

    QDialog::close();
    Status::instance()->show_errors_popup(errors_index);
}

void RenameDialog::on_edited() {
    const bool any_changed = edits_changed(all_edits);
    const bool name_not_empty = !name_edit->get_input().isEmpty();
    
    reset_button->setEnabled(any_changed);
    ok_button->setEnabled(name_not_empty && any_changed);
}

void RenameDialog::reset() {
    const AdObject object = AD()->search_object(target);
    edits_load(all_edits, object);
}
