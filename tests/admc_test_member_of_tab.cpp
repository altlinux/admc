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

#include "admc_test_member_of_tab.h"

#include "select_dialogs/select_object_dialog.h"
#include "tabs/membership_tab.h"
#include "tabs/ui_membership_tab.h"

#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

// NOTE: there's a primary group by default hence the offset
// by 1 of all row values

void ADMCTestMemberOfTab::init() {
    ADMCTest::init();

    view = new QTreeView(parent_widget);
    auto primary_button = new QPushButton(parent_widget);
    add_button = new QPushButton(parent_widget);
    remove_button = new QPushButton(parent_widget);
    auto properties_button = new QPushButton(parent_widget);
    auto primary_group_label = new QLabel(parent_widget);

    edit = new MembershipTabEdit(view, primary_button, add_button, remove_button, properties_button, primary_group_label, MembershipTabType_MemberOf, parent_widget);

    model = edit->findChild<QStandardItemModel *>();
    QVERIFY(model);

    // Create test user
    const QString user_name = TEST_USER;
    user_dn = test_object_dn(user_name, CLASS_USER);
    const bool create_user_success = ad.object_add(user_dn, CLASS_USER);
    QVERIFY(create_user_success);

    // Create test group
    const QString group_name = TEST_GROUP;
    group_dn = test_object_dn(group_name, CLASS_GROUP);
    const bool create_group_success = ad.object_add(group_dn, CLASS_GROUP);
    QVERIFY(create_group_success);

    // Load it into the tab
    const AdObject object = ad.search_object(user_dn);
    edit->load(ad, object);
}

// Loading a group without members should result in empty
// model
void ADMCTestMemberOfTab::load_empty() {
    QCOMPARE(model->rowCount(), 1);
}

// Loading a group with members should put members in the
// model
void ADMCTestMemberOfTab::load() {
    const bool add_success = ad.group_add_member(group_dn, user_dn);
    QVERIFY(add_success);

    const AdObject object = ad.search_object(user_dn);
    edit->load(ad, object);

    QCOMPARE(model->rowCount(), 2);

    const bool contains_group = (get_group_row(group_dn) != -1);
    QVERIFY(contains_group);
}

int ADMCTestMemberOfTab::get_group_row(const QString &dn) {
    const QString group_name = dn_get_name(dn);

    for (int row = 0; row < model->rowCount(); row++) {
        auto item = model->item(row, 0);
        if (item->text() == group_name) {
            return row;
        }
    }

    return -1;
}

// Removing members should remove members from model and group
void ADMCTestMemberOfTab::remove() {
    load();

    const int group_row = get_group_row(group_dn);
    const QModelIndex index = model->index(group_row, 0);
    view->setCurrentIndex(index);

    remove_button->click();

    edit->apply(ad, user_dn);

    const AdObject updated_object = ad.search_object(user_dn);
    const QList<QString> member_list = updated_object.get_strings(ATTRIBUTE_MEMBER_OF);

    QCOMPARE(model->rowCount(), 1);
    QVERIFY(member_list.isEmpty());
}

void ADMCTestMemberOfTab::add() {
    add_button->click();

    select_object_dialog_select(group_dn);

    // Apply and check object state
    edit->apply(ad, user_dn);
    const AdObject object = ad.search_object(user_dn);
    const QList<QString> member_of_list = object.get_strings(ATTRIBUTE_MEMBER_OF);
    QVERIFY(member_of_list.contains(group_dn));

    // Check ui state after applying (just in case)
    QCOMPARE(model->rowCount(), 2);
    const bool contains_after_add = (get_group_row(group_dn) != -1);
    QVERIFY(contains_after_add);
}

QTEST_MAIN(ADMCTestMemberOfTab)
