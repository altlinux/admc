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

#ifndef STATUS_BAR_H
#define STATUS_BAR_H

/**
 * Collects status messages and displays them in a status
 * bar and status log. Can also display a log of recent
 * error messages in a dialog.
 */

#include <QObject>

class QTextEdit;
class QStatusBar;

enum StatusType {
    StatusType_Success,
    StatusType_Error
};

class Status final : public QObject {
Q_OBJECT

public:   
    QStatusBar *status_bar;
    QTextEdit *status_log;

    static Status *instance();

    void message(const QString &msg, const StatusType &type);

    // To show an error log for operation(s) that result in
    // Status messages, call start_error_log() before
    // performing operation(s) and end_error_log() after. If
    // any errors occured, error log will open when
    // end_error_log() is called.
    void start_error_log();
    void end_error_log();

private:
    QList<QString> error_log;

    Status();

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;
    Status(Status&&) = delete;
    Status& operator=(Status&&) = delete;
};

Status *STATUS();

#endif /* STATUS_BAR_H */
