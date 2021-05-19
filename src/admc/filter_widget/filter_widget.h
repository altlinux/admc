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

class QTabWidget;
class QString;
class FilterWidgetTab;
class FilterWidgetSimpleTab;

class FilterWidget final : public QWidget {
Q_OBJECT

public:
    FilterWidget(const QList<QString> classes);

    QString get_filter() const;

    void serialize(QDataStream &stream) const;
    void deserialize(QDataStream &stream);

private:
    QTabWidget *tab_widget;
    FilterWidgetSimpleTab *simple_tab;
};

class FilterWidgetTab : public QWidget {
Q_OBJECT

public:
    virtual QString get_filter() const = 0;
    
    virtual void serialize(QDataStream &stream) const = 0;
    virtual void deserialize(QDataStream &stream) = 0;
};

QDataStream &operator<<(QDataStream &stream, const FilterWidget *widget);
QDataStream &operator>>(QDataStream &stream, FilterWidget *widget);

QDataStream &operator<<(QDataStream &stream, const FilterWidgetTab *widget);
QDataStream &operator>>(QDataStream &stream, FilterWidgetTab *widget);

#endif /* FILTER_WIDGET_H */
