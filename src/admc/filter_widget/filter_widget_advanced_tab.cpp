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

#include "filter_widget/filter_widget_advanced_tab.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QLabel>

FilterWidgetAdvancedTab::FilterWidgetAdvancedTab()
: FilterWidgetTab()
{
    auto label = new QLabel(tr("Enter LDAP filter:"));
    ldap_filter_edit = new QPlainTextEdit(this);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(ldap_filter_edit);
}

QString FilterWidgetAdvancedTab::get_filter() const {
    const QString filter = ldap_filter_edit->toPlainText();

    return filter;
}

void FilterWidgetAdvancedTab::save_state(QHash<QString, QVariant> &state) const {
    const QString filter = ldap_filter_edit->toPlainText();
    state["filter"] = filter;
}

void FilterWidgetAdvancedTab::load_state(const QHash<QString, QVariant> &state) {
    const QString filter = state["filter"].toString();
    ldap_filter_edit->setPlainText(filter);
}
