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
    qRegisterMetaType<QHash<QString, AdObject>>("QHash<QString, AdObject>");

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
    const QString head_dn = ADCONFIG()->domain_head();
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
    
    QAbstractItemModel *model = view->model();

    QList<QModelIndex> search_stack;
    
    // NOTE: start at invalid index to iterate over top items
    search_stack.append(QModelIndex());

    bool found_object = false;

    while (!search_stack.isEmpty()) {
        const QModelIndex index = search_stack.takeFirst();

        const QString dn = index.data(ObjectRole_DN).toString();

        // NOTE: need to expand items because some models
        // used in ADMC load the model dynamically from
        // server as items are expanded (for example, the
        // model used by move dialog)
        const bool is_parent_of_object = (target_dn.contains(dn));
        if (is_parent_of_object) {
            view->expand(index);
        }

        const bool found_object = (dn == target_dn);
        if (found_object) {
            view->setCurrentIndex(index);

            return;
        }

        for (int row = 0; row < model->rowCount(index); row++) {
            const QModelIndex child = model->index(row, 0, index);

            search_stack.append(child);
        }
    }

    QFAIL(qPrintable(QString("Failed to navigate to object %1").arg(target_dn)));
}

void ADMCTest::wait_for_find_results_to_load(QTreeView *view) {
    int timer = 0;
    while (view->model()->rowCount() == 0) {
        QTest::qWait(1);
        timer++;
        QVERIFY2((timer < 1000), "Find results failed to load, took too long");
    }
}
