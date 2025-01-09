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

#include "admc_test_find_object_dialog.h"

#include "adldap.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/ui_filter_widget.h"
#include "filter_widget/ui_filter_widget_advanced_tab.h"
#include "filter_widget/ui_filter_widget_simple_tab.h"
#include "find_widgets/find_object_dialog.h"
#include "find_widgets/find_widget.h"
#include "ui_find_object_dialog.h"
#include "ui_find_widget.h"

#include <QTreeView>

void ADMCTestFindObjectDialog::simple_tab() {
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    auto find_dialog = new FindObjectDialog(nullptr, parent, parent_widget);
    find_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(find_dialog, 1000));

    FindWidget *find_widget = find_dialog->ui->find_widget;

    // Enter name in search field
    FilterWidgetSimpleTab *simple_tab = find_widget->ui->filter_widget->ui->simple_tab;
    simple_tab->ui->name_edit->setText(user_name);

    // Press find button
    QPushButton *find_button = find_widget->ui->find_button;
    find_button->click();

    // Confirm that results are not empty
    auto find_results = find_dialog->findChild<QTreeView *>();

    wait_for_find_results_to_load(find_results);

    QVERIFY2(find_results->model()->rowCount(), "No results found");
}

void ADMCTestFindObjectDialog::advanced_tab() {
    const QString parent = test_arena_dn();

    const QString user_name = TEST_USER;
    const QString user_dn = test_object_dn(user_name, CLASS_USER);

    // Create test user
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY2(create_user_success, "Failed to create user");
    QVERIFY2(object_exists(user_dn), "Created user doesn't exist");

    auto find_dialog = new FindObjectDialog(nullptr, parent, parent_widget);
    find_dialog->open();
    QVERIFY(QTest::qWaitForWindowExposed(find_dialog, 1000));

    FilterWidget *filter_widget = find_dialog->ui->find_widget->ui->filter_widget;
    QTabWidget *tab_widget = filter_widget->ui->tab_widget;
    FilterWidgetAdvancedTab *advanced_tab = filter_widget->ui->advanced_tab;
    tab_widget->setCurrentWidget(advanced_tab);

    QPlainTextEdit *ldap_filter_edit = advanced_tab->ui->ldap_filter_edit;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DN, user_dn);
    ldap_filter_edit->setPlainText(filter);

    QPushButton *find_button = find_dialog->ui->find_widget->ui->find_button;
    find_button->click();

    auto find_results = find_dialog->findChild<QTreeView *>();

    wait_for_find_results_to_load(find_results);

    QVERIFY2(find_results->model()->rowCount(), "No results found");
}

QTEST_MAIN(ADMCTestFindObjectDialog)
