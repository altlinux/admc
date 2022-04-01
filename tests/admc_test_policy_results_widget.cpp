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

#include "admc_test_policy_results_widget.h"

#include "ad_filter.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "gplink.h"
#include "policy_results_widget.h"
#include "utils.h"

#include <QStandardItemModel>
#include <QTreeView>

// NOTE: unlike other tests, here we have to create the test
// thing (gpo) once and reuse between tests because creating
// and deleting gpo's takes too much time. If we did that
// for every test they would take forever.

const QString gpo_name = "test_policy_for_admc_test_results_widget";

void ADMCTestPolicyResultsWidget::initTestCase() {
    ADMCTest::initTestCase();

    const bool gpo_add_success = ad.gpo_add(gpo_name, gpo);
    QVERIFY(gpo_add_success);
}

void ADMCTestPolicyResultsWidget::cleanupTestCase() {
    ADMCTest::cleanupTestCase();

    // Delete old test-policy, if needed
    const QString base = g_adconfig->domain_dn();
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DISPLAY_NAME, gpo_name);
    const QList<QString> attributes = QList<QString>();
    const QHash<QString, AdObject> search_results = ad.search(base, SearchScope_All, filter, attributes);

    for (const QString &dn : search_results.keys()) {
        bool deleted_object;
        ad.gpo_delete(dn, &deleted_object);
    }
}

void ADMCTestPolicyResultsWidget::init() {
    ADMCTest::init();

    widget = new PolicyResultsWidget(parent_widget);
    add_widget(widget);

    ResultsView *results_view = widget->get_view();
    view = results_view->detail_view();

    model = widget->findChild<QStandardItemModel *>();
    QVERIFY(model);
}

void ADMCTestPolicyResultsWidget::load_empty() {
    widget->update(gpo);

    QCOMPARE(view->model()->rowCount(), 0);
}

void ADMCTestPolicyResultsWidget::load() {
    // Link gpo to ou
    const QString ou_dn = test_object_dn(TEST_OU, CLASS_OU);
    const bool create_ou_success = ad.object_add(ou_dn, CLASS_OU);
    QVERIFY(create_ou_success);
    const AdObject ou_object = ad.search_object(ou_dn);
    const QString gplink_string = ou_object.get_string(ATTRIBUTE_GPLINK);
    Gplink gplink = Gplink(gplink_string);
    gplink.add(gpo);
    gplink.set_option(gpo, GplinkOption_Enforced, true);
    ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink.to_string());

    widget->update(gpo);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->columnCount(), 4);

    QList<QStandardItem *> item_list;
    for (int col = 0; col < 4; col++) {
        QStandardItem *item = model->item(0, col);
        QVERIFY(item);
        item_list.append(item);
    }

    QCOMPARE(item_list[0]->text(), TEST_OU);
    QCOMPARE(item_list[1]->checkState(), Qt::Checked);
    QCOMPARE(item_list[2]->checkState(), Qt::Unchecked);
    QCOMPARE(item_list[3]->text(), dn_get_parent_canonical(ou_dn));
}

QTEST_MAIN(ADMCTestPolicyResultsWidget)
