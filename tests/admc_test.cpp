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

#include "admc_test.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/select_base_widget.h"
#include "globals.h"
#include "select_container_dialog.h"
#include "select_object_advanced_dialog.h"
#include "select_object_dialog.h"
#include "utils.h"
#include "edits/attribute_edit.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QTest>
#include <QTimer>
#include <QTreeView>
#include <QFormLayout>

#define PRINT_FOCUS_WIDGET_BEFORE_TAB false
#define PRINT_FOCUS_WIDGET_AFTER_TAB false

void ADMCTest::initTestCase() {
    qRegisterMetaType<QHash<QString, AdObject>>("QHash<QString, AdObject>");

    QVERIFY2(ad.is_connected(), "Failed to connect to AD server");

    g_adconfig->load(ad, QLocale(QLocale::English));
    AdInterface::set_permanent_adconfig(g_adconfig);

    // TODO: temp band-aid. A proper solution would be to
    // switch to using a pointer for AD and then
    // initializing it here after loading adconfig. This way
    // set_adconfig() won't be needed.
    ad.set_adconfig(g_adconfig);

    // Cleanup before all tests in-case this test suite was
    // previously interrupted and a cleanup wasn't performed
    cleanup();
}

void ADMCTest::cleanupTestCase() {
}

void ADMCTest::init() {
    parent_widget = new QWidget();
    layout = new QFormLayout();
    parent_widget->setLayout(layout);

    edits.clear();

    const QString dn = test_arena_dn();
    const bool create_success = ad.object_add(dn, CLASS_OU);

    QVERIFY2(create_success, "Failed to create test-arena");

    // Show parent widget and wait for it to appear
    parent_widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(parent_widget, 1000));
}

void ADMCTest::cleanup() {
    // Print AD error messages
    if (ad.any_error_messages()) {
        qInfo() << "AD errors:";

        for (const auto &message : ad.messages()) {
            if (message.type() == AdMessageType_Error) {
                qInfo() << message.text();
            }
        }
    }

    ad.clear_messages();

    if (parent_widget != nullptr) {
        delete parent_widget;
        parent_widget = nullptr;
    }

    // Delete test arena, if it exists
    const QString dn = test_arena_dn();

    const QHash<QString, AdObject> results = ad.search(dn, SearchScope_Object, QString(), QList<QString>());
    const bool test_arena_exists = (results.size() == 1);
    if (test_arena_exists) {
        const bool delete_success = ad.object_delete(dn);
        QVERIFY2(delete_success, "Failed to delete test-arena or it's contents");
        QVERIFY2(!object_exists(dn), "Deleted test-arena still exists");
    }
}

QString ADMCTest::test_arena_dn() {
    const QString head_dn = g_adconfig->domain_head();
    const QString dn = QString("OU=test-arena,%1").arg(head_dn);

    return dn;
}

QString ADMCTest::test_object_dn(const QString &name, const QString &object_class) {
    const QString parent = test_arena_dn();
    const QString dn = dn_from_name_and_parent(name, parent, object_class);

    return dn;
}

bool ADMCTest::object_exists(const QString &dn) {
    const QHash<QString, AdObject> results = ad.search(dn, SearchScope_Object, QString(), QList<QString>());
    const bool exists = (results.size() == 1);

    return exists;
}

// Go down the list of objects by pressing Down arrow
// until current item's dn equals to target dn
void navigate_until_object(QTreeView *view, const QString &target_dn, const int dn_role) {
    QModelIndex prev_index;

    QAbstractItemModel *model = view->model();

    QList<QModelIndex> search_stack;

    // NOTE: start at invalid index to iterate over top items
    search_stack.append(QModelIndex());

    while (!search_stack.isEmpty()) {
        const QModelIndex index = search_stack.takeFirst();

        const QString dn = index.data(dn_role).toString();

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

void ADMCTest::close_message_box() {
    auto message_box = parent_widget->findChild<QMessageBox *>();
    if (message_box != nullptr) {
        QVERIFY(QTest::qWaitForWindowExposed(message_box, 1000));
        message_box->accept();

        delete message_box;
    }
}

void ADMCTest::select_in_select_dialog(SelectObjectDialog *select_dialog, const QString &dn) {
    auto add_button = select_dialog->findChild<QPushButton *>("add_button");
    QVERIFY(add_button != nullptr);
    add_button->click();

    // Find dialog has been opened, so switch to it
    auto find_select_dialog = select_dialog->findChild<SelectObjectAdvancedDialog *>();
    QVERIFY(find_select_dialog != nullptr);
    QVERIFY(QTest::qWaitForWindowExposed(find_select_dialog, 1000));

    // Enter group name in "Name" edit
    const QString name = dn_get_name(dn);
    auto simple_tab = select_dialog->findChild<FilterWidgetSimpleTab *>();
    QVERIFY(simple_tab != nullptr);
    auto name_edit = simple_tab->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);
    name_edit->setText(name);

    // Press "Find" button
    auto find_button = find_select_dialog->findChild<QPushButton *>("find_button");
    QVERIFY(find_button != nullptr);
    find_button->click();

    // Switch to find results
    auto find_results_view = find_select_dialog->findChild<QTreeView *>();
    QVERIFY(find_results_view != nullptr);

    wait_for_find_results_to_load(find_results_view);

    // Select group in view
    navigate_until_object(find_results_view, dn, ObjectRole_DN);
    const QModelIndex selected_index = find_results_view->selectionModel()->currentIndex();
    const QString selected_dn = selected_index.data(ObjectRole_DN).toString();

    find_select_dialog->accept();
}

void ADMCTest::select_object_dialog_select(const QString &dn) {
    auto select_dialog = parent_widget->findChild<SelectObjectDialog *>();
    QVERIFY(select_dialog != nullptr);

    auto select_base_widget = select_dialog->findChild<SelectBaseWidget *>();
    QVERIFY(select_base_widget != nullptr);
    select_base_widget_add(select_base_widget, test_arena_dn());

    auto edit = select_dialog->findChild<QLineEdit *>("edit");
    QVERIFY(edit != nullptr);

    auto add_button = select_dialog->findChild<QPushButton *>("add_button");
    QVERIFY(add_button != nullptr);

    edit->setText(dn_get_name(dn));
    add_button->click();

    select_dialog->accept();

    delete select_dialog;
}

void ADMCTest::add_attribute_edit(AttributeEdit *edit) {
    edit->add_to_layout(layout);
}

void ADMCTest::add_widget(QWidget *widget) {
    layout->addWidget(widget);
}

// Edit should do nothing if value wasn't modified
void ADMCTest::test_edit_apply_unmodified(AttributeEdit *edit, const QString &dn) {
    const AdObject object_before = ad.search_object(dn);

    edit->load(ad, object_before);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject object_after = ad.search_object(dn);

    QVERIFY(object_before.get_attributes_data() == object_after.get_attributes_data());
}

void select_base_widget_add(SelectBaseWidget *widget, const QString &dn) {
    auto browse_button = widget->findChild<QPushButton *>();
    QVERIFY(browse_button != nullptr);

    browse_button->click();

    auto select_container_dialog = widget->findChild<SelectContainerDialog *>();
    QVERIFY(select_container_dialog != nullptr);
    QVERIFY(QTest::qWaitForWindowExposed(select_container_dialog, 1000));

    auto select_container_view = select_container_dialog->findChild<QTreeView *>();
    QVERIFY(select_container_view != nullptr);
    navigate_until_object(select_container_view, dn, ContainerRole_DN);

    select_container_dialog->accept();
    QVERIFY(QTest::qWaitForWindowExposed(widget, 1000));

    // NOTE: have to delete manually, dialog deletes itself
    // on close a bit late which causes consecutive calls
    // calls to get the dialog that should've been destroyed
    delete select_container_dialog;
}
