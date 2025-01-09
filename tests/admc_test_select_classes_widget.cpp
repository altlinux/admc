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

#include "admc_test_select_classes_widget.h"

#include "filter_widget/class_filter_dialog.h"
#include "filter_widget/select_classes_widget.h"
#include "filter_widget/ui_class_filter_dialog.h"
#include "filter_widget/ui_select_classes_widget.h"

void ADMCTestSelectClassesWidget::init() {
    ADMCTest::init();

    select_classes_widget = new SelectClassesWidget();
    select_classes_widget->set_classes(filter_classes, filter_classes);
    select_classes_widget->enable_filtering_all_classes();
    add_widget(select_classes_widget);

    select_button = select_classes_widget->ui->select_button;
    classes_display = select_classes_widget->ui->classes_display;
}

void ADMCTestSelectClassesWidget::test_foo_data() {
    QTest::addColumn<bool>("all_checked");
    QTest::addColumn<QList<QString>>("specific_classes_checked");
    QTest::addColumn<QString>("correct_display_string");
    QTest::addColumn<QString>("correct_filter");

    // clang-format off
    QTest::newRow("all only") << true << QList<QString>() << "All" << "";
    // NOTE: all should override other checkboxes, even
    // if they are checked
    QTest::newRow("all + some specific") << true << QList<QString>({"User", "Group"}) << "All" << "";
    QTest::newRow("User") << false << QList<QString>({"User"}) << "User" << "(objectClass=user)";
    QTest::newRow("User + Group") << false << QList<QString>({"User", "Group"}) << "Group, User" << "(|(objectClass=user)(objectClass=group))";
    // clang-format on
}

void ADMCTestSelectClassesWidget::test_foo() {
    QFETCH(bool, all_checked);
    QFETCH(QList<QString>, specific_classes_checked);
    QFETCH(QString, correct_display_string);
    QFETCH(QString, correct_filter);

    auto check_output_correctness = [&]() {
        QCOMPARE(classes_display->text(), correct_display_string);
        QCOMPARE(select_classes_widget->get_filter(), correct_filter);
    };

    auto get_class_checkbox_list = [](ClassFilterDialog *dialog) {
        ClassFilterWidget *class_filter_widget = dialog->ui->class_filter_widget;
        const QList<QCheckBox *> out = class_filter_widget->findChildren<QCheckBox *>();

        return out;
    };

    // Check correctness of input->output
    select_button->click();

    ClassFilterDialog *class_filter_dialog = select_classes_widget->findChild<ClassFilterDialog *>();
    QVERIFY(class_filter_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(class_filter_dialog, 1000));

    QCheckBox *all_checkbox = class_filter_dialog->ui->all_checkbox;
    all_checkbox->setChecked(all_checked);

    const QList<QCheckBox *> class_checkbox_list = get_class_checkbox_list(class_filter_dialog);

    for (QCheckBox *check : class_checkbox_list) {
        const bool match = specific_classes_checked.contains(check->text());

        check->setChecked(match);
    }

    class_filter_dialog->accept();

    check_output_correctness();

    // Check state saving/restoring
    const QVariant state = select_classes_widget->save_state();
    select_classes_widget->restore_state(state);

    check_output_correctness();

    // Check that reopening dialog retains state
    select_button->click();

    ClassFilterDialog *class_filter_dialog_2 = select_classes_widget->findChild<ClassFilterDialog *>();
    QVERIFY(class_filter_dialog_2);
    QVERIFY(QTest::qWaitForWindowExposed(class_filter_dialog_2, 1000));

    class_filter_dialog_2->accept();

    check_output_correctness();
}

QTEST_MAIN(ADMCTestSelectClassesWidget)
