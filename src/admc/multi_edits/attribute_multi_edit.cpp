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

#include "multi_edits/attribute_multi_edit.h"

#include "multi_tabs/properties_multi_tab.h"

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>

AttributeMultiEdit::AttributeMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent)
: QObject(parent) {
    if (edits_out.contains(this)) {
        qDebug() << "ERROR: attribute edit added twice to list!";
    } else {
        edits_out.append(this);
    }

    apply_check = new QCheckBox();
    label = new QLabel();

    check_and_label_wrapper = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    check_and_label_wrapper->setLayout(layout);
    layout->addWidget(apply_check);
    layout->addWidget(label);

    connect(
        apply_check, &QAbstractButton::toggled,
        this, &AttributeMultiEdit::on_check_toggled);
}

bool AttributeMultiEdit::apply(AdInterface &ad, const QList<QString> &target_list) {
    const bool need_to_apply = apply_check->isChecked();
    if (!need_to_apply) {
        return true;
    }

    bool total_success = true;

    for (const QString &target : target_list) {
        const bool success = apply_internal(ad, target);

        if (!success) {
            total_success = false;
        }
    }

    apply_check->setChecked(false);

    return total_success;
}

void AttributeMultiEdit::reset() {
    // NOTE: when apply_check is unchecked, the
    // on_check_toggled() calls set_enabled()
    apply_check->setChecked(false);
}

void AttributeMultiEdit::on_check_toggled() {
    if (apply_check->isChecked()) {
        emit edited();
    }

    // NOTE: call set_enabled() of the subclass
    const bool enabled = apply_check->isChecked();
    set_enabled(enabled);
}

void multi_edits_connect_to_tab(const QList<AttributeMultiEdit *> &edits, PropertiesMultiTab *tab) {
    for (auto edit : edits) {
        QObject::connect(
            edit, &AttributeMultiEdit::edited,
            tab, &PropertiesMultiTab::on_edit_edited);
    }
}

void multi_edits_add_to_layout(const QList<AttributeMultiEdit *> &edits, QFormLayout *layout) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}
