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

#include "admc_test_attributes_tab.h"

#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "attribute_dialogs/ui_list_attribute_dialog.h"
#include "attribute_dialogs/ui_string_attribute_dialog.h"
#include "tabs/ui_attributes_tab.h"

#include <QDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>

Q_DECLARE_METATYPE(AttributeFilter)

// NOTE: can't be set because order is important. Read only
// has to be set first to enable other filters.
const QList<AttributeFilter> all_filters = {
    AttributeFilter_Unset,
    AttributeFilter_ReadOnly,
    AttributeFilter_Mandatory,
    AttributeFilter_Optional,
    AttributeFilter_SystemOnly,
    AttributeFilter_Constructed,
    AttributeFilter_Backlink,
};

void ADMCTestAttributesTab::init() {
    ADMCTest::init();

    view = new QTreeView(parent_widget);
    filter_button = new QPushButton(parent_widget);
    edit_button = new QPushButton(parent_widget);
    auto view_button = new QPushButton(parent_widget);
    load_optional_attrs_button = new QPushButton(parent_widget);

    edit = new AttributesTabEdit(view, filter_button, edit_button, view_button, load_optional_attrs_button, parent_widget);

    filter_menu = view->findChild<AttributesTabFilterMenu *>();
    QVERIFY(filter_menu);

    model = edit->findChild<QStandardItemModel *>();
    QVERIFY(model);

    proxy = edit->findChild<QSortFilterProxyModel *>();
    QVERIFY(proxy);

    // Create test user
    const QString name = TEST_USER;
    dn = test_object_dn(name, CLASS_USER);
    const bool create_success = ad.object_add(dn, CLASS_USER);
    QVERIFY(create_success);

    // Load it into the tab
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    // NOTE: filters might be messed up in settings by user
    // so reset it before tests
    set_filter(all_filters, true);
}

void ADMCTestAttributesTab::cleanup() {
    ADMCTest::cleanup();

    edit_list.clear();
}

void ADMCTestAttributesTab::load() {
    // Check that some attributes loaded correctly
    auto check_value = [&](const QString &name, const QString &value) {
        const QList<QStandardItem *> item_list = model->findItems(name);

        if (item_list.size() != 1) {
            return false;
        }

        QStandardItem *name_item = item_list[0];
        QStandardItem *value_item = model->item(name_item->row(), 1);
        const QString &this_value = value_item->text();
        const bool value_is_correct = (this_value == value);

        return value_is_correct;
    };

    check_value("name", TEST_USER);
    check_value("distinguishedName", dn);
}

void ADMCTestAttributesTab::filter_data() {
    QTest::addColumn<AttributeFilter>("filter");
    QTest::addColumn<QString>("attribute");

    QTest::newRow("unset") << AttributeFilter_Unset << ATTRIBUTE_HOME_PHONE;
    QTest::newRow("read only") << AttributeFilter_ReadOnly << ATTRIBUTE_OBJECT_GUID;
    QTest::newRow("system only") << AttributeFilter_SystemOnly << ATTRIBUTE_WHEN_CREATED;
    QTest::newRow("constructed") << AttributeFilter_Constructed << "allowedAttributes";
    QTest::newRow("backlink") << AttributeFilter_Backlink << ATTRIBUTE_MEMBER_OF;
    QTest::newRow("mandatory") << AttributeFilter_Mandatory << "instanceType";
    QTest::newRow("optional") << AttributeFilter_Optional << ATTRIBUTE_COUNTRY_CODE;

    // NOTE: read only also needs to affect these read only
    // attributes
    QTest::newRow("read only allowed") << AttributeFilter_ReadOnly << "allowedAttributes";
    QTest::newRow("read only when created") << AttributeFilter_ReadOnly << ATTRIBUTE_WHEN_CREATED;
    QTest::newRow("read only member of") << AttributeFilter_ReadOnly << ATTRIBUTE_MEMBER_OF;
}

// Test filters by checking that affected attributes are
// visible/hidden when their filters are checked/unchecked
void ADMCTestAttributesTab::filter() {
    QFETCH(AttributeFilter, filter);
    QFETCH(QString, attribute);

    auto check_filtering = [&](const bool should_be_visible) {
        const QList<QStandardItem *> item_list = model->findItems(attribute);
        QVERIFY2((item_list.size() == 1), qPrintable(QString("Failed to find attribute %1)").arg(attribute)));

        QStandardItem *item = item_list[0];
        const QModelIndex index = item->index();
        const QModelIndex proxy_index = proxy->mapFromSource(index);
        const bool is_visible = proxy_index.isValid();

        const bool correct_filtering = (is_visible == should_be_visible);
        QVERIFY2(correct_filtering, qPrintable(QString("filter = %1, attribute = %2, is_visible = %3, should_be_visible = %4").arg(filter).arg(attribute).arg(is_visible).arg(should_be_visible)));
    };

    // Disable(check) all filters (to filter nothing) and
    // check that attribute is shown
    set_filter(all_filters, true);
    check_filtering(true);

    // Enable(uncheck) filter and check that attribute is
    // hidden
    set_filter({filter}, false);
    check_filtering(false);

    // Reset after test is done
    set_filter(all_filters, true);
}

// Change description attribute, apply and then confirm
// changes
void ADMCTestAttributesTab::apply() {
    const QString correct_value = "test description";

    navigate_until_object(view, "description", Qt::DisplayRole);
    edit_button->click();

    auto list_attribute_dialog = view->findChild<ListAttributeDialog *>();
    QVERIFY(list_attribute_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(list_attribute_dialog, 1000));

    QPushButton *add_button = list_attribute_dialog->ui->add_button;
    add_button->click();

    auto string_attribute_dialog = list_attribute_dialog->findChild<StringAttributeDialog *>();
    QVERIFY(string_attribute_dialog);
    QVERIFY(QTest::qWaitForWindowExposed(string_attribute_dialog, 1000));

    QPlainTextEdit *string_attribute_dialog_edit = string_attribute_dialog->ui->edit;
    string_attribute_dialog_edit->setPlainText(correct_value);

    string_attribute_dialog->accept();
    list_attribute_dialog->accept();

    edit->apply(ad, dn);

    const AdObject object = ad.search_object(dn);
    const QString description_value = object.get_string(ATTRIBUTE_DESCRIPTION);
    QCOMPARE(description_value, correct_value);
}

void ADMCTestAttributesTab::set_filter(const QList<AttributeFilter> &filter_list, const bool state) {
    for (const AttributeFilter &filter : filter_list) {
        QAction *action = filter_menu->findChild<QAction *>(QString::number(filter));
        QVERIFY(action);
        action->setChecked(state);
    }
}

QTEST_MAIN(ADMCTestAttributesTab)
