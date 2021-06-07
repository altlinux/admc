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

#ifndef FILTER_WIDGET_H
#define FILTER_WIDGET_H

/**
 * Allows user to enter a filter, which is then passed on to
 * a parent widget. Has tabs for different ways to enter a
 * filter: normal and advanced tab.
 */

#include <QWidget>

class QTabWidget;
class QString;
class FilterWidgetTab;
class FilterWidgetSimpleTab;
class FilterWidgetNormalTab;
class FilterWidgetAdvancedTab;

class FilterWidget final : public QWidget {
Q_OBJECT

public:
    FilterWidget(const QList<QString> classes);

    QString get_filter() const;

    void save_state(QHash<QString, QVariant> &state) const;
    void load_state(const QHash<QString, QVariant> &state);

private:
    QTabWidget *tab_widget;
    FilterWidgetSimpleTab *simple_tab;
    FilterWidgetNormalTab *normal_tab;
    FilterWidgetAdvancedTab *advanced_tab;
};

class FilterWidgetTab : public QWidget {
Q_OBJECT

public:
    virtual QString get_filter() const = 0;
};

#endif /* FILTER_WIDGET_H */
