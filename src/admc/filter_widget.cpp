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

#include "filter_widget.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QComboBox>

FilterWidget::FilterWidget()
: QWidget()
{
    advanced_tab = new QWidget();
    {
        ldap_filter_edit = new QPlainTextEdit(this);

        auto layout = new QVBoxLayout();
        advanced_tab->setLayout(layout);
        layout->addWidget(ldap_filter_edit);
    }

    normal_tab = new QWidget();
    {
    
    }

    tab_widget = new QTabWidget();
    tab_widget->addTab(normal_tab, tr("Filter"));
    tab_widget->addTab(advanced_tab, tr("Advanced"));

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(tab_widget);

    connect(
        ldap_filter_edit, &QPlainTextEdit::textChanged,
        [this]() {
            emit changed();
        });
}

QString FilterWidget::get_filter() const {
    const QWidget *current_tab = tab_widget->currentWidget();

    if (current_tab == advanced_tab) {
        return ldap_filter_edit->toPlainText();
    } else {
        return QString();
    }

    return QString();
}
