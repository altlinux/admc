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

FilterWidget::FilterWidget(const QList<QString> classes)
: QWidget() {
    tab_widget = new QTabWidget();

    auto add_tab = [this](FilterWidgetTab *tab, const QString &title) {
        tab_widget->addTab(tab, title);
    };

    simple_tab = new FilterWidgetSimpleTab(classes);
    normal_tab = new FilterWidgetNormalTab(classes);
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

void FilterWidget::save_state(QHash<QString, QVariant> &state) const {
    const int current_tab_index = tab_widget->currentIndex();

    QHash<QString, QVariant> simple_state;
    QHash<QString, QVariant> normal_state;
    QHash<QString, QVariant> advanced_state;

    simple_tab->save_state(simple_state);
    normal_tab->save_state(normal_state);
    advanced_tab->save_state(advanced_state);

    state["current_tab_index"] = QVariant(current_tab_index);
    state["simple_state"] = simple_state;
    state["normal_state"] = normal_state;
    state["advanced_state"] = advanced_state;
}

void FilterWidget::load_state(const QHash<QString, QVariant> &state) {
    const int current_tab_index = state["current_tab_index"].toInt();
    const QHash<QString, QVariant> simple_state = state["simple_state"].toHash();
    const QHash<QString, QVariant> normal_state = state["normal_state"].toHash();
    const QHash<QString, QVariant> advanced_state = state["advanced_state"].toHash();

    simple_tab->load_state(simple_state);
    normal_tab->load_state(normal_state);
    advanced_tab->load_state(advanced_state);

    tab_widget->setCurrentIndex(current_tab_index);
}
