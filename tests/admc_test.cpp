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

#include "admc_test.h"

#include "adldap.h"
#include "attribute_edits/attribute_edit.h"
#include "console_impls/object_impl.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/select_base_widget.h"
#include "filter_widget/ui_filter_widget.h"
#include "filter_widget/ui_filter_widget_simple_tab.h"
#include "filter_widget/ui_select_base_widget.h"
#include "find_widgets/find_widget.h"
#include "globals.h"
#include "select_dialogs/select_container_dialog.h"
#include "select_dialogs/select_object_advanced_dialog.h"
#include "select_dialogs/select_object_dialog.h"
#include "ui_find_widget.h"
#include "ui_select_container_dialog.h"
#include "ui_select_object_advanced_dialog.h"
#include "ui_select_object_dialog.h"
#include "utils.h"
#include "fsmo/fsmo_utils.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QTest>
#include <QTimer>
#include <QTreeView>

#define PRINT_FOCUS_WIDGET_BEFORE_TAB false
#define PRINT_FOCUS_WIDGET_AFTER_TAB false

void ADMCTest::initTestCase() {
    qRegisterMetaType<QHash<QString, AdObject>>("QHash<QString, AdObject>");

    bool connected = true;
    if (!ad.is_connected()) {
        QStringList dc_list = get_domain_hosts(ad.get_domain(), QString());
        dc_list.removeAll(ad.get_dc()); // Remove invalid host
        for (const QString &dc : dc_list) {
            ad.set_dc(dc);
            ad.update_dc();
            connected = ad.is_connected();
            if (connected) {
                break;
            }
        }
    }

    QVERIFY2(connected, "Failed to connect to AD server");

    g_adconfig->load(ad, QLocale(QLocale::English));
    AdInterface::set_config(g_adconfig);

    if (!current_dc_is_master_for_role(ad, FSMORole_PDCEmulation)) {
        connect_host_with_role(ad, FSMORole_PDCEmulation);
    }

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

        if (ad.is_connected()) {
            qInfo() << "Reconnection to DC " << ad.get_dc() << " is successfull.";
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
    const QString head_dn = g_adconfig->domain_dn();
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

bool ADMCTest::message_box_is_open() const {
    auto message_box = parent_widget->findChild<QMessageBox *>();
    const bool out = (message_box != nullptr);

    return out;
}

void ADMCTest::select_in_select_dialog(SelectObjectDialog *select_dialog, const QString &dn) {
    QPushButton *add_button = select_dialog->ui->add_button;
    add_button->click();

    // Find dialog has been opened, so switch to it
    auto find_select_dialog = select_dialog->findChild<SelectObjectAdvancedDialog *>();
    QVERIFY(find_select_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(find_select_dialog, 1000));

    FindWidget *find_widget = find_select_dialog->ui->find_widget;

    // Enter group name in "Name" edit
    const QString name = dn_get_name(dn);
    QLineEdit *name_edit = find_widget->ui->filter_widget->ui->simple_tab->ui->name_edit;
    name_edit->setText(name);

    // Press "Find" button
    auto find_button = find_widget->ui->find_button;
    find_button->click();

    // Switch to find results
    auto find_results_view = find_select_dialog->findChild<QTreeView *>();
    QVERIFY(find_results_view);

    wait_for_find_results_to_load(find_results_view);

    // Select group in view
    navigate_until_object(find_results_view, dn, ObjectRole_DN);
    const QModelIndex selected_index = find_results_view->selectionModel()->currentIndex();
    const QString selected_dn = selected_index.data(ObjectRole_DN).toString();

    find_select_dialog->accept();
}

void ADMCTest::select_object_dialog_select(const QString &dn) {
    auto select_dialog = parent_widget->findChild<SelectObjectDialog *>();
    QVERIFY(select_dialog);

    SelectBaseWidget *select_base_widget = select_dialog->ui->select_base_widget;
    select_base_widget_add(select_base_widget, test_arena_dn());

    QLineEdit *edit = select_dialog->ui->name_edit;
    edit->setText(dn_get_name(dn));

    QPushButton *add_button = select_dialog->ui->add_button;
    add_button->click();

    select_dialog->accept();

    delete select_dialog;
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

void ADMCTest::select_base_widget_add(SelectBaseWidget *widget, const QString &dn) {
    QPushButton *browse_button = widget->ui->browse_button;
    browse_button->click();

    auto select_container_dialog = widget->findChild<SelectContainerDialog *>();
    QVERIFY(select_container_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(select_container_dialog, 1000));

    QTreeView *select_container_view = select_container_dialog->ui->view;
    navigate_until_object(select_container_view, dn, ContainerRole_DN);

    select_container_dialog->accept();
    QVERIFY(QTest::qWaitForWindowExposed(widget, 1000));
}

void test_lineedit_autofill(QLineEdit *src_edit, QLineEdit *dest_edit) {
    const QString expected_dest_text = "test";

    src_edit->setText(expected_dest_text);

    const QString actual_dest_text = dest_edit->text();
    QCOMPARE(actual_dest_text, expected_dest_text);
}
