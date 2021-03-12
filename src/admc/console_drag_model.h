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

/**
 * This model doesn't implement specific drag drop behavior
 * but instead allows the user of the model to implement.
 * User of the model should set the drop functions as well
 * as connect to the drop() signal.
 */

#include <QStandardItemModel>

typedef QMimeData *(*FunMimeData)(const QList<QModelIndex> &indexes);
typedef bool (*FunCanDrop)(const QMimeData *data, const QModelIndex &parent);

class ConsoleDragModel : public QStandardItemModel {
Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    // This function should construct the mimedata from
    // model indexes
    void set_fun_mime_data(FunMimeData fun);

    // This function should return whether given mimedata
    // can be dropped on given model index
    void set_fun_can_drop(FunCanDrop fun);

    QMimeData *mimeData(const QList<QModelIndex> &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

signals:
    // Connect to this slot and perform the drop operation
    // in the slot (if it's possible)
    void drop(const QMimeData *data, const QModelIndex &parent);

private:
    FunMimeData fun_mime_data = nullptr; 
    FunCanDrop fun_can_drop = nullptr; 
};

#endif /* CONSOLE_DRAG_MODEL_H */
