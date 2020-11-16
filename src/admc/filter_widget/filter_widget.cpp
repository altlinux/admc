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

#include "filter_widget/filter_widget.h"
#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "ad_interface.h"
#include "utils.h"

#include <QDebug>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>

FilterWidget::FilterWidget()
: QWidget()
{
    tab_widget = new QTabWidget();
    tab_widget->addTab(new FilterWidgetSimpleTab(), tr("Simple"));
    tab_widget->addTab(new FilterWidgetNormalTab(), tr("Normal"));
    tab_widget->addTab(new FilterWidgetAdvancedTab(), tr("Advanced"));

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(tab_widget);
}

QString FilterWidget::get_filter() const {
    const FilterWidgetTab *current_tab = dynamic_cast<FilterWidgetTab *> (tab_widget->currentWidget());

    if (current_tab) {
        return current_tab->get_filter();
    } else {
        qDebug() << "Inserted a non FilterWidgetTab into FilterWidget";
        return QString();
    }
}
