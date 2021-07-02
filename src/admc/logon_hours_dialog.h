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

#ifndef LOGON_HOURS_DIALOG_H
#define LOGON_HOURS_DIALOG_H

/**
 *
 */

#include <QDialog>

class QTableView;
class QStandardItemModel;
class QRadioButton;

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

#define LOGON_HOURS_SIZE (7 * 3)

class LogonHoursDialog : public QDialog {
    Q_OBJECT

public:
    LogonHoursDialog(QWidget *parent);

    void load(const QByteArray &value);
    QByteArray get() const;

    void accept() override;
    void reject() override;

private:
    QTableView *view;
    QStandardItemModel *model;
    QByteArray original_value;
    QRadioButton *local_time_button;

    void switch_to_local_time();
    void switch_to_utc_time();
    int get_offset() const;
    bool is_local_time;
};

QList<QList<bool>> logon_hours_to_bools(const QByteArray &byte_list, const int time_offset = 0);
QByteArray logon_hours_to_bytes(const QList<QList<bool>> bool_list, const int time_offset = 0);

#endif /* LOGON_HOURS_DIALOG_H */
