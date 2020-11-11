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
#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "ad_interface.h"
#include "select_dialog.h"
#include "utils.h"

#include <QLabel>
#include <QTabWidget>
#include <QGridLayout>
#include <QPushButton>

FilterWidget::FilterWidget()
: QWidget()
{
    tab_widget = new QTabWidget();

    auto normal_tab = new FilterWidgetNormalTab();
    tab_widget->addTab(normal_tab, tr("Normal"));
    tabs.append(normal_tab);

    auto advanced_tab = new FilterWidgetAdvancedTab();
    tab_widget->addTab(advanced_tab, tr("Advanced"));
    tabs.append(advanced_tab);

    auto layout = new QGridLayout();
    setLayout(layout);

    const int tab_widget_row = layout->rowCount();
    const int tab_widget_col_span = layout->columnCount();
    layout->addWidget(tab_widget, tab_widget_row, 0, 1, tab_widget_col_span);
}

QString FilterWidget::get_filter() const {
    const FilterWidgetTab *current_tab = get_current_tab();
    return current_tab->get_filter();
}

const FilterWidgetTab *FilterWidget::get_current_tab() const {
    const int index = tab_widget->currentIndex();
    const FilterWidgetTab *current_tab = tabs[index];

    return current_tab;
}
