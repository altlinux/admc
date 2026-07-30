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

#include "attribute_edits/schedule_hours_edit.h"

#include "adldap.h"
#include "attribute_edits/schedule_hours_dialog.h"
#include "utils.h"

#include <QPushButton>

ScheduleHoursEdit::ScheduleHoursEdit(QPushButton *button_arg, QObject *parent)
: AttributeEdit(parent) {
    button = button_arg;

    connect(
        button, &QPushButton::clicked,
        this, &ScheduleHoursEdit::open_dialog);
}

void ScheduleHoursEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);

    if (object.is_class(CLASS_USER) || object.is_class(CLASS_INET_ORG_PERSON)) {
        schedule_attribute = ATTRIBUTE_LOGON_HOURS;
    }
    else if (object.is_class(CLASS_SITE_LINK)) {
        schedule_attribute = ATTRIBUTE_LINK_SCHEDULE;
    }
    else {
        schedule_attribute = QString();
    }

    current_value = object.get_value(schedule_attribute);
}

bool ScheduleHoursEdit::apply(AdInterface &ad, const QString &dn) const {
    if (schedule_attribute.isEmpty()) {
        return false;
    }

    const bool success = ad.attribute_replace_value(dn, schedule_attribute, current_value);

    return success;
}

void ScheduleHoursEdit::open_dialog() {
    ScheduleHoursDialog::ScheduleType type = schedule_attribute == ATTRIBUTE_LINK_SCHEDULE ?
                ScheduleHoursDialog::ScheduleType_SiteLink :
                ScheduleHoursDialog::ScheduleType_UserLogon;
    auto dialog = new ScheduleHoursDialog(current_value, button, type);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            const QByteArray new_value = dialog->get();
            const bool value_changed = (new_value != current_value);

            if (value_changed) {
                current_value = dialog->get();

                emit edited();
            }
        });
}
