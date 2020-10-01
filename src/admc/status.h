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

#include <QObject>

class QTextEdit;
class QStatusBar;

enum StatusType {
    StatusType_Success,
    StatusType_Error
};

// Pushes messages about AD operations to status bar and status log
class Status final : public QObject {
Q_OBJECT

public:
    static Status *instance();
    
    void init(QStatusBar *status_bar_arg, QTextEdit *status_log_arg);
    void message(const QString &msg, const StatusType &type);
    int get_errors_size() const;
    void show_errors_popup(int starting_index);

private slots:
    void on_toggle_show_status_log();

private:
    bool initialized = false;
    QStatusBar *status_bar = nullptr;
    QTextEdit* status_log = nullptr;
    QList<QString> errors;

    using QObject::QObject;

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;
    Status(Status&&) = delete;
    Status& operator=(Status&&) = delete;
};

#endif /* STATUS_BAR_H */
