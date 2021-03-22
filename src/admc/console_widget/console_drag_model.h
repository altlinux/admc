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

#ifndef CONSOLE_DRAG_MODEL_H
#define CONSOLE_DRAG_MODEL_H

#include <QStandardItemModel>

/**
 * Implements drag and drop. Note that this only implements
 * the framework for drag and drop. The actual logic is
 * implemented by console widget.
 */

class ConsoleDragModel : public QStandardItemModel {
Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

signals:
    // Save these indexes when drag is started
    void start_drag(const QList<QModelIndex> &dropped) const;

    // Set "ok" to true if can drop and false if can't drop
    // NOTE: "ok" is supposed to be set only once
    void can_drop(const QModelIndex &target, bool *ok) const;

    void drop(const QModelIndex &target);
};

#endif /* CONSOLE_DRAG_MODEL_H */
