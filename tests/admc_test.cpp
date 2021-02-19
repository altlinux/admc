/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "admc_test.h"

#include "ad_interface.h"
#include "ad_utils.h"
#include "ad_object.h"
#include "ad_config.h"
#include "status.h"
#include "object_model.h"

#include <QTest>
#include <QModelIndex>
#include <QTreeView>

#define PRINT_FOCUS_WIDGET_BEFORE_TAB false
#define PRINT_FOCUS_WIDGET_AFTER_TAB false

void ADMCTest::initTestCase() {
    QVERIFY2(ad.is_connected(), "Failed to connect to AD server");

    // TODO: check for load failure
    ADCONFIG()->load(ad);

    // NOTE: temporary band-aid until messages are routed correctly throgh AdInterface instance. This makes status error messages be printed to console. I think it's useful to understand why a test failed. When messages are collected in an AdInterface instances, can just print them here ourselves and avoid touching Status.
    STATUS()->print_errors = true;

    // Cleanup before all tests in-case this test suite was
    // previously interrupted and a cleanup wasn't performed
    cleanup();
}

void ADMCTest::cleanupTestCase() {

}

void ADMCTest::init() {
    parent_widget = new QWidget();
    
    const QString dn = test_arena_dn();
    const bool create_success = ad.object_add(dn, CLASS_OU);

    QVERIFY2(create_success, "Failed to create test-arena");
}

void ADMCTest::cleanup() {
    if (parent_widget != nullptr) {
        delete parent_widget;
        parent_widget = nullptr;
    }

    // Delete test arena, if it exists
    const QString dn = test_arena_dn();

    const QHash<QString, AdObject> search_results = ad.search(QString(), QList<QString>(), SearchScope_Object, dn);
    const bool test_arena_exists = (search_results.size() == 1);
    if (test_arena_exists) {
        const bool delete_success = ad.object_delete(dn);
        QVERIFY2(delete_success, "Failed to delete test-arena or it's contents");
        QVERIFY2(!object_exists(dn), "Deleted test-arena still exists");
    }
}

QString ADMCTest::test_arena_dn() {
    const QString head_dn = ad.domain_head();
    const QString dn = QString("OU=test-arena,%1").arg(head_dn);

    return dn;
}

QString ADMCTest::test_object_dn(const QString &name, const QString &object_class) {
    const QString parent = test_arena_dn();
    const QString dn = dn_from_name_and_parent(name, parent, object_class);

    return dn;
}

bool ADMCTest::object_exists(const QString &dn) {
    const QHash<QString, AdObject> search_results = ad.search(QString(), QList<QString>(), SearchScope_Object, dn);
    const bool exists = (search_results.size() == 1);

    return exists;
}

void tab(const int n) {
    static int tab_number = 0;
    for (int i = 0; i < n; i++) {
        if (PRINT_FOCUS_WIDGET_BEFORE_TAB) {
            qInfo() << tab_number << "=" << QApplication::focusWidget();
        }

        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab);

        if (PRINT_FOCUS_WIDGET_AFTER_TAB) {
            qInfo() << tab_number << "=" << QApplication::focusWidget();
        }
    }
}

// Go down the list of objects by pressing Down arrow
// until current item's dn equals to target dn
void navigate_until_object(QTreeView *view, const QString &target_dn) {
    QModelIndex prev_index;
    
    while (true) {
        const QModelIndex current_index = view->selectionModel()->currentIndex();

        const QString current_dn = current_index.data(ObjectRole_DN).toString();
        const bool found_object = (current_dn == target_dn);
        if (found_object) {
            // NOTE: have to set current to select the row. If the first item in view happens to match and no navigation is done, then that first row won't be "selected". Widgets that select items from views rely on whole rows being selected, like they are when you click on them.
            view->setCurrentIndex(current_index);
            break;
        }

        // NOTE: when reached end of view, current index
        // will stop changing
        const bool navigated_to_end_of_view = (prev_index == current_index);
        QVERIFY2(!navigated_to_end_of_view, "Navigated to end of view and failed to find object");
        if (navigated_to_end_of_view) {
            break;
        }

        QTest::keyClick(QApplication::focusWidget(), Qt::Key_Down);

        prev_index = current_index;
    }
}
