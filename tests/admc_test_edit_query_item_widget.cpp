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

#include "admc_test_edit_query_item_widget.h"

#include "edit_query_item_widget.h"
#include "edit_query_item_widget_p.h"
#include "console_impls/query_item_impl.h"
#include "filter_widget/select_base_widget.h"
#include "tab_widget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>

void ADMCTestEditQueryItemWidget::init() {
    ADMCTest::init();

    widget = new EditQueryItemWidget();
    add_widget(widget);

    name_edit = widget->findChild<QLineEdit *>("name_edit");
    QVERIFY(name_edit != nullptr);

    description_edit = widget->findChild<QLineEdit *>("description_edit");
    QVERIFY(description_edit != nullptr);

    scope_checkbox = widget->findChild<QCheckBox *>();
    QVERIFY(scope_checkbox != nullptr);

    SelectBaseWidget *select_base_widget = widget->findChild<SelectBaseWidget *>();
    QVERIFY(select_base_widget != nullptr);
    
    base_combo = select_base_widget->findChild<QComboBox *>();
    QVERIFY(base_combo != nullptr);

    edit_filter_button = widget->findChild<QPushButton *>("edit_filter_button");
    QVERIFY(edit_filter_button != nullptr);

    filter_display = widget->findChild<QTextEdit *>("filter_display");
}

void ADMCTestEditQueryItemWidget::save_and_load() {
    // Define and set correct values
    const QString correct_name = "name";
    name_edit->setText(correct_name);
    
    const QString correct_description = "description";
    description_edit->setText(correct_description);
    
    const bool correct_scope = true;
    scope_checkbox->setChecked(correct_scope);

    // NOTE: just using the default value for base correct
    // value
    const QString correct_base = base_combo->currentText();
    QVERIFY(!correct_base.isEmpty());

    // Set correct filter filter (a bit complicated!)
    edit_filter_button->click();

    EditQueryItemFilterDialog *dialog = widget->findChild<EditQueryItemFilterDialog *>();
    QVERIFY(dialog != nullptr);

    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    QTabWidget *tab_widget = dialog->findChild<QTabWidget *>();
    QVERIFY(tab_widget != nullptr);

    QWidget *simple_tab = tab_widget->widget(0);
    QVERIFY(simple_tab != nullptr);

    QLineEdit *filter_name_edit = simple_tab->findChild<QLineEdit *>("name_edit");
    filter_name_edit->setText("test");

    dialog->accept();

    const QString correct_filter = filter_display->toPlainText();

    QString name;
    QString description;
    QString filter;
    QString base;
    QByteArray filter_state;
    bool scope_is_children;
    widget->save(name, description, filter, base, scope_is_children, filter_state);

    const QList<QStandardItem *> row = [=]() {
        QList<QStandardItem *> out;

        for (int i = 0; i < QueryColumn_COUNT; i++) {
            out.append(new QStandardItem());
        }

        return out;
    }();
    
    // NOTE: need to insert into model so that indexes are valid
    auto model = new QStandardItemModel();
    model->appendRow(row);

    console_query_item_load(row, name, description, filter, filter_state, base, scope_is_children);

    widget->load(row[0]->index());

    QCOMPARE(name_edit->text(), correct_name);
    QCOMPARE(description_edit->text(), correct_description);
    QCOMPARE(scope_checkbox->isChecked(), correct_scope);
    QCOMPARE(base_combo->currentText(), correct_base);
    QCOMPARE(filter_display->toPlainText(), correct_filter);
}

QTEST_MAIN(ADMCTestEditQueryItemWidget)
