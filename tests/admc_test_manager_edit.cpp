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

#include "admc_test_manager_edit.h"

#include "edits/manager_edit.h"
#include "globals.h"
#include "properties_dialog.h"

#include <QPushButton>
#include <QLineEdit>

void ADMCTestManagerEdit::init() {
    ADMCTest::init();

    edit = new ManagerEdit(ATTRIBUTE_MANAGER, &edits, parent_widget);
    add_attribute_edit(edit);

    manager_display = parent_widget->findChild<QLineEdit *>("manager_display");
    change_button = parent_widget->findChild<QPushButton *>("change_button");
    clear_button = parent_widget->findChild<QPushButton *>("clear_button");
    properties_button = parent_widget->findChild<QPushButton *>("properties_button");

    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    const QString manager_name = QString("%1%2").arg(TEST_USER, "-manager");
    manager_dn = test_object_dn(manager_name, CLASS_USER);
    const bool create_manager_success = ad.object_add(manager_dn, CLASS_USER);
    QVERIFY(create_manager_success);
}

void ADMCTestManagerEdit::load_empty() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QVERIFY(manager_display->text().isEmpty());
}

void ADMCTestManagerEdit::load() {
    const bool replace_success = ad.attribute_replace_string(dn, ATTRIBUTE_MANAGER, manager_dn);
    QVERIFY(replace_success);

    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QCOMPARE(manager_display->text(), manager_dn);
}

void ADMCTestManagerEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestManagerEdit::apply_after_change() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    change_button->click();
    select_object_dialog_select(manager_dn);

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const QString updated_manager = updated_object.get_string(ATTRIBUTE_MANAGER);
    QCOMPARE(updated_manager, manager_dn);
}

void ADMCTestManagerEdit::apply_after_clear() {
    load();

    clear_button->click();

    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const QString updated_manager = updated_object.get_string(ATTRIBUTE_MANAGER);
    QVERIFY(updated_manager.isEmpty());
}

void ADMCTestManagerEdit::properties() {
    load();

    properties_button->click();

    const bool properties_open = []() {
        const QWidgetList widget_list = QApplication::topLevelWidgets();
        for (QWidget *widget : widget_list) {
            auto properties_dialog = qobject_cast<PropertiesDialog *>(widget);

            if (properties_dialog != nullptr) {
                return true;
            }
        }

        return false;
    }();
    QVERIFY(properties_open);
}

QTEST_MAIN(ADMCTestManagerEdit)
