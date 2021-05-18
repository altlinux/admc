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

#include <QDebug>
#include <QTabWidget>
#include <QVBoxLayout>

FilterWidget::FilterWidget(const QList<QString> classes)
: QWidget()
{
    tab_widget = new QTabWidget();

    auto add_tab =
    [this](FilterWidgetTab *tab, const QString &title) {
        tab_widget->addTab(tab, title);

        connect(
            tab, &FilterWidgetTab::changed,
            [this]() {
                emit changed();
            });
    };

    simple_tab = new FilterWidgetSimpleTab(classes);
    add_tab(simple_tab, tr("Simple"));
    add_tab(new FilterWidgetNormalTab(classes), tr("Normal"));
    add_tab(new FilterWidgetAdvancedTab(), tr("Advanced"));

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

void FilterWidget::serialize(QDataStream &stream) const {
    stream << simple_tab;
}

void FilterWidget::deserialize(QDataStream &stream) {
    stream >> simple_tab;
}

QDataStream &operator<<(QDataStream &stream, const FilterWidget *widget) {
    widget->serialize(stream);

    return stream;
}

QDataStream &operator>>(QDataStream &stream, FilterWidget *widget) {
    widget->deserialize(stream);
    
    return stream;
}
