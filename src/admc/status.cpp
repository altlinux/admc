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

#include "status.h"

#include "ad_interface.h"

#include <QStatusBar>
#include <QTextEdit>
#include <QDialog>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QCoreApplication>

#define MAX_MESSAGES_IN_LOG 200

Status *Status::instance() {
    static Status status;
    return &status;
}

Status *STATUS() {
    return Status::instance();
}

Status::Status() {
    status_bar = new QStatusBar();
    status_log = new QTextEdit();
    status_log->setReadOnly(true);
}

void Status::add_message(const QString &msg, const StatusType &type) {
    status_bar->showMessage(msg);
    
    const QColor color =
    [type]() {
        switch (type) {
            case StatusType_Success: return Qt::darkGreen;
            case StatusType_Error: return Qt::red;
        }
        return Qt::black;
    }();

    const QColor original_color = status_log->textColor();
    status_log->setTextColor(color);
    status_log->append(msg);
    status_log->setTextColor(original_color);

    // Limit number of messages in log by deleting old ones
    // once over limit
    QTextCursor cursor = status_log->textCursor();
    const int message_count = cursor.blockNumber();
    if (message_count > MAX_MESSAGES_IN_LOG) {
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 0);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    }

    // Move cursor to newest message
    QTextCursor end_cursor = status_log->textCursor();
    end_cursor.movePosition(QTextCursor::End);
    status_log->setTextCursor(end_cursor);
}

void Status::display_ad_messages(const AdInterface &ad, QWidget *parent) {
    const QList<AdMessage> messages = ad.messages();

    if (messages.isEmpty()) {
        return;
    }

    //
    // Display all messages in status log
    //
    for (const auto &message : messages) {
        const StatusType status_type =
        [message]() {
            switch (message.type()) {
                case AdMessageType_Success: return StatusType_Success;
                case AdMessageType_Error: return StatusType_Error;
            }
            return StatusType_Success;
        }();

        add_message(message.text(), status_type);
    }

    //
    // Display all error messages in error log
    //
    if (ad.any_error_messages()) {
        auto dialog = new QDialog(parent);
        dialog->setWindowTitle(QCoreApplication::translate("Status", "Errors occured"));
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setMinimumWidth(600);

        const QString errors_text =
        [messages]() {
            QList<QString> errors;

            for (const auto &message : messages) {
                if (message.type() == AdMessageType_Error) {
                    errors.append(message.text());
                }
            }
            
            return errors.join("\n");
        }();

        auto errors_display = new QPlainTextEdit();
        errors_display->setPlainText(errors_text);

        auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok);

        auto layout = new QVBoxLayout();
        dialog->setLayout(layout);
        layout->addWidget(errors_display);
        layout->addWidget(button_box);

        QObject::connect(
            button_box, &QDialogButtonBox::accepted,
            dialog, &QDialog::accept);

        dialog->open();
    }
}
