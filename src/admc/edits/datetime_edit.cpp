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

#include "edits/datetime_edit.h"

#include "adldap.h"
#include "globals.h"

#include <QDateTimeEdit>
#include <QFormLayout>

DateTimeEdit::DateTimeEdit(QDateTimeEdit *edit_arg, const QString &attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    edit = edit_arg;
    attribute = attribute_arg;
    attribute = attribute_arg;

    init();
}


DateTimeEdit::DateTimeEdit(const QString &attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    edit = new QDateTimeEdit();
    attribute = attribute_arg;

    init();
}

void DateTimeEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QDateTime datetime = object.get_datetime(attribute, g_adconfig);
    const QDateTime datetime_local = datetime.toLocalTime();

    edit->setDateTime(datetime_local);
}

void DateTimeEdit::set_read_only(const bool read_only) {
    edit->setDisabled(read_only);
}

void DateTimeEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(attribute, "") + ":";
    layout->addRow(label_text, edit);
}

bool DateTimeEdit::apply(AdInterface &ad, const QString &dn) const {
    const QDateTime datetime_local = edit->dateTime();
    const QDateTime datetime = datetime_local.toUTC();

    const bool success = ad.attribute_replace_datetime(dn, attribute, datetime);

    return success;
}

void DateTimeEdit::init() {
    edit->setDisplayFormat(DATETIME_DISPLAY_FORMAT);

    QObject::connect(
        edit, &QDateTimeEdit::dateTimeChanged,
        [this]() {
            emit edited();
        });
}
