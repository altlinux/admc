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

#include "details_tab.h"
#include "details_widget.h"
#include "ad_interface.h"

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

DetailsTab::DetailsTab(DetailsWidget *details_arg)
: QWidget(details_arg) {
    details = details_arg;
    title = tr("Default tab title");
}

QString DetailsTab::target() const {
    return details->get_target();
}

QString DetailsTab::get_title() const {
    return title;
}

void DetailsTab::add_attribute_edit(const QString &attribute, const QString &label_text, QLayout *label_layout, QLayout *edit_layout) {
    auto label = new QLabel(label_text, this);
    auto edit = new QLineEdit(this);

    label_layout->addWidget(label);
    edit_layout->addWidget(edit);

    connect(
        edit, &QLineEdit::editingFinished,
        [this, edit, attribute]() {
            const QString new_value = edit->text();
            const QString current_value = AdInterface::instance()->attribute_get(target(), attribute);
            edit->text();

            if (new_value != current_value) {
                AdInterface::instance()->attribute_replace(target(), attribute, new_value);
            }
        });
    reload_attribute_edit(edit, attribute);
}

void DetailsTab::add_attribute_display(const QString &attribute, const QString &label_text, QLayout *label_layout, QLayout *edit_layout) {
    auto label = new QLabel(label_text, this);
    auto edit = new QLineEdit(this);

    edit->setReadOnly(true);

    label_layout->addWidget(label);
    edit_layout->addWidget(edit);

    reload_attribute_edit(edit, attribute);
}

void DetailsTab::reload_attribute_edit(QLineEdit *edit, const QString &attribute) {
    connect(
        this, &DetailsTab::reloaded,
        [this, edit, attribute]() {
            QString current_value;

            if (attribute_is_datetime(attribute)) {
                const QString datetime_raw = AdInterface::instance()->attribute_get(target(), attribute);
                current_value = datetime_raw_to_string(attribute, datetime_raw);
            } else if (attribute == ATTRIBUTE_OBJECT_CLASS) {
                // TODO: not sure how to get the "primary" attribute, for now just getting the last one
                const QList<QString> classes = AdInterface::instance()->attribute_get_multi(target(), attribute);
                current_value = classes.last();
            } else {
                current_value = AdInterface::instance()->attribute_get(target(), attribute);
            }

            edit->setText(current_value);
        });
}
