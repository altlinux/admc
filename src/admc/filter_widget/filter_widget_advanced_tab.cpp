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

#include "filter_widget/filter_widget_advanced_tab.h"
#include "filter_widget/ui_filter_widget_advanced_tab.h"

FilterWidgetAdvancedTab::FilterWidgetAdvancedTab()
: FilterWidgetTab() {
    ui = new Ui::FilterWidgetAdvancedTab();
    ui->setupUi(this);
}

FilterWidgetAdvancedTab::~FilterWidgetAdvancedTab() {
    delete ui;
}

QString FilterWidgetAdvancedTab::get_filter() const {
    const QString filter = ui->ldap_filter_edit->toPlainText();

    return filter;
}

void FilterWidgetAdvancedTab::clear() {
    ui->ldap_filter_edit->clear();
}

QVariant FilterWidgetAdvancedTab::save_state() const {
    const QString filter = ui->ldap_filter_edit->toPlainText();

    return QVariant(filter);
}

void FilterWidgetAdvancedTab::restore_state(const QVariant &state_variant) {
    const QString filter = state_variant.toString();
    ui->ldap_filter_edit->setPlainText(filter);
}
