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

#include "filter_widget/filter_widget.h"
#include "filter_widget/filter_widget_advanced_tab.h"
#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/filter_widget_simple_tab.h"

#include <QDebug>
#include <QTabWidget>
#include <QVBoxLayout>

FilterWidget::FilterWidget(AdConfig *adconfig, const QList<QString> classes)
: QWidget() {
    tab_widget = new QTabWidget();

    auto add_tab = [this](FilterWidgetTab *tab, const QString &title) {
        tab_widget->addTab(tab, title);
    };

    simple_tab = new FilterWidgetSimpleTab(adconfig, classes);
    normal_tab = new FilterWidgetNormalTab(adconfig, classes);
    advanced_tab = new FilterWidgetAdvancedTab();
    add_tab(simple_tab, tr("Simple"));
    add_tab(normal_tab, tr("Normal"));
    add_tab(advanced_tab, tr("Advanced"));

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(tab_widget);
}

QString FilterWidget::get_filter() const {
    const FilterWidgetTab *current_tab = dynamic_cast<FilterWidgetTab *>(tab_widget->currentWidget());

    if (current_tab) {
        return current_tab->get_filter();
    } else {
        qDebug() << "Inserted a non FilterWidgetTab into FilterWidget";
        return QString();
    }
}

QVariant FilterWidget::save_state() const {
    QHash<QString, QVariant> state;
    
    state["current_tab_index"] = tab_widget->currentIndex();
    state["simple_state"] = simple_tab->save_state();
    state["normal_state"] = normal_tab->save_state();
    state["advanced_state"] = advanced_tab->save_state();

    return QVariant(state);
}

void FilterWidget::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();
    
    tab_widget->setCurrentIndex(state["current_tab_index"].toInt());
    simple_tab->restore_state(state["simple_state"]);
    normal_tab->restore_state(state["normal_state"]);
    advanced_tab->restore_state(state["advanced_state"]);
}
