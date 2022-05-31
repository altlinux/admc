/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "admc_test_policy_ou_results_widget.h"

#include "ad_filter.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "gplink.h"
#include "policy_ou_results_widget.h"
#include "utils.h"

#include <QStandardItemModel>
#include <QTreeView>

// NOTE: unlike other tests, here we have to create the test
// thing (gpo) once and reuse between tests because creating
// and deleting gpo's takes too much time. If we did that
// for every test they would take forever.

const QString ou_name = "test_ou_for_admc_test";
const QString gpo_alpha_name = "test_policy_for_admc_test_alpha";
const QString gpo_beta_name = "test_policy_for_admc_test_beta";
const QList<QString> gpo_name_list = {
    gpo_alpha_name,
    gpo_beta_name
};
QList<QString> gpo_dn_list = {
    QString(),
    QString()
};

void ADMCTestPolicyOUResultsWidget::initTestCase() {
    ADMCTest::initTestCase();

    for (int i = 0; i < gpo_name_list.size(); i++) {
        const QString gpo_name = gpo_name_list[i];
        QString created_dn;
        const bool gpo_add_success = ad.gpo_add(gpo_name, created_dn);
        gpo_dn_list[i] = created_dn;
        QVERIFY(gpo_add_success);
    }
}

void ADMCTestPolicyOUResultsWidget::cleanupTestCase() {
    ADMCTest::cleanupTestCase();

    // Delete old test-policy, if needed
    for (const QString &gpo_name : gpo_name_list) {
        const QString base = g_adconfig->domain_dn();
        const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_DISPLAY_NAME, gpo_name);
        const QList<QString> attributes = QList<QString>();
        const QHash<QString, AdObject> search_results = ad.search(base, SearchScope_All, filter, attributes);

        for (const QString &dn : search_results.keys()) {
            bool deleted_object;
            ad.gpo_delete(dn, &deleted_object);
        }
    }
}

void ADMCTestPolicyOUResultsWidget::init() {
    ADMCTest::init();

    widget = new PolicyOUResultsWidget(parent_widget);
    add_widget(widget);

    ResultsView *results_view = widget->get_view();
    view = results_view->detail_view();

    model = widget->findChild<QStandardItemModel *>();
    QVERIFY(model);

    ou_dn = test_object_dn(ou_name, CLASS_OU);
    const bool create_ou_succes = ad.object_add(ou_dn, CLASS_OU);
    QVERIFY(create_ou_succes);
}

void ADMCTestPolicyOUResultsWidget::load_empty() {
    widget->update(ou_dn);

    QCOMPARE(view->model()->rowCount(), 0);
}

void ADMCTestPolicyOUResultsWidget::load() {
    // Link gpo to ou
    const AdObject ou_object = ad.search_object(ou_dn);
    const QString gplink_string = ou_object.get_string(ATTRIBUTE_GPLINK);
    Gplink gplink = Gplink(gplink_string);
    gplink.add(gpo_dn_list[0]);
    gplink.set_option(gpo_dn_list[0], GplinkOption_Enforced, true);
    const bool modify_gplink_success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink.to_string());
    QVERIFY(modify_gplink_success);

    widget->update(ou_dn);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->columnCount(), 3);

    QList<QStandardItem *> item_list;
    for (int col = 0; col < model->columnCount(); col++) {
        QStandardItem *item = model->item(0, col);
        QVERIFY(item);
        item_list.append(item);
    }

    QCOMPARE(item_list[0]->text(), gpo_name_list[0]);
    QCOMPARE(item_list[1]->checkState(), Qt::Checked);
    QCOMPARE(item_list[2]->checkState(), Qt::Unchecked);
}

QTEST_MAIN(ADMCTestPolicyOUResultsWidget)
