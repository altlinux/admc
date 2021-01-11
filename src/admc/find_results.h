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

#ifndef FIND_RESULTS_H
#define FIND_RESULTS_H

/**
 * Used by find dialog to display find results as a list of
 * objects.
 */

#include <QWidget>

class QTreeView;
class QLabel;
class QStandardItemModel;
class QStandardItem;
template <typename T> class QList;

class FindResults final : public QWidget {
Q_OBJECT

public:
    QTreeView *view;
    
    FindResults();

    void load(const QString &filter, const QString &search_base);

    // Returns a copy of item's rows, so these items need to become owned by something or deleted
    QList<QList<QStandardItem *>> get_selected_rows() const;

private:
    QStandardItemModel *model;
    QLabel *object_count_label;

    void showEvent(QShowEvent *event);
};

#endif /* FIND_RESULTS_H */
