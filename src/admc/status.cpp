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

void Status::message(const QString &msg, const StatusType &type) {
    if (type == StatusType_Error) {
        error_log.append(msg);
    }

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

    if (print_errors && type == StatusType_Error) {
        qInfo() << msg;
    }
}

void Status::start_error_log() {
    error_log.clear();
}

void Status::end_error_log(QWidget *parent) {
    if (error_log.isEmpty()) {
        return;
    }

    auto dialog = new QDialog(parent);
    dialog->setWindowTitle(QCoreApplication::translate("Status", "Errors occured"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMinimumWidth(600);

    auto log = new QPlainTextEdit();
    const QString text = error_log.join("\n");
    log->setPlainText(text);

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok);

    auto layout = new QVBoxLayout();
    dialog->setLayout(layout);
    layout->addWidget(log);
    layout->addWidget(button_box);

    QObject::connect(
        button_box, &QDialogButtonBox::accepted,
        dialog, &QDialog::accept);

    dialog->open();
}

void Status::display_ad_messages(const AdInterface &ad, QWidget *parent) {
    const QList<AdMessage> messages = ad.messages();

    if (messages.isEmpty()) {
        return;
    }

    //
    // Display last message in status bar
    //
    const AdMessage last_message = messages.last();
    status_bar->showMessage(last_message.text());

    //
    // Display all messages in status log
    //
    const QColor original_status_log_color = status_log->textColor();

    for (const auto &message : messages) {
        const QColor color =
        [message]() {
            switch (message.type()) {
                case AdMessageType_Success: return Qt::darkGreen;
                case AdMessageType_Error: return Qt::red;
            }
            return Qt::black;
        }();

        status_log->setTextColor(color);
        status_log->append(message.text());
    }

    status_log->setTextColor(original_status_log_color);

    //
    // Display all error messages in error log
    //
    const bool any_errors =
    [messages]() {
        for (const auto &message : messages) {
            if (message.type() == AdMessageType_Error) {
                return true;
            }
        }

        return false;
    }();

    if (any_errors) {
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
