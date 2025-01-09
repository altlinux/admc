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

#include "admc_test_filter_widget.h"

#include "filter_widget/class_filter_widget.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/select_classes_widget.h"
#include "filter_widget/ui_filter_widget.h"
#include "filter_widget/ui_filter_widget_advanced_tab.h"
#include "filter_widget/ui_filter_widget_normal_tab.h"
#include "filter_widget/ui_filter_widget_simple_tab.h"
#include "filter_widget/ui_select_classes_widget.h"

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

void ADMCTestFilterWidget::init() {
    ADMCTest::init();

    filter_widget = new FilterWidget();
    filter_widget->set_classes(filter_classes, filter_classes);
    add_widget(filter_widget);

    tab_widget = filter_widget->ui->tab_widget;
    simple_tab = filter_widget->ui->simple_tab;
    normal_tab = filter_widget->ui->normal_tab;
    advanced_tab = filter_widget->ui->advanced_tab;
}

void ADMCTestFilterWidget::test_simple_tab() {
    tab_widget->setCurrentWidget(simple_tab);

    SelectClassesWidget *select_classes_widget = simple_tab->ui->select_classes_widget;

    QPushButton *select_button = select_classes_widget->ui->select_button;
    select_button->click();

    auto class_filter_dialog = select_classes_widget->findChild<QDialog *>();
    QVERIFY(class_filter_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(class_filter_dialog, 1000));

    const QList<QCheckBox *> checkbox_list = class_filter_dialog->findChildren<QCheckBox *>();
    QVERIFY(!checkbox_list.isEmpty());

    for (QCheckBox *checkbox : checkbox_list) {
        const bool checked = (checkbox->text() == "User");
        checkbox->setChecked(checked);
    }

    class_filter_dialog->accept();

    QLineEdit *name_edit = simple_tab->ui->name_edit;
    name_edit->setText("test");

    const QString correct_filter = "(&(name=*test*)(objectClass=user))";
    const QString filter = filter_widget->get_filter();
    QCOMPARE(correct_filter, filter);

    // Serialize
    const QVariant state = filter_widget->save_state();

    // Change state
    name_edit->setText("changed");

    // Deserialize
    filter_widget->restore_state(state);

    const QString filter_deserialized = filter_widget->get_filter();
    QCOMPARE(correct_filter, filter_deserialized);
}

void ADMCTestFilterWidget::test_normal_tab() {
    tab_widget->setCurrentWidget(normal_tab);

    SelectClassesWidget *select_classes_widget = normal_tab->ui->select_classes_widget;

    QPushButton *select_button = select_classes_widget->ui->select_button;
    select_button->click();

    auto class_filter_dialog = select_classes_widget->findChild<QDialog *>();
    QVERIFY(class_filter_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(class_filter_dialog, 1000));

    const QList<QCheckBox *> checkbox_list = class_filter_dialog->findChildren<QCheckBox *>();
    QVERIFY(!checkbox_list.isEmpty());

    for (QCheckBox *checkbox : checkbox_list) {
        const bool checked = (checkbox->text() == "User");
        checkbox->setChecked(checked);
    }

    class_filter_dialog->accept();

    QLineEdit *value_edit = normal_tab->ui->value_edit;
    value_edit->setText("value");

    QPushButton *add_button = normal_tab->ui->add_button;
    add_button->click();

    const QString correct_filter = "(&(objectClass=user)(assistant=value))";
    const QString filter = filter_widget->get_filter();
    QCOMPARE(correct_filter, filter);

    // Serialize
    const QVariant state = filter_widget->save_state();

    // Change state
    value_edit->setText("value 2");
    add_button->click();

    // Deserialize
    filter_widget->restore_state(state);

    const QString filter_deserialized = filter_widget->get_filter();
    QCOMPARE(correct_filter, filter_deserialized);
}

void ADMCTestFilterWidget::test_advanced_tab() {
    tab_widget->setCurrentWidget(advanced_tab);

    QPlainTextEdit *edit = advanced_tab->ui->ldap_filter_edit;

    edit->setPlainText("test");

    const QString correct_filter = "test";
    const QString filter = filter_widget->get_filter();
    QCOMPARE(correct_filter, filter);

    // Serialize
    const QVariant state = filter_widget->save_state();

    // Change state
    edit->setPlainText("changed");

    // Deserialize
    filter_widget->restore_state(state);

    const QString filter_deserialized = filter_widget->get_filter();
    QCOMPARE(correct_filter, filter_deserialized);
}

QTEST_MAIN(ADMCTestFilterWidget)
