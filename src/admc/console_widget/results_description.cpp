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

#include "console_widget/results_description.h"

// NOTE: empty ctor is needed by QHash
ResultsDescription::ResultsDescription() {
    m_widget = nullptr;
    m_view = nullptr;
}

ResultsDescription::ResultsDescription(QWidget *widget_arg, ResultsView *view_arg, const QList<QString> &column_labels_arg, const QList<int> &default_columns_arg) {
    m_widget = widget_arg;
    m_view = view_arg;
    m_column_labels = column_labels_arg;
    m_default_columns = default_columns_arg;
}

QWidget *ResultsDescription::widget() const {
    return m_widget;
}

ResultsView *ResultsDescription::view() const {
    return m_view;
}

QList<QString> ResultsDescription::column_labels() const {
    return m_column_labels;
}

QList<int> ResultsDescription::default_columns() const {
    return m_default_columns;
}

// NOTE: this is a bit round-about way of getting column
// count, but should be fine
int ResultsDescription::column_count() const {
    return m_column_labels.size();
}
