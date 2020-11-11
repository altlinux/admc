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

#ifndef FILTER_WIDGET_H
#define FILTER_WIDGET_H

/**
 * Allows user to enter a filter, which is then passed on to
 * a parent widget. Has tabs for different ways to enter a
 * filter: normal and advanced tab.
 */

#include <QWidget>
#include <QString>
#include <QList>

class QTabWidget;
class FilterWidgetTab;

class FilterWidget final : public QWidget {
Q_OBJECT

public:
    FilterWidget();

    QString get_filter() const;

private:
    QTabWidget *tab_widget;
    QList<FilterWidgetTab *> tabs;

    const FilterWidgetTab *get_current_tab() const;
};

class FilterWidgetTab : public QWidget {
public:
    virtual QString get_filter() const = 0;
};

#endif /* FILTER_WIDGET_H */
