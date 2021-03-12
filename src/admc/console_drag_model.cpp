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

#include "console_drag_model.h"

#include <QDebug>
#include <QMimeData>

QModelIndex prev_parent = QModelIndex();

void ConsoleDragModel::set_fun_mime_data(FunMimeData fun) {
    fun_mime_data = fun;
}

void ConsoleDragModel::set_fun_can_drop(FunCanDrop fun) {
    fun_can_drop = fun;
}

QMimeData *ConsoleDragModel::mimeData(const QList<QModelIndex> &indexes) const {
    if (fun_mime_data != nullptr) {
        auto data = fun_mime_data(indexes);

        return data;
    } else {
        qDebug() << "ConsoleDragModel::mimeData() f-n ptr is unset";
        
        return new QMimeData();
    }
}

bool ConsoleDragModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    if (fun_can_drop == nullptr) {
        qDebug() << "ConsoleDragModel::canDropMimeData() f-n ptr is unset";

        return false;
    }

    // NOTE: this is a bandaid for a Qt bug. The bug causes
    // canDropMimeData() to stop being called for a given
    // view if canDropMimeData() returns false for the first
    // call when drag is entering the view. This results in
    // the drop indicator being stuck in "can't drop" state
    // until dragging leaves the view. The bandaid fix is to
    // always return true from canDropMimeData() when parent
    // index changes (when hovering over new object). Also
    // return true for invalid index for cases where drag is
    // hovered over empty space.
    // https://bugreports.qt.io/browse/QTBUG-76418
    if (prev_parent != parent || parent == QModelIndex()) {
        prev_parent = parent;
        return true;
    }

    const bool can_drop = fun_can_drop(data, parent);

    return can_drop;
}

bool ConsoleDragModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &parent) {
    emit drop(data, parent);

    return true;
}
