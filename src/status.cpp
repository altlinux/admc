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

Status::Status(QStatusBar *status_bar_arg, QTextEdit *status_log_arg, QObject *parent)
: QObject(parent)
{
    status_bar = status_bar_arg;
    status_log = status_log_arg;

    add_message(tr("Ready"));

    const BoolSetting *show_status_log_setting = Settings::instance()->bool_setting(BoolSettingType_ShowStatusLog);
    connect(
        show_status_log_setting, &BoolSetting::changed,
        this, &Status::on_toggle_show_status_log);
    connect(
        AdInterface::instance(), &AdInterface::message,
        this, &Status::add_message);
}

void Status::on_toggle_show_status_log() {
    const bool show_status_log = Settings::instance()->get_bool(BoolSettingType_ShowStatusLog);

    if (show_status_log) {
        status_log->setVisible(true); 
    } else {
        status_log->setVisible(false); 
    }
}

void Status::add_message(const QString &msg) {
    status_bar->showMessage(msg);

    const QColor original_color = status_log->textColor();
    QColor color = original_color;
    if (msg.contains("Failed")) {
        color = Qt::red;
    } else {
        color = Qt::darkGreen;
    }

    status_log->setTextColor(color);
    status_log->append(msg);
    status_log->setTextColor(original_color);

    // Scroll text edit to the newest message
    status_log->ensureCursorVisible();
}
