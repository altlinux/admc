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

#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/select_classes_widget.h"
#include "ad_interface.h"
#include "filter.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

FilterWidgetSimpleTab::FilterWidgetSimpleTab()
: FilterWidgetTab()
{
    select_classes = new SelectClassesWidget();

    name_edit = new QLineEdit(this);

    auto name_layout = new QFormLayout();
    name_layout->addRow(tr("Name:"), name_edit);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(select_classes);
    layout->addLayout(name_layout);
    layout->addStretch(100);
}

QString FilterWidgetSimpleTab::get_filter() const {
    const QString name_filter =
    [this]() {
        const QString name = name_edit->text();

        return filter_CONDITION(Condition_Contains, ATTRIBUTE_NAME, name);
    }();

    const QString classes_filter = select_classes->get_filter();

    return filter_AND({name_filter, classes_filter});
}
