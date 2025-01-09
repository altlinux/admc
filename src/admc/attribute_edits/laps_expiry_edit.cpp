/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "attribute_edits/laps_expiry_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QDateTimeEdit>
#include <QPushButton>

LAPSExpiryEdit::LAPSExpiryEdit(QDateTimeEdit *edit_arg, QPushButton *reset_expiry_button, QObject *parent)
: AttributeEdit(parent) {
    edit = edit_arg;

    connect(
        edit, &QDateTimeEdit::dateTimeChanged,
        this, &AttributeEdit::edited);
    connect(
        reset_expiry_button, &QPushButton::clicked,
        this, &LAPSExpiryEdit::reset_expiry);
}

void LAPSExpiryEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    const QDateTime datetime = object.get_datetime(ATTRIBUTE_LAPS_EXPIRATION, g_adconfig);
    const QDateTime datetime_local = datetime.toLocalTime();

    edit->setDateTime(datetime_local);
}

bool LAPSExpiryEdit::apply(AdInterface &ad, const QString &dn) const {
    const QDateTime datetime_local = edit->dateTime();
    const QDateTime datetime = datetime_local.toUTC();

    const bool success = ad.attribute_replace_datetime(dn, ATTRIBUTE_LAPS_EXPIRATION, datetime);

    return success;
}

void LAPSExpiryEdit::reset_expiry() {
    const QDateTime current_datetime_local = QDateTime::currentDateTime();
    edit->setDateTime(current_datetime_local);

    emit edited();
}
