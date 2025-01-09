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

#include "admc_test_edit_query_item_widget.h"

#include "console_impls/query_item_impl.h"
#include "edit_query_widgets/edit_query_item_widget.h"
#include "filter_widget/filter_dialog.h"
#include "filter_widget/filter_widget.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/select_base_widget.h"
#include "filter_widget/ui_filter_widget.h"
#include "filter_widget/ui_filter_widget_simple_tab.h"
#include "filter_widget/ui_select_base_widget.h"
#include "tab_widget.h"
#include "ui_edit_query_item_widget.h"
#include "ui_filter_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QTextEdit>

void ADMCTestEditQueryItemWidget::init() {
    ADMCTest::init();

    widget = new EditQueryItemWidget();
    add_widget(widget);

    name_edit = widget->ui->name_edit;
    description_edit = widget->ui->description_edit;
    scope_checkbox = widget->ui->scope_checkbox;

    SelectBaseWidget *select_base_widget = widget->ui->select_base_widget;

    base_combo = select_base_widget->ui->combo;

    edit_filter_button = widget->ui->edit_filter_button;

    filter_display = widget->ui->filter_display;
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

    FilterDialog *dialog = widget->findChild<FilterDialog *>();
    QVERIFY(dialog);

    QVERIFY(QTest::qWaitForWindowExposed(dialog, 1000));

    FilterWidgetSimpleTab *simple_tab = dialog->ui->filter_widget->ui->simple_tab;
    QLineEdit *filter_name_edit = simple_tab->ui->name_edit;
    filter_name_edit->setText("test");

    dialog->accept();

    const QString correct_filter = filter_display->toPlainText();

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

    {
        const QString name = widget->name();
        const QString description = widget->description();
        const QString filter = widget->filter();
        const QString base = widget->base();
        const QByteArray filter_state = widget->filter_state();
        const bool scope_is_children = widget->scope_is_children();

        console_query_item_load(row, name, description, filter, filter_state, base, scope_is_children);
    }

    const QModelIndex index = row[0]->index();

    {
        QString name;
        QString description;
        bool scope_is_children;
        QByteArray filter_state;
        QString filter;
        get_query_item_data(index, &name, &description, &scope_is_children, &filter_state, &filter);

        widget->set_data(name, description, scope_is_children, filter_state, filter);
    }

    QCOMPARE(name_edit->text(), correct_name);
    QCOMPARE(description_edit->text(), correct_description);
    QCOMPARE(scope_checkbox->isChecked(), correct_scope);
    QCOMPARE(base_combo->currentText(), correct_base);
    QCOMPARE(filter_display->toPlainText(), correct_filter);
}

QTEST_MAIN(ADMCTestEditQueryItemWidget)
