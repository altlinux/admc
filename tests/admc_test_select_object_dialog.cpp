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

#include "admc_test_select_object_dialog.h"

#include "admc_test_select_base_widget.h"
#include "console_impls/object_impl.h"
#include "filter_widget/select_base_widget.h"
#include "select_dialogs/select_object_dialog.h"
#include "select_dialogs/select_object_match_dialog.h"
#include "ui_select_object_dialog.h"
#include "ui_select_object_match_dialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTreeView>

void ADMCTestSelectObjectDialog::init() {
    ADMCTest::init();

    dialog = new SelectObjectDialog({CLASS_USER}, SelectObjectDialogMultiSelection_Yes, parent_widget);
    dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    SelectBaseWidget *select_base_widget = dialog->ui->select_base_widget;
    select_base_widget_add(select_base_widget, test_arena_dn());

    edit = dialog->ui->name_edit;
    add_button = dialog->ui->add_button;
}

void ADMCTestSelectObjectDialog::empty() {
    const QList<QString> selected = dialog->get_selected();
    QVERIFY(selected.isEmpty());
}

void ADMCTestSelectObjectDialog::no_matches() {
    const QString dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    edit->setText("no-match");

    add_button->click();

    close_message_box();

    const QList<QString> selected = dialog->get_selected();
    QVERIFY(selected.isEmpty());
}

void ADMCTestSelectObjectDialog::one_match() {
    const QString dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    edit->setText(TEST_USER);
    add_button->click();

    const QList<QString> selected = dialog->get_selected();
    QCOMPARE(selected.size(), 1);
    if (selected.size() == 1) {
        QCOMPARE(selected[0], dn);
    }
}

void ADMCTestSelectObjectDialog::multiple_matches() {
    const QString dn1 = test_object_dn(QString(TEST_USER) + "1", CLASS_USER);
    const bool create_success1 = ad.object_add(dn1, CLASS_USER);
    QVERIFY(create_success1);

    const QString dn2 = test_object_dn(QString(TEST_USER) + "2", CLASS_USER);
    const bool create_success2 = ad.object_add(dn2, CLASS_USER);
    QVERIFY(create_success2);

    select_object_in_multi_match_dialog(TEST_USER, dn2);

    const QList<QString> selected = dialog->get_selected();
    QCOMPARE(selected, QList<QString>({dn2}));
}

// Adding same object two times should open message box
// warning about duplicate and also not add it twice
void ADMCTestSelectObjectDialog::one_match_duplicate() {
    const QString dn = test_object_dn(TEST_USER, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    edit->setText(TEST_USER);
    add_button->click();

    const QList<QString> selected_first = dialog->get_selected();
    QCOMPARE(selected_first, QList<QString>({dn}));

    edit->setText(TEST_USER);

    add_button->click();

    close_message_box();

    const QList<QString> selected_second = dialog->get_selected();
    QCOMPARE(selected_second, QList<QString>({dn}));
}

// Duplicates should also be processed for multiple match
// case
void ADMCTestSelectObjectDialog::multiple_match_duplicate() {
    const QString dn1 = test_object_dn(QString(TEST_USER) + "1", CLASS_USER);
    const bool create_success1 = ad.object_add(dn1, CLASS_USER);
    QVERIFY(create_success1);

    const QString dn2 = test_object_dn(QString(TEST_USER) + "2", CLASS_USER);
    const bool create_success2 = ad.object_add(dn2, CLASS_USER);
    QVERIFY(create_success2);

    select_object_in_multi_match_dialog(TEST_USER, dn2);

    const QList<QString> selected_first = dialog->get_selected();
    QCOMPARE(selected_first, QList<QString>({dn2}));

    select_object_in_multi_match_dialog(TEST_USER, dn2);

    const QList<QString> selected_second = dialog->get_selected();
    QCOMPARE(selected_second, QList<QString>({dn2}));
}

void ADMCTestSelectObjectDialog::select_object_in_multi_match_dialog(const QString &name, const QString &dn) {
    edit->setText(name);

    add_button->click();

    auto match_dialog = dialog->findChild<SelectObjectMatchDialog *>();
    QVERIFY(match_dialog);

    QTreeView *match_dialog_view = match_dialog->ui->view;

    wait_for_find_results_to_load(match_dialog_view);

    navigate_until_object(match_dialog_view, dn, ObjectRole_DN);

    match_dialog->accept();

    close_message_box();
}

QTEST_MAIN(ADMCTestSelectObjectDialog)
