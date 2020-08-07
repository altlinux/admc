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

#include "general_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QDialog>

// TODO: logon hours, logon computers, remove crazy spacing

// NOTE: https://ldapwiki.com/wiki/MMC%20Account%20Tab

GeneralWidget::GeneralWidget(QWidget *parent)
: QWidget(parent)
{   
    name_label = new QLabel(this);

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(name_label);

    auto make_line_edit =
    [this, layout](const QString &attribute, const QString &label_text) {
        auto label = new QLabel(label_text, this);
        auto edit = new QLineEdit(this);

        auto edit_layout = new QHBoxLayout();
        edit_layout->addWidget(label);
        edit_layout->addWidget(edit);

        layout->insertLayout(-1, edit_layout);

        connect(
            edit, &QLineEdit::editingFinished,
            [this, edit, attribute]() {
                const QString new_value = edit->text();
                const QString current_value = AdInterface::instance()->attribute_get(target_dn, attribute);
                edit->text();

                if (new_value != current_value) {
                    AdInterface::instance()->attribute_replace(target_dn, attribute, new_value);
                }
            });
        connect(
            this, &GeneralWidget::target_changed,
            [this, edit, attribute]() {
                const QString current_value = AdInterface::instance()->attribute_get(target_dn, attribute);

                edit->setText(current_value);
            });
    };

    make_line_edit("displayName", tr("Display name:"));
}

void GeneralWidget::change_target(const QString &dn) {
    target_dn = dn;

    const QString name = AdInterface::instance()->attribute_get(target_dn, ATTRIBUTE_NAME);
    name_label->setText(name);

    emit target_changed();
}
