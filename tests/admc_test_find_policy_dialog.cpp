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

#include "admc_test_find_policy_dialog.h"

#include "find_widgets/find_policy_dialog.h"
#include "find_widgets/find_policy_dialog_p.h"
#include "ui_find_policy_dialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

void ADMCTestFindPolicyDialog::init() {
    ADMCTest::init();

    ConsoleWidget *console = new ConsoleWidget(parent_widget);

    dialog = new FindPolicyDialog(console, parent_widget);
}

void ADMCTestFindPolicyDialog::add_filter_data() {
    QTest::addColumn<int>("search_item");
    QTest::addColumn<int>("condition");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("expected_filter_display");
    QTest::addColumn<QString>("expected_filter");

    // clang-format off
    QTest::newRow("1") << (int) SearchItem_Name << (int) Condition_Contains << "test" << "Name Contains: \"test\"" << "(displayName=*test*)";
    QTest::newRow("2") << (int) SearchItem_GUID << (int) Condition_Contains << "test2" << "GUID Contains: \"test2\"" << "(cn=*test2*)";
    QTest::newRow("3") << (int) SearchItem_GUID << (int) Condition_Equals << "{guid}" << "GUID Is (exactly): \"{guid}\"" << "(cn={guid})";
    // clang-format on
}

void ADMCTestFindPolicyDialog::add_filter() {
    QFETCH(int, search_item);
    QFETCH(int, condition);
    QFETCH(QString, value);
    QFETCH(QString, expected_filter_display);
    QFETCH(QString, expected_filter);

    QComboBox *search_item_combo = dialog->ui->search_item_combo;
    QComboBox *condition_combo = dialog->ui->condition_combo;
    QLineEdit *value_edit = dialog->ui->value_edit;
    QPushButton *add_button = dialog->ui->add_button;
    QListWidget *filter_list = dialog->ui->filter_list;

    search_item_combo->setCurrentIndex(search_item);

    const int condition_index = [&]() {
        for (int i = 0; i < condition_combo->count(); i++) {
            const int data = condition_combo->itemData(i, Qt::UserRole).toInt();
            if (data == condition) {
                return i;
            }
        }

        return -1;
    }();

    condition_combo->setCurrentIndex(condition_index);

    value_edit->setText(value);

    add_button->click();

    QCOMPARE(filter_list->count(), 1);

    QListWidgetItem *filter_item = filter_list->item(0);

    const QString filter_display = filter_item->text();
    QCOMPARE(filter_display, expected_filter_display);

    const QString filter = filter_item->data(Qt::UserRole).toString();
    QCOMPARE(filter, expected_filter);
}

QTEST_MAIN(ADMCTestFindPolicyDialog)
