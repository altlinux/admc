/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "logon_hours_dialog.h"

#include "ad_utils.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QDebug>

#define DAYS_IN_WEEK 7
#define HOURS_IN_DAY 24

LogonHoursDialog::LogonHoursDialog(QWidget *parent)
: QDialog(parent) {
    model = new QStandardItemModel(DAYS_IN_WEEK, HOURS_IN_DAY, this);
    model->setVerticalHeaderLabels({
        tr("Monday"),
        tr("Tuesday"),
        tr("Wednesday"),
        tr("Thursday"),
        tr("Friday"),
        tr("Saturday"),
        tr("Sunday"),
    });

    view = new QTableView();
    view->setModel(model);
    view->setSelectionMode(QAbstractItemView::MultiSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->horizontalHeader()->setHighlightSections(false);
    view->verticalHeader()->setHighlightSections(false);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    for (int col = 0; col < model->columnCount(); col++) {
        view->setColumnWidth(col, 5);
    }

    auto button_box = new QDialogButtonBox();
    button_box->addButton(QDialogButtonBox::Ok);
    button_box->addButton(QDialogButtonBox::Cancel);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &LogonHoursDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &LogonHoursDialog::reject);
}

void LogonHoursDialog::load(const QByteArray &value) {
    original_value = value;

    view->clearSelection();

    // NOTE: value may be empty if it's undefined
    if (value.size() != LOGON_HOURS_SIZE) {
        return;
    }

    const QList<QList<bool>> bools = logon_hours_to_bools(value);

    for (int day = 0; day < 7; day++) {
        for (int h = 0; h < 24; h++) {
            const bool selected = bools[day][h];

            if (selected) {
                // TODO: day shift?
                const int row = day;
                // TODO: timezone shift
                const int column = h;
                const QModelIndex index = model->index(row, column);
                view->selectionModel()->select(index, QItemSelectionModel::Select);
            }
        }
    }
}

QByteArray LogonHoursDialog::get() const {
    const QList<QList<bool>> bools = [&]() {
        QList<QList<bool>> out = logon_hours_to_bools(QByteArray(LOGON_HOURS_SIZE, '\0'));

        const QList<QModelIndex> selected = view->selectionModel()->selectedIndexes();

        for (const QModelIndex &index : selected) {
            const int day = index.row();
            const int h = index.column();

            out[day][h] = true;
        }

        return out;
    }();

    const QByteArray out = logon_hours_to_bytes(bools);

    return out;
}

void LogonHoursDialog::accept() {


    QDialog::accept();
}

void LogonHoursDialog::reject() {
    load(original_value);

    QDialog::reject();
}

QList<QList<bool>> logon_hours_to_bools(const QByteArray &byte_list) {
    // Convet byte array to list of bools
    const QList<bool> joined = [&]() {
        QList<bool> out;

        for (const char byte : byte_list) {
            for (int bit_i = 0; bit_i < 8; bit_i++) {
                const int bit = (0x01 << bit_i);
                const bool is_set = bit_is_set((int) byte, bit);
                out.append(is_set);
            }
        }

        return out;
    }();

    // Split the list into sublists for each day (24 hours)
    const QList<QList<bool>> out = [&]() {
        QList<QList<bool>> out_the;

        for (int i = 0; i < joined.size(); i += 24) {
            const QList<bool> day_list = joined.mid(i, 24);
            out_the.append(day_list);
        }

        return out_the;        
    }();

    return out;
}

QByteArray logon_hours_to_bytes(const QList<QList<bool>> bool_list) {
    const QList<bool> joined = [&]() {
        QList<bool> out;

        for (const QList<bool> &sublist : bool_list) {
            out += sublist;
        }

        return out;
    }();

    const QByteArray out = [&]() {
        QByteArray bytes;

        for (int i = 0; i * 8 < joined.size(); i++) {
            const QList<bool> byte_list = joined.mid(i * 8, 8);

            int byte = 0;
            for (int bit_i = 0; bit_i < 8; bit_i++) {
                const int bit = (0x01 << bit_i);
                byte = bit_set(byte, bit, byte_list[bit_i]);
            }

            bytes.append(byte);
        }

        return bytes;
    }();

    return out;
}
