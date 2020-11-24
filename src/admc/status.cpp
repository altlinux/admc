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
#include "settings.h"

#include <QStatusBar>
#include <QTextEdit>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

Status *Status::instance() {
    static Status instance;
    return &instance;
}

void Status::init(QStatusBar *status_bar_arg, QTextEdit *status_log_arg) {
    if (initialized) {
        printf("ERROR: double init of Status!\n");
        return;
    } else {
        initialized = true;
    }

    status_bar = status_bar_arg;
    status_log = status_log_arg;

    const BoolSettingSignal *show_status_log_setting = SETTINGS()->get_bool_signal(BoolSetting_ShowStatusLog);
    connect(
        show_status_log_setting, &BoolSettingSignal::changed,
        this, &Status::on_toggle_show_status_log);
    on_toggle_show_status_log();
}

void Status::on_toggle_show_status_log() {
    const bool show_status_log = SETTINGS()->get_bool(BoolSetting_ShowStatusLog);

    if (show_status_log) {
        status_log->setVisible(true); 
    } else {
        status_log->setVisible(false); 
    }
}

// TODO: probably want to cap total amount of message and delete old ones?
void Status::message(const QString &msg, const StatusType &type) {
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

    // Scroll text edit to the newest message
    status_log->ensureCursorVisible();

    if (type == StatusType_Error) {
        errors.append(msg);
    }
}

int Status::get_errors_size() const {
    return errors.size();
}

// TODO: make size fit the contents of error log?
void Status::show_errors_popup(int starting_index) {
    const bool no_errors = (starting_index >= errors.size());
    if (no_errors) {
        return;
    }

    // NOTE: message box is too small and non-resizable
    QDialog dialog;
    dialog.resize(400, 400);
    dialog.setModal(true);

    auto label = new QLabel(tr("Errors occured:"));

    auto error_log = new QTextEdit();
    error_log->setReadOnly(true);

    for (int i = starting_index; i < errors.size(); i++) {
        const QString error = errors[i];
        error_log->append(error);
    }

    const auto layout = new QVBoxLayout();
    dialog.setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(error_log);

    dialog.exec();
}
