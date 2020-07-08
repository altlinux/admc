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

#include "ad_interface.h"

class QTextEdit;
class QStatusBar;

// Pushes messages about AD operations to status bar and status log
class Status final : public QObject {
Q_OBJECT

public:
    explicit Status(QStatusBar *status_bar_arg, QTextEdit *status_log_arg, QObject *parent);

private slots:
    void on_toggle_show_status_log(bool checked);

private:
    QStatusBar *status_bar = nullptr;
    QTextEdit* status_log = nullptr;

    void message(const QString &msg, int status_bar_timeout = 0);

};

#endif /* STATUS_BAR_H */
