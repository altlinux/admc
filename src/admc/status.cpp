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

#include "status.h"

#include "adldap.h"
#include "error_log_dialog.h"
#include "globals.h"
#include "settings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

#define MAX_MESSAGES_IN_LOG 200

void Status::init(QStatusBar *statusbar, QTextEdit *message_log) {
    m_status_bar = statusbar;
    m_message_log = message_log;
}

void Status::add_message(const QString &msg, const StatusType &type) {
    if (m_status_bar == nullptr || m_message_log == nullptr) {
        return;
    }

    m_status_bar->showMessage(msg);

    const QString timestamp = []() {
        const QDateTime current_datetime = QDateTime::currentDateTime();
        return current_datetime.toString("hh:mm:ss");
    }();

    const QString timestamped_msg = QString("%1 %2").arg(timestamp, msg);

    const bool timestamps_ON = settings_get_variant(SETTING_timestamp_log).toBool();

    const QColor color = [type]() {
        switch (type) {
            case StatusType_Success: return Qt::darkGreen;
            case StatusType_Error: return Qt::red;
        }
        return Qt::black;
    }();

    const QColor original_color = m_message_log->textColor();
    m_message_log->setTextColor(color);
    if (timestamps_ON) {
        m_message_log->append(timestamped_msg);
    } else {
        m_message_log->append(msg);
    }
    m_message_log->setTextColor(original_color);

    // Limit number of messages in log by deleting old ones
    // once over limit
    QTextCursor cursor = m_message_log->textCursor();
    const int message_count = cursor.blockNumber();
    if (message_count > MAX_MESSAGES_IN_LOG) {
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 0);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    }

    // Move cursor to newest message
    QTextCursor end_cursor = m_message_log->textCursor();
    end_cursor.movePosition(QTextCursor::End);
    m_message_log->setTextCursor(end_cursor);
}

void Status::display_ad_messages(const QList<AdMessage> &messages, QWidget *parent) {

    log_messages(messages);

    ad_error_log(messages, parent);
}

void Status::display_ad_messages(const AdInterface &ad, QWidget *parent) {
    const QList<AdMessage> messages = ad.messages();

    display_ad_messages(messages, parent);
}

void Status::log_messages(const QList<AdMessage> &messages) {
    if (m_status_bar == nullptr || m_message_log == nullptr) {
        return;
    }

    for (const AdMessage &message : messages) {
        const StatusType status_type = [message]() {
            switch (message.type()) {
                case AdMessageType_Success: return StatusType_Success;
                case AdMessageType_Error: return StatusType_Error;
            }
            return StatusType_Success;
        }();

        add_message(message.text(), status_type);
    }
}

void Status::log_messages(const AdInterface &ad) {
    const QList<AdMessage> messages = ad.messages();

    log_messages(messages);
}

void ad_error_log(const QList<AdMessage> &messages, QWidget *parent) {
    const QList<QString> error_list = [&]() {
        QList<QString> out;

        for (const auto &message : messages) {
            if (message.type() == AdMessageType_Error) {
                out.append(message.text());
            }
        }

        return out;
    }();

    error_log(error_list, parent);
}

void ad_error_log(const AdInterface &ad, QWidget *parent) {
    const QList<AdMessage> messages = ad.messages();

    ad_error_log(messages, parent);
}

void error_log(const QList<QString> error_list, QWidget *parent) {
    if (error_list.isEmpty()) {
        return;
    }

    auto error_log_dialog = new ErrorLogDialog(parent);

    const QString errors_text = error_list.join("\n");
    error_log_dialog->set_text(errors_text);
    error_log_dialog->open();
}
