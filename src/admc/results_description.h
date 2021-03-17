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

#ifndef RESULTS_DESCRIPTION_H
#define RESULTS_DESCRIPTION_H

#include <QList>
#include <QString>

class ResultsView;
class QWidget;

class ResultsDescription {

public:
    ResultsDescription(QWidget *widget, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns);

    QWidget *widget() const;
    ResultsView *view() const;
    QList<QString> column_labels() const;
    QList<int> default_columns() const;
    int column_count() const;

private:
    QWidget *m_widget;
    ResultsView *m_view;
    QList<QString> m_column_labels;
    QList<int> m_default_columns;
};

#endif /* RESULTS_DESCRIPTION_H */
