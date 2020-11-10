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
#include <QComboBox>
#include <QTabWidget>
#include <QGridLayout>
#include <QPushButton>

// TODO: search base input should be in parent widget because its not always needed. For example it's not needed when filter widget will be used to filter contents widget.

FilterWidget::FilterWidget()
: QWidget()
{
    auto search_base_combo_label = new QLabel(tr("In:"));
    search_base_combo = new QComboBox();

    // TODO: technically, entire directory does NOT equal to the domain. In cases where we're browsing multiple domains at the same time (or maybe some other situations as well), we'd need "Entire directory" AND all of domains. Currently search base is set to domain anyway, so would need to start from reworking that.
    search_base_combo->addItem(tr("Entire directory"), AD()->search_base());

    const QString users_dn = "CN=Users," + AD()->search_base();
    search_base_combo->addItem("Users", users_dn);

    auto custom_search_base_button = new QPushButton(tr("Browse"));
    connect(
        custom_search_base_button, &QAbstractButton::clicked,
        this, &FilterWidget::on_custom_search_base);


    tab_widget = new QTabWidget();

    auto normal_tab = new FilterWidgetNormalTab();
    tab_widget->addTab(normal_tab, tr("Normal"));
    tabs.append(normal_tab);

    auto advanced_tab = new FilterWidgetAdvancedTab();
    tab_widget->addTab(advanced_tab, tr("Advanced"));
    tabs.append(advanced_tab);

    auto layout = new QGridLayout();
    setLayout(layout);

    const int search_base_combo_row = layout->rowCount();
    layout->addWidget(search_base_combo_label, search_base_combo_row, 0);
    layout->addWidget(search_base_combo, search_base_combo_row, 1);
    layout->addWidget(custom_search_base_button, search_base_combo_row, 2);

    const int tab_widget_row = layout->rowCount();
    const int tab_widget_col_span = layout->columnCount();
    layout->addWidget(tab_widget, tab_widget_row, 0, 1, tab_widget_col_span);
}

QString FilterWidget::get_filter() const {
    const FilterWidgetTab *current_tab = get_current_tab();
    return current_tab->get_filter();
}

QString FilterWidget::get_search_base() const {
    const int index = search_base_combo->currentIndex();
    const QVariant item_data = search_base_combo->itemData(index);

    return item_data.toString();
}

void FilterWidget::on_custom_search_base() {
    // TODO: maybe need some other classes?
    const QList<QString> selecteds = SelectDialog::open({CLASS_CONTAINER, CLASS_OU});

    if (!selecteds.isEmpty()) {
        const QString selected = selecteds[0];
        const QString name = dn_get_rdn(selected);

        search_base_combo->addItem(name, selected);

        // Select newly added search base
        const int new_base_index = search_base_combo->count() - 1;
        search_base_combo->setCurrentIndex(new_base_index);
    }
}

const FilterWidgetTab *FilterWidget::get_current_tab() const {
    const int index = tab_widget->currentIndex();
    const FilterWidgetTab *current_tab = tabs[index];

    return current_tab;
}
