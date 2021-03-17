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

#include "results_description.h"

ResultsDescription::ResultsDescription(QTreeView *view_arg, const QList<QString> &column_labels_arg, const QList<int> &default_columns_arg) {
    view = view_arg;
    column_labels = column_labels_arg;
    default_columns = default_columns_arg;
}

QTreeView *ResultsDescription::get_view() const {
    return view;
}

QList<QString> ResultsDescription::get_column_labels() const {
    return column_labels;
}

QList<int> ResultsDescription::get_default_columns() const {
    return default_columns;
}

// NOTE: this is a bit round-about way of getting column
// count, but should be fine
int ResultsDescription::get_column_count() const {
    return column_labels.size();
}
