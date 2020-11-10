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

#ifndef FILTER_WIDGET_NORMAL_TAB_H
#define FILTER_WIDGET_NORMAL_TAB_H

/**
 * Allows user to enter a filter in a structured way,
 * without requiring knowledge of LDAP filter syntax.
 * Presents possible attributes and possible filter
 * conditions. User can build filters and add them a to a
 * list. The list is composed into one single LDAP filter
 * string.
 */

#include "filter_widget/filter_widget.h"

#include <QString>
#include <QSet>

class QComboBox;
class QLineEdit;
class QListWidget;

class FilterWidgetNormalTab final : public FilterWidgetTab {
Q_OBJECT

public:
    FilterWidgetNormalTab();

    QString get_filter() const;

private slots:
    void on_attribute_class_combo();
    void on_add_filter();
    void on_select_classes();
    void on_remove_filter();
    void on_clear_filters();

private:
    QTabWidget *tab_widget;
    QWidget *normal_tab;
    QWidget *advanced_tab;
    QComboBox *attribute_class_combo;
    QComboBox *attribute_combo;
    QComboBox *condition_combo;
    QLineEdit *value_edit;
    QListWidget *filter_list;
    QLineEdit *selected_classes_display;

    QSet<QString> selected_search_classes;
};

#endif /* FILTER_WIDGET_NORMAL_TAB_H */
