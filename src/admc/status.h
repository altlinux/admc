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

#ifndef STATUS_H
#define STATUS_H

/**
 * Collects status messages and displays them in a status
 * bar and status log. Can also display a log of recent
 * error messages in a dialog.
 */

#include "utils.h"

class QTextEdit;
class QStatusBar;
class QString;
class QWidget;
class AdInterface;

enum StatusType {
    StatusType_Success,
    StatusType_Error
};

class Status {

public:   
    static Status *instance();

    QStatusBar *status_bar() const;
    QTextEdit *message_log() const;

    void add_message(const QString &msg, const StatusType &type);

    void display_ad_messages(const AdInterface &ad, QWidget *parent);

private:
    QStatusBar *m_status_bar;
    QTextEdit *m_message_log;

    Status();
};

Status *STATUS();

#endif /* STATUS_BAR_H */
