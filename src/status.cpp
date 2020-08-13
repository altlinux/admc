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
#include "settings.h"

#include <QStatusBar>
#include <QTextEdit>
#include <QAction>

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

    status_bar->showMessage(tr("Ready"));

    const BoolSettingSignal *show_status_log_setting = Settings::instance()->get_bool_signal(BoolSetting_ShowStatusLog);
    connect(
        show_status_log_setting, &BoolSettingSignal::changed,
        this, &Status::on_toggle_show_status_log);
    on_toggle_show_status_log();
}

void Status::on_toggle_show_status_log() {
    const bool show_status_log = Settings::instance()->get_bool(BoolSetting_ShowStatusLog);

    if (show_status_log) {
        status_log->setVisible(true); 
    } else {
        status_log->setVisible(false); 
    }
}

void Status::message(const QString &msg, const StatusType &type) {
    auto get_color =
    [type]() {
        switch (type) {
            case StatusType_Success: return Qt::darkGreen;
            case StatusType_Error: return Qt::red;
        }
        return Qt::black;
    };
    QColor color = get_color();

    const QColor original_color = status_log->textColor();
    status_log->setTextColor(color);
    status_log->append(msg);
    status_log->setTextColor(original_color);

    // Scroll text edit to the newest message
    status_log->ensureCursorVisible();   
}
