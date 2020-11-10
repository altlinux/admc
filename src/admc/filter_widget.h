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
 * a parent widget. Has two tabs for different ways to enter
 * a filter. The normal tab shows all possible filter
 * parameters, from which a filter is built. Filters are
 * AND'ed together into one big filter. The advanced tab
 * allows to enter the LDAP filter string directly.
 */

// TODO: for now just AND'ing filters, somehow allow OR'ing?

#include <QWidget>
#include <QString>
#include <QSet>

class QPlainTextEdit;
class QTabWidget;
class QComboBox;
class QLineEdit;
class QListWidget;

class FilterWidget final : public QWidget {
Q_OBJECT

public:
    FilterWidget();

    QString get_filter() const;
    QString get_search_base() const;

signals:
    void changed();

private slots:
    void on_class_combo();
    void on_custom_search_base();
    void on_add_filter();
    void on_select_classes();

private:
    QTabWidget *tab_widget;
    QWidget *normal_tab;
    QWidget *advanced_tab;
    QComboBox *attributes_combo;
    QComboBox *class_combo;
    QComboBox *search_base_combo;
    QComboBox *condition_combo;
    QLineEdit *value_edit;
    QListWidget *filter_list;

    QPlainTextEdit *ldap_filter_edit;

    QSet<QString> selected_search_classes;
};

#endif /* FILTER_WIDGET_H */
