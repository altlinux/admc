/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "attribute_edits/schedule_hours_dialog.h"
#include "attribute_edits/ui_schedule_hours_dialog.h"

#include "ad_utils.h"
#include "core/settings.h"

#include <QDateTime>
#include <QStandardItemModel>
#include <QTimeZone>

QList<bool> shift_list(const QList<bool> &list, const int shift_amount);

ScheduleHoursDialog::ScheduleHoursDialog(const QByteArray &value, QWidget *parent, ScheduleType type_arg)
: QDialog(parent), type(type_arg) {
    ui = new Ui::ScheduleHoursDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    model = new QStandardItemModel(DAYS_IN_WEEK, HOURS_IN_DAY, this);
    model->setVerticalHeaderLabels({
        tr("Sunday"),
        tr("Monday"),
        tr("Tuesday"),
        tr("Wednesday"),
        tr("Thursday"),
        tr("Friday"),
        tr("Saturday"),
    });

    QList<QString> horizontalheader_labels;
    for (int i = 0; i < HOURS_IN_DAY; i++) {
        const QString label = QString::number(i);
        horizontalheader_labels.append(label);
    }

    model->setHorizontalHeaderLabels(horizontalheader_labels);

    ui->view->setModel(model);
    ui->view->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    for (int col = 0; col < model->columnCount(); col++) {
        ui->view->setColumnWidth(col, 5);
    }

    ui->local_time_button->setChecked(true);
    is_local_time = true;

    load(value);

    settings_setup_dialog_geometry(SETTING_schedule_hours_dialog_geometry,
                                   this);

    const QPalette palette = ui->view->palette();
    QColor color = palette.highlight().color();
    const QString allowed_style_sheet =
        QString("background-color: rgb(%1, %2, %3);").arg(
            QString::number(color.red()),
            QString::number(color.green()),
            QString::number(color.blue()));

    ui->legend_allowed->setStyleSheet(allowed_style_sheet);

    color = palette.base().color();
    const QString denied_style_sheet =
        QString("background-color: rgb(%1, %2, %3);").arg(
            QString::number(color.red()),
            QString::number(color.green()),
            QString::number(color.blue()));

    ui->legend_denied->setStyleSheet(denied_style_sheet);

    connect(
        ui->local_time_button, &QRadioButton::toggled,
        this, &ScheduleHoursDialog::on_local_time_button_toggled);
}

ScheduleHoursDialog::~ScheduleHoursDialog() {
    delete ui;
}

void ScheduleHoursDialog::load(const QByteArray &value) {
    ui->view->clearSelection();

    if (type == ScheduleType_SiteLink) {
        if (value.isEmpty() || value.size() < SITE_LINK_SCHEDULE_TOTAL_SIZE) {
            // Create default schedule structure
            const QByteArray header = QByteArray((const char*)sitelink_schedule_header_bytes, 20);
            original_value = header + QByteArray(SITE_LINK_SCHEDULE_DATA_SIZE, (char)0xFF);
        } else {
            original_value = value;
        }
    } else {
        original_value = value;
    }

    QList<QList<bool>> bools;
    if (type == ScheduleType_SiteLink) {
        const QByteArray payload = original_value.mid(SITE_LINK_SCHEDULE_HEADER_SIZE, SITE_LINK_SCHEDULE_DATA_SIZE);
        bools = site_schedule_bytes_to_bools(payload, get_offset());
    } else {
        bools = logon_hours_to_bools(original_value, get_offset());
    }

    for (int day = 0; day < DAYS_IN_WEEK; day++) {
        for (int h = 0; h < HOURS_IN_DAY; h++) {
            const bool selected = bools[day][h];

            if (selected) {
                const int row = day;
                const int column = h;
                const QModelIndex index = model->index(row, column);
                ui->view->selectionModel()->select(index, QItemSelectionModel::Select);
            }
        }
    }
}

QByteArray ScheduleHoursDialog::get() const {
    const QList<QModelIndex> selected =
        ui->view->selectionModel()->selectedIndexes();

    QList<QList<bool>> bools =
        (type == ScheduleType_SiteLink) ?
        site_schedule_bytes_to_bools(
            QByteArray(SITE_LINK_SCHEDULE_DATA_SIZE, '\0'), 0) :
        logon_hours_to_bools(QByteArray(LOGON_HOURS_SIZE, '\0'));

    for (const QModelIndex &index : selected) {
        const int day = index.row();
        const int h = index.column();
        bools[day][h] = true;
    }

    // Get original bools for comparison
    QList<QList<bool>> original_bools;
    if (type == ScheduleType_SiteLink) {
        const QByteArray payload = original_value.mid(SITE_LINK_SCHEDULE_HEADER_SIZE, SITE_LINK_SCHEDULE_DATA_SIZE);
        original_bools = site_schedule_bytes_to_bools(payload, get_offset());
    } else {
        original_bools = logon_hours_to_bools(original_value, get_offset());
    }

    // NOTE: input has to always be equal to output.
    // Therefore, for the case where original value was
    // unset, we need this special logic so that input
    // doesn't change to a non-empty bytearray.
    if (bools == original_bools) {
        return original_value;
    } else {
        if (type == ScheduleType_SiteLink) {
            // Convert to site schedule bytes
            const QByteArray schedule_data = bools_to_site_schedule_bytes(bools, get_offset());

            QByteArray result;
            // Add header (20 bytes) - always the same
            result.append(QByteArray((const char*)sitelink_schedule_header_bytes, 20));
            // Add schedule data (168 bytes)
            result.append(schedule_data);

            return result;
        } else {
            // logonHours format
            const QByteArray hours_bytes = logon_hours_to_bytes(bools, get_offset());
            return hours_bytes;
        }
    }
}

// Get current value, change time state and reload value
void ScheduleHoursDialog::on_local_time_button_toggled(bool checked) {
    const QByteArray current_value = get();

    // NOTE: important to change state after get() call so
    // current_value is in correct format
    is_local_time = checked;

    load(current_value);
}

int get_current_utc_offset() {
    const QDateTime current_datetime = QDateTime::currentDateTime();
    const int offset_s = QTimeZone::systemTimeZone().offsetFromUtc(current_datetime);
    const int offset_h = offset_s / 60 / 60;

    return offset_h;
}

int ScheduleHoursDialog::get_offset() const {
    if (is_local_time) {
        return get_current_utc_offset();
    } else {
        return 0;
    }
}

QList<QList<bool>> logon_hours_to_bools(const QByteArray &byte_list_arg, const int time_offset) {
    // NOTE: value may be empty or malformed. In that
    // case treat both as values that "allow all logon
    // times" (all bits set). This also handles the
    // case where value is unset and we need to treat
    // it as "allow all logon times".
    QByteArray byte_list;
    if (byte_list_arg.size() == LOGON_HOURS_SIZE) {
        // logonHours format (21 bytes)
        byte_list = byte_list_arg;
    } else {
        // Invalid or empty - allow all
        byte_list = QByteArray(LOGON_HOURS_SIZE, (char) 0xFF);
    }

    // Convet byte array to list of bools
    QList<bool> joined;
    for (const char byte : byte_list) {
        for (int bit_i = 0; bit_i < 8; bit_i++) {
            const int bit = (0x01 << bit_i);
            const bool is_set = bitmask_is_set((int) byte, bit);
            joined.append(is_set);
        }
    }
    joined = shift_list(joined, time_offset);;

    // Split the list into sublists for each day
    QList<QList<bool>> result;
    for (int i = 0; i < joined.size(); i += HOURS_IN_DAY) {
        const QList<bool> day_list = joined.mid(i, HOURS_IN_DAY);
        result.append(day_list);
    }

    return result;
}

QByteArray logon_hours_to_bytes(const QList<QList<bool>> bool_list, const int time_offset) {
    QList<bool> joined;
    for (const QList<bool> &sublist : bool_list) {
        joined += sublist;
    }

    joined = shift_list(joined, -time_offset);

    QByteArray result;
    for (int i = 0; i * 8 < joined.size(); i++) {
        const QList<bool> byte_list = joined.mid(i * 8, 8);

        int byte = 0;
        for (int bit_i = 0; bit_i < 8; bit_i++) {
            const int bit = (0x01 << bit_i);
            byte = bitmask_set(byte, bit, byte_list[bit_i]);
        }

        result.append(byte);
    }

    return result;
}

QList<bool> shift_list(const QList<bool> &list, const int shift_amount) {
    if (abs(shift_amount) > list.size()) {
        return list;
    }

    QList<bool> result;

    for (int i = 0; i < list.size(); i++) {
        int shifted_i = i - shift_amount;

        if (shifted_i < 0) {
            shifted_i += list.size();
        } else if (shifted_i >= list.size()) {
            shifted_i -= list.size();
        }

        result.append(list[shifted_i]);
    }

    return result;
}

// Convert schedule attribute (byte-per-hour format) to bool grid
QList<QList<bool>> site_schedule_bytes_to_bools(const QByteArray &byte_list_arg, const int time_offset) {
    // NOTE: value may be empty or malformed. In that case treat
    // as "allow all" (all bytes set to 0xFF)
    const QByteArray byte_list = byte_list_arg.size() == SITE_LINK_SCHEDULE_DATA_SIZE ?
                byte_list_arg : QByteArray(SITE_LINK_SCHEDULE_DATA_SIZE, SITE_LINK_SCHEDULE_ALLOWED);

    // Convert byte array to list of bools (each byte = 1 hour)
    QList<bool> joined;
    for (const char byte : byte_list) {
        // Consider hour "allowed" if byte is 0xFF (or anything except denied marker)
        const bool is_allowed = ((unsigned char)byte == (unsigned char)SITE_LINK_SCHEDULE_ALLOWED);
        joined.append(is_allowed);
    }

    joined = shift_list(joined, time_offset);

    // Split the list into sublists for each day
    QList<QList<bool>> out;
    for (int i = 0; i < joined.size(); i += HOURS_IN_DAY) {
        const QList<bool> day_list = joined.mid(i, HOURS_IN_DAY);
        out.append(day_list);
    }

    return out;
}

QByteArray bools_to_site_schedule_bytes(const QList<QList<bool> > bool_list, const int time_offset) {
    QList<bool> joined;
    for (const QList<bool> &sublist : bool_list) {
        joined += sublist;
    }

    joined = shift_list(joined, -time_offset);

    QByteArray bytes;
    // Each bool becomes one byte (not one bit like in user logon schedule)
    for (bool is_allowed : joined) {
        bytes.append(is_allowed ? SITE_LINK_SCHEDULE_ALLOWED : SITE_LINK_SCHEDULE_DENIED);
    }

    return bytes;
}
