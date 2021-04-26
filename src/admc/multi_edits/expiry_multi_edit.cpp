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

#include "multi_edits/expiry_multi_edit.h"

#include "edits/expiry_widget.h"
#include "adldap.h"
#include "globals.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>

ExpiryMultiEdit::ExpiryMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: AttributeMultiEdit(edits_out, parent)
{
    edit_widget = new ExpiryWidget();

    check = new QCheckBox();

    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";
    auto label = new QLabel(label_text);

    check_and_label_wrapper = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    check_and_label_wrapper->setLayout(layout);
    layout->addWidget(check);
    layout->addWidget(label);

    connect(
        check, &QAbstractButton::toggled,
        this, &ExpiryMultiEdit::on_check_toggled);
    connect(
        edit_widget, &ExpiryWidget::edited,
        [this]() {
            emit edited();
        });

    on_check_toggled();
}

void ExpiryMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(check_and_label_wrapper, edit_widget);
}

bool ExpiryMultiEdit::apply(AdInterface &ad, const QList<QString> &target_list) {
    const bool need_to_apply = check->isChecked();
    if (!need_to_apply) {
        return true;
    }

    bool total_success = true;

    for (const QString &target : target_list) {
        const bool success = edit_widget->apply(ad, target);

        if (!success) {
            total_success = false;
        }
    }

    check->setChecked(false);


    return total_success;
}

void ExpiryMultiEdit::reset() {
    check->setChecked(false);
}

void ExpiryMultiEdit::on_check_toggled() {
    if (check->isChecked()) {
        emit edited();
    }

    edit_widget->setEnabled(check->isChecked());
}
