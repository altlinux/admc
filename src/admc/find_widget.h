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

#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

/**
 * Provides a way for user to find objects. FilterWidget is
 * used for filter input and FindResults for displaying
 * objects. Used by FindDialog and SelectDialog.
 */

#include <QWidget>

class FilterWidget;
class FindResults;
class QComboBox;
class QStandardItem;
template <typename T> class QList;

class FindWidget final : public QWidget {
Q_OBJECT

public:
    FindResults *find_results;
    
    FindWidget(const QList<QString> classes, const QString &default_search_base);

    QList<QList<QStandardItem *>> get_selected_rows() const;

private slots:
    void select_custom_search_base();
    void on_filter_changed();
    void find();

private:
    FilterWidget *filter_widget;
    QComboBox *search_base_combo;
};

#endif /* FIND_WIDGET_H */
