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

#include "editors/bool_editor.h"
#include "adldap.h"
#include "globals.h"

#include <QRadioButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

BoolEditor::BoolEditor(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: AttributeEditor(parent)
{
    setWindowTitle(tr("Edit boolean"));

    QLabel *attribute_label = make_attribute_label(attribute);

    true_button = new QRadioButton(tr("True"));
    false_button = new QRadioButton(tr("False"));
    unset_button = new QRadioButton(tr("Unset"));

    if (values.isEmpty()) {
        unset_button->setChecked(true);
    } else {
        const QByteArray value = values[0];
        const QString value_string = QString(value);
        const bool value_bool = ad_string_to_bool(value_string);

        if (value_bool) {
            true_button->setChecked(true);        
        } else {
            false_button->setChecked(true);
        }
    }

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
    layout->addWidget(true_button);
    layout->addWidget(false_button);
    layout->addWidget(unset_button);
    layout->addWidget(button_box);

    if (g_adconfig->get_attribute_is_system_only(attribute)) {
        true_button->setEnabled(false);
        false_button->setEnabled(false);
        unset_button->setEnabled(false);
    }
}

QList<QByteArray> BoolEditor::get_new_values() const {
    if (unset_button->isChecked()) {
        return QList<QByteArray>();
    } else if (true_button->isChecked()) {
        const QByteArray value = QString(LDAP_BOOL_TRUE).toUtf8();
        return {value};
    } else if (false_button->isChecked()) {
        const QByteArray value = QString(LDAP_BOOL_FALSE).toUtf8();
        return {value};
    } else {
        return QList<QByteArray>();
    }
}
