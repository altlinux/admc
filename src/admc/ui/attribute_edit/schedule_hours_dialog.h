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

#ifndef LOGON_HOURS_DIALOG_H
#define LOGON_HOURS_DIALOG_H

/**
 * Dialog for editing logon hours of a user account.
 */

#include <QDialog>

class QStandardItemModel;

// NOTE: this is the order in logon hours value. Different
// from the display order in the dialog where first day is monday
enum Weekday {
    Weekday_Sunday,
    Weekday_Monday,
    Weekday_Tuesday,
    Weekday_Wednesday,
    Weekday_Thursday,
    Weekday_Friday,
    Weekday_Saturday,
    Weekday_COUNT,
};

// TODO: Split that dialog on two dialogs for user logon and site link
// schedules

const int DAYS_IN_WEEK = 7;
const int HOURS_IN_DAY = 24;
const int LOGON_HOURS_SIZE = 21;

// Site link schedule attribute constants (for site replication)
const int SITE_LINK_SCHEDULE_TOTAL_SIZE = 188;
const int SITE_LINK_SCHEDULE_HEADER_SIZE = 20;
const int SITE_LINK_SCHEDULE_DATA_SIZE = 168;

// Values for schedule bytes
const char SITE_LINK_SCHEDULE_ALLOWED = (char)0xFF;   // Replication allowed
const char SITE_LINK_SCHEDULE_DENIED = (char)0xF0;    // Replication denied (or other value you see)

namespace Ui {
class ScheduleHoursDialog;
}

class ScheduleHoursDialog : public QDialog {
    Q_OBJECT

public:
    enum ScheduleType {
        ScheduleType_SiteLink,
        ScheduleType_UserLogon
    };

    Ui::ScheduleHoursDialog *ui;

    ScheduleHoursDialog(const QByteArray &value, QWidget *parent, ScheduleType type_arg = ScheduleType_UserLogon);
    ~ScheduleHoursDialog();

    QByteArray get() const;
    void load(const QByteArray &value);

private:
    QStandardItemModel *model;
    QByteArray original_value;

    void switch_to_local_time();
    void on_local_time_button_toggled(bool checked);
    int get_offset() const;
    bool is_local_time;
    ScheduleType type;

    const unsigned char sitelink_schedule_header_bytes[20] = {
        0xBC, 0x00, 0x00, 0x00,  // Size = 188
        0x00, 0x00, 0x00, 0x00,  // Bandwidth = 0
        0x01, 0x00, 0x00, 0x00,  // NumSchedules = 1
        0x00, 0x00, 0x00, 0x00,  // Reserved = 0
        0x14, 0x00, 0x00, 0x00   // Offset = 20
    };
};

QList<QList<bool>> logon_hours_to_bools(const QByteArray &byte_list, const int time_offset = 0);
QByteArray logon_hours_to_bytes(const QList<QList<bool>> bool_list, const int time_offset = 0);

// Convert site schedule attribute (byte-per-hour format) to bool grid
QList<QList<bool>> site_schedule_bytes_to_bools(const QByteArray &byte_list_arg, const int time_offset = 0);
// Convert bool grid to schedule attribute (byte-per-hour format)
QByteArray bools_to_site_schedule_bytes(const QList<QList<bool>> bool_list, const int time_offset);

int get_current_utc_offset();

#endif /* LOGON_HOURS_DIALOG_H */
