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

#ifndef STATUS_H
#define STATUS_H

/**
 * Collects status messages and displays them in a status
 * bar and status log. Can also display a log of recent
 * error messages in a dialog.
 */

class QTextEdit;
class QStatusBar;
class QString;
class QWidget;
class AdInterface;
class AdMessage;
template <typename T>
class QList;

enum StatusType {
    StatusType_Success,
    StatusType_Error
};

class Status {

public:
    void init(QStatusBar *statusbar, QTextEdit *message_log);

    void add_message(const QString &msg, const StatusType &type);

    void display_ad_messages(const QList<AdMessage> &messages, QWidget *parent);
    void display_ad_messages(const AdInterface &ad, QWidget *parent);

    // Display all messages in status log without dialogs
    void log_messages(const QList<AdMessage> &messages);
    void log_messages(const AdInterface &ad);

private:
    QStatusBar *m_status_bar;
    QTextEdit *m_message_log;
};

// Opens a dialog containing ad error messages in a
// scrollable list. Nothing is done if no errors occured.
void ad_error_log(const QList<AdMessage> &messages, QWidget *parent);
void ad_error_log(const AdInterface &ad, QWidget *parent);
void error_log(const QList<QString> error_list, QWidget *parent);

#endif /* STATUS_BAR_H */
