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

#include "tab_widget.h"

#include <QStackedWidget>
#include <QListWidget>
#include <QHBoxLayout>

TabWidget::TabWidget()
: QWidget()
{
    list_widget = new QListWidget();
    list_widget->setMinimumWidth(50);
    list_widget->setMaximumWidth(150);

    stacked_widget = new QStackedWidget();
    stacked_widget->setFrameStyle(QFrame::Raised);
    stacked_widget->setFrameShape(QFrame::Box);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->addWidget(list_widget);
    layout->addWidget(stacked_widget);

    connect(
        list_widget, &QListWidget::currentRowChanged,
        [this](int index) {
            stacked_widget->setCurrentIndex(index);
        });
}

void TabWidget::add_tab(QWidget *tab, const QString &title) {
    list_widget->addItem(title);
    stacked_widget->addWidget(tab);
}
