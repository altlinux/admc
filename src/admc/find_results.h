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

#ifndef FIND_RESULTS_H
#define FIND_RESULTS_H

/**
 * Used by find dialog to display find results as a list of
 * objects.
 */

#include <QWidget>

#include <QPersistentModelIndex>

class QStandardItem;
class QMenu;
class AdObject;
class ConsoleWidget;

class FindResults final : public QWidget {
    Q_OBJECT

public:
    FindResults();
    ~FindResults();

    void clear();

    // Append results to list and re-sort
    void load(const QHash<QString, AdObject> &results);

    // NOTE: returned items need to be re-parented or deleted!
    QList<QList<QStandardItem *>> get_selected_rows() const;

    void add_actions(QMenu *action_menu, QMenu *view_menu);

private:
    ConsoleWidget *console;
    QPersistentModelIndex head_index;
};

#endif /* FIND_RESULTS_H */
