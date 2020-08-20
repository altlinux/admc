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
#include "status.h"

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QList>

// TODO: figure out what can and can't be renamed and disable renaming for exceptions (computers can't for example)

RenameDialog::RenameDialog(const QString &target_arg, QWidget *parent)
: QDialog(parent)
{
    target = target_arg;

    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);

    const QString name = AdInterface::instance()->attribute_get(target, ATTRIBUTE_NAME);
    const auto title_label = new QLabel(QString(tr("Renaming \"%1\":")).arg(name), this);

    const auto ok_button = new QPushButton(tr("OK"), this);
    connect(
        ok_button, &QAbstractButton::clicked,
        this, &QDialog::accept);

    const auto cancel_button = new QPushButton(tr("Cancel"), this);
    connect(
        cancel_button, &QAbstractButton::clicked,
        this, &QDialog::reject);

    label_layout = new QVBoxLayout();
    edit_layout = new QVBoxLayout();

    const auto attributes_layout = new QHBoxLayout();
    attributes_layout->insertLayout(-1, label_layout);
    attributes_layout->insertLayout(-1, edit_layout);

    const auto top_layout = new QGridLayout(this);
    top_layout->addWidget(title_label, 0, 0);
    top_layout->addLayout(attributes_layout, 1, 0);
    top_layout->addWidget(cancel_button, 2, 0, Qt::AlignLeft);
    top_layout->addWidget(ok_button, 2, 2, Qt::AlignRight);

    name_edit = {
        "",
        nullptr
    };

    add_attribute_edit(ATTRIBUTE_NAME, tr("Name:"));

    // Add extra name-related attribute edits for users/groups
    const bool is_user = AdInterface::instance()->is_user(target);
    const bool is_group = AdInterface::instance()->is_group(target);
    if (is_user) {
        add_attribute_edit(ATTRIBUTE_FIRST_NAME, tr("First name:"));
        add_attribute_edit(ATTRIBUTE_SN, tr("Last name:"));
        add_attribute_edit(ATTRIBUTE_DISPLAY_NAME, tr("Display name:"));
        add_attribute_edit(ATTRIBUTE_USER_PRINCIPAL_NAME, tr("Logon name:"));
        add_attribute_edit(ATTRIBUTE_SAMACCOUNT_NAME, tr("Logon name (pre-2000):"));
    } if (is_group) {
        add_attribute_edit(ATTRIBUTE_SAMACCOUNT_NAME, tr("Logon name (pre-2000):"));
    }

    connect(
        this, &QDialog::accepted,
        this, &RenameDialog::on_accepted);
}

void RenameDialog::on_accepted() {
    auto push_changes_from_attribute_edit =
    [this](const AttributeEdit &e) {
        const QString attribute = e.attribute;
        const QLineEdit *edit = e.edit;

        const QString current_value = AdInterface::instance()->attribute_get(target, attribute);
        const QString new_value = edit->text();

        const bool changed = (new_value != current_value);
        if (changed) {
            if (attribute == ATTRIBUTE_NAME) {
                AdInterface::instance()->object_rename(target, new_value);
            } else {
                AdInterface::instance()->attribute_replace(target, attribute, new_value);
            }
        }
    };

    // NOTE: change name last so that other attribute changes can complete on old DN
    for (auto e : edits) {
        push_changes_from_attribute_edit(e);
    }

    if (name_edit.edit != nullptr) {
        push_changes_from_attribute_edit(name_edit);
    }

    const QString name = AdInterface::instance()->attribute_get(target, ATTRIBUTE_NAME);
    Status::instance()->message(QString(tr("Renamed object - %1")).arg(name), StatusType_Success);
}

void RenameDialog::add_attribute_edit(const QString &attribute, const QString &label_text) {
    const auto label = new QLabel(label_text);
    auto edit = new QLineEdit();

    label_layout->addWidget(label);
    edit_layout->addWidget(edit);

    const QString current_value = AdInterface::instance()->attribute_get(target, attribute);
    edit->setText(current_value);

    AttributeEdit attribute_edit = {
        QString(attribute),
        edit
    };

    if (attribute == ATTRIBUTE_NAME) {
        name_edit = attribute_edit;
    } else {
        edits.append(attribute_edit);
    }
}
