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

#include "attribute_edits/logon_hours_dialog.h"
#include "attribute_edits/ui_logon_hours_dialog.h"

#include "ad_utils.h"
#include "settings.h"

#include <QDateTime>
#include <QStandardItemModel>
#include <QTimeZone>

QList<bool> shift_list(const QList<bool> &list, const int shift_amount);

LogonHoursDialog::LogonHoursDialog(const QByteArray &value, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::LogonHoursDialog();
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

    const QList<QString> horizontalheader_labels = []() {
        QList<QString> out;

        for (int i = 0; i < HOURS_IN_DAY; i++) {
            const QString label = QString::number(i);
            out.append(label);
        }

        return out;
    }();
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

    settings_setup_dialog_geometry(SETTING_logon_hours_dialog_geometry, this);

    const QString allowed_style_sheet = [&]() {
        const QPalette palette = ui->view->palette();
        const QColor color = palette.highlight().color();
        const QString out = QString("background-color: rgb(%1, %2, %3);").arg(QString::number(color.red()), QString::number(color.green()), QString::number(color.blue()));

        return out;
    }();
    ui->legend_allowed->setStyleSheet(allowed_style_sheet);

    const QString denied_style_sheet = [&]() {
        const QPalette palette = ui->view->palette();
        const QColor color = palette.base().color();
        const QString out = QString("background-color: rgb(%1, %2, %3);").arg(QString::number(color.red()), QString::number(color.green()), QString::number(color.blue()));

        return out;
    }();
    ui->legend_denied->setStyleSheet(denied_style_sheet);

    connect(
        ui->local_time_button, &QRadioButton::toggled,
        this, &LogonHoursDialog::on_local_time_button_toggled);
}

LogonHoursDialog::~LogonHoursDialog() {
    delete ui;
}

void LogonHoursDialog::load(const QByteArray &value) {
    ui->view->clearSelection();

    original_value = value;

    const QList<QList<bool>> bools = logon_hours_to_bools(value, get_offset());

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

QByteArray LogonHoursDialog::get() const {
    const QList<QList<bool>> bools = [&]() {
        QList<QList<bool>> out = logon_hours_to_bools(QByteArray(LOGON_HOURS_SIZE, '\0'));

        const QList<QModelIndex> selected = ui->view->selectionModel()->selectedIndexes();

        for (const QModelIndex &index : selected) {
            const int day = index.row();
            const int h = index.column();

            out[day][h] = true;
        }

        return out;
    }();

    const QList<QList<bool>> original_bools = logon_hours_to_bools(original_value);

    // NOTE: input has to always be equal to output.
    // Therefore, for the case where original value was
    // unset, we need this special logic so that input
    // doesn't change to a non-empty bytearray.
    if (bools == original_bools) {
        return original_value;
    } else {
        const QByteArray out = logon_hours_to_bytes(bools, get_offset());

        return out;
    }
}

// Get current value, change time state and reload value
void LogonHoursDialog::on_local_time_button_toggled(bool checked) {
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

int LogonHoursDialog::get_offset() const {
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
    const QByteArray byte_list = [&]() {
        if (byte_list_arg.size() == LOGON_HOURS_SIZE) {
            return byte_list_arg;
        } else {
            return QByteArray(LOGON_HOURS_SIZE, (char) 0xFF);
        }
    }();

    // Convet byte array to list of bools
    const QList<bool> joined = [&]() {
        QList<bool> out;

        for (const char byte : byte_list) {
            for (int bit_i = 0; bit_i < 8; bit_i++) {
                const int bit = (0x01 << bit_i);
                const bool is_set = bitmask_is_set((int) byte, bit);
                out.append(is_set);
            }
        }

        out = shift_list(out, time_offset);

        return out;
    }();

    // Split the list into sublists for each day
    const QList<QList<bool>> out = [&]() {
        QList<QList<bool>> out_the;

        for (int i = 0; i < joined.size(); i += HOURS_IN_DAY) {
            const QList<bool> day_list = joined.mid(i, HOURS_IN_DAY);
            out_the.append(day_list);
        }

        return out_the;
    }();

    return out;
}

QByteArray logon_hours_to_bytes(const QList<QList<bool>> bool_list, const int time_offset) {
    const QList<bool> joined = [&]() {
        QList<bool> out;

        for (const QList<bool> &sublist : bool_list) {
            out += sublist;
        }

        out = shift_list(out, -time_offset);

        return out;
    }();

    const QByteArray out = [&]() {
        QByteArray bytes;

        for (int i = 0; i * 8 < joined.size(); i++) {
            const QList<bool> byte_list = joined.mid(i * 8, 8);

            int byte = 0;
            for (int bit_i = 0; bit_i < 8; bit_i++) {
                const int bit = (0x01 << bit_i);
                byte = bitmask_set(byte, bit, byte_list[bit_i]);
            }

            bytes.append(byte);
        }

        return bytes;
    }();

    return out;
}

QList<bool> shift_list(const QList<bool> &list, const int shift_amount) {
    if (abs(shift_amount) > list.size()) {
        return list;
    }

    QList<bool> out;

    for (int i = 0; i < list.size(); i++) {
        const int shifted_i = [&]() {
            int out_i = i - shift_amount;

            if (out_i < 0) {
                out_i += list.size();
            } else if (out_i >= list.size()) {
                out_i -= list.size();
            }

            return out_i;
        }();

        out.append(list[shifted_i]);
    }

    return out;
}
