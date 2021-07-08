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

#include "admc_test_logon_hours_dialog.h"

#include "logon_hours_dialog.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QRadioButton>

void ADMCTestLogonHoursDialog::init() {
    ADMCTest::init();

    dialog = new LogonHoursDialog(parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    // NOTE: use utc for 
    local_time_button = dialog->findChild<QRadioButton *>("local_time_button");
    utc_time_button = dialog->findChild<QRadioButton *>("utc_time_button");

    view = dialog->findChild<QTableView *>();
    QVERIFY(view != nullptr);

    model = dialog->findChild<QStandardItemModel *>();
    QVERIFY(model != nullptr);

    selection_model = view->selectionModel();
}

auto empty_day = []() {
    QList<bool> out;

    for (int i = 0; i < HOURS_IN_DAY; i++) {
        out.append(false);
    }

    return out;
};

auto empty_week = []() {
    QList<QList<bool>> out;

    for (int day = 0; day < DAYS_IN_WEEK; day++) {
        out.append(empty_day());
    }

    return out;
};

const QByteArray empty_bytes = QByteArray(LOGON_HOURS_SIZE, '\0');
const QByteArray test_bytes = []() {
    QByteArray out = empty_bytes;
    out[Weekday_Tuesday * 3] = 'a';

    return out;
}();
const QList<QList<bool>> test_bools = []() {
    QList<QList<bool>> out = empty_week();
    out[Weekday_Tuesday][0] = true;
    out[Weekday_Tuesday][5] = true;
    out[Weekday_Tuesday][6] = true;

    return out;
}();

void ADMCTestLogonHoursDialog::conversion_funs() {
    const QList<QList<bool>> converted_bools = logon_hours_to_bools(test_bytes);
    QVERIFY(converted_bools == test_bools);

    const QByteArray converted_bytes = logon_hours_to_bytes(test_bools);

    QVERIFY(converted_bytes == test_bytes);
}

void ADMCTestLogonHoursDialog::load() {
    utc_time_button->setChecked(true);
    
    dialog->load(test_bytes);

    const QList<QModelIndex> selected = selection_model->selectedIndexes();
    const QSet<QModelIndex> selected_set = selected.toSet();

    const QSet<QModelIndex> correct_selected_set = {
        model->index(2, 0),
        model->index(2, 5),
        model->index(2, 6),
    };

    QVERIFY(selected_set == correct_selected_set);
}

// Dialog should handle loading undefined value, where
// bytearray is empty
void ADMCTestLogonHoursDialog::load_undefined() {
    dialog->load(QByteArray());

    const QByteArray out = dialog->get();
    QVERIFY(out == empty_bytes);
}

void ADMCTestLogonHoursDialog::get_unchanged() {
    dialog->load(test_bytes);
    const QByteArray returned_bytes = dialog->get();
    QVERIFY(returned_bytes == test_bytes);
}

void ADMCTestLogonHoursDialog::handle_timezone() {
    const QByteArray bytes = [&]() {
        QByteArray out = empty_bytes;
        out[Weekday_Tuesday * 3] = 0x01;

        return out;
    }();
    dialog->load(bytes);

    auto test_selected = [&](const QList<QModelIndex> &correct_selected) {
    };

    // First do UTC
    utc_time_button->setChecked(true);

    const QList<QModelIndex> correct_utc_selected = {
        model->index(Weekday_Tuesday, 0),
    };
    const QList<QModelIndex> utc_selected = selection_model->selectedIndexes();
    QVERIFY(utc_selected == correct_utc_selected);

    // Then local time
    local_time_button->setChecked(true);

    const QList<QModelIndex> correct_local_selected = [&]() {
        // UTC rows
        int row = Weekday_Tuesday;
        int col = 0;

        // Apply timezone offset
        const int offset = get_current_utc_offset();
        col += offset;
        if (col < 0) {
            col += HOURS_IN_DAY;
            row--;
        } else if (col >= HOURS_IN_DAY) {
            col -= HOURS_IN_DAY;
            row++;
        }
    
        return QList<QModelIndex>({model->index(row, col)});
    }();
    const QList<QModelIndex> local_selected = selection_model->selectedIndexes();
    QVERIFY(local_selected == correct_local_selected);
}

QTEST_MAIN(ADMCTestLogonHoursDialog)
