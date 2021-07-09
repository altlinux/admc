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

#include "admc_test_filter_widget.h"

#include "filter_classes_widget.h"
#include "filter_widget/filter_builder.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/select_classes_widget.h"

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#define SIMPLE_TAB_INDEX 0
#define NORMAL_TAB_INDEX 1
#define ADVANCED_TAB_INDEX 2

void ADMCTestFilterWidget::init() {
    ADMCTest::init();

    filter_widget = new FilterWidget(filter_classes);
    add_widget(filter_widget);

    tab_widget = filter_widget->findChild<QTabWidget *>();
    QVERIFY(tab_widget != nullptr);

    simple_tab = tab_widget->widget(SIMPLE_TAB_INDEX);
    QVERIFY(simple_tab != nullptr);

    normal_tab = tab_widget->widget(NORMAL_TAB_INDEX);
    QVERIFY(normal_tab != nullptr);

    advanced_tab = tab_widget->widget(ADVANCED_TAB_INDEX);
    QVERIFY(advanced_tab != nullptr);
}

void ADMCTestFilterWidget::test_simple_tab() {
    tab_widget->setCurrentIndex(SIMPLE_TAB_INDEX);

    auto select_classes_widget = simple_tab->findChild<SelectClassesWidget *>();
    QVERIFY(select_classes_widget != nullptr);

    auto select_button = select_classes_widget->findChild<QPushButton *>();
    QVERIFY(select_button != nullptr);
    select_button->click();

    auto select_classes_dialog = select_classes_widget->findChild<QDialog *>();
    QVERIFY(select_classes_dialog != nullptr);
    QVERIFY(QTest::qWaitForWindowExposed(select_classes_dialog, 1000));

    const QList<QCheckBox *> checkbox_list = select_classes_dialog->findChildren<QCheckBox *>();
    QVERIFY(!checkbox_list.isEmpty());

    for (QCheckBox *checkbox : checkbox_list) {
        const bool checked = (checkbox->text() == "User");
        checkbox->setChecked(checked);
    }

    select_classes_dialog->accept();

    QLineEdit *name_edit = simple_tab->findChild<QLineEdit *>("name_edit");
    name_edit->setText("test");

    const QString correct_filter = "(&(name=*test*)(objectClass=user))";
    const QString filter = filter_widget->get_filter();
    QVERIFY(correct_filter == filter);

    // Serialize
    QHash<QString, QVariant> state;
    filter_widget->save_state(state);

    // Change state
    name_edit->setText("changed");

    // Deserialize
    filter_widget->load_state(state);

    const QString filter_deserialized = filter_widget->get_filter();
    QVERIFY(correct_filter == filter_deserialized);
}

void ADMCTestFilterWidget::test_normal_tab() {
    tab_widget->setCurrentIndex(NORMAL_TAB_INDEX);

    auto select_classes_widget = normal_tab->findChild<SelectClassesWidget *>();
    QVERIFY(select_classes_widget != nullptr);

    auto select_button = select_classes_widget->findChild<QPushButton *>();
    QVERIFY(select_button != nullptr);
    select_button->click();

    auto select_classes_dialog = select_classes_widget->findChild<QDialog *>();
    QVERIFY(select_classes_dialog != nullptr);
    QVERIFY(QTest::qWaitForWindowExposed(select_classes_dialog, 1000));

    const QList<QCheckBox *> checkbox_list = select_classes_dialog->findChildren<QCheckBox *>();
    QVERIFY(!checkbox_list.isEmpty());

    for (QCheckBox *checkbox : checkbox_list) {
        const bool checked = (checkbox->text() == "User");
        checkbox->setChecked(checked);
    }

    select_classes_dialog->accept();

    auto filter_builder = normal_tab->findChild<FilterBuilder *>();
    QVERIFY(filter_builder != nullptr);

    QLineEdit *value_edit = filter_builder->findChild<QLineEdit *>("value_edit");
    QVERIFY(value_edit != nullptr);
    value_edit->setText("value");

    QPushButton *add_button = normal_tab->findChild<QPushButton *>("add_button");
    QVERIFY(add_button != nullptr);
    add_button->click();

    const QString correct_filter = "(&(objectClass=user)(assistant=value))";
    const QString filter = filter_widget->get_filter();
    QVERIFY(correct_filter == filter);

    // Serialize
    QHash<QString, QVariant> state;
    filter_widget->save_state(state);

    // Change state
    value_edit->setText("value 2");
    add_button->click();

    // Deserialize
    filter_widget->load_state(state);

    const QString filter_deserialized = filter_widget->get_filter();
    QVERIFY(correct_filter == filter_deserialized);
}

void ADMCTestFilterWidget::test_advanced_tab() {
    tab_widget->setCurrentIndex(ADVANCED_TAB_INDEX);

    auto edit = advanced_tab->findChild<QPlainTextEdit *>();
    QVERIFY(edit != nullptr);

    edit->setPlainText("test");

    const QString correct_filter = "test";
    const QString filter = filter_widget->get_filter();
    QVERIFY(correct_filter == filter);

    // Serialize
    QHash<QString, QVariant> state;
    filter_widget->save_state(state);

    // Change state
    edit->setPlainText("changed");

    // Deserialize
    filter_widget->load_state(state);

    const QString filter_deserialized = filter_widget->get_filter();
    QVERIFY(correct_filter == filter_deserialized);
}

QTEST_MAIN(ADMCTestFilterWidget)
