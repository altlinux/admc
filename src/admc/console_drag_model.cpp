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

#include "ad_interface.h"
#include "ad_object.h"
#include "status.h"
#include "object_model.h"

#include <QMimeData>
#include <QString>

// TODO: dragging queries and query folders

QModelIndex prev_parent = QModelIndex();

// TODO: when implementing console widget, this should be
// removed. The replacement will be to store node id's in
// mimedata so that objects can be obtained directly from
// models
QMimeData *ConsoleDragModel::mimeData(const QModelIndexList &indexes) const {
    auto data = new QMimeData();

    // Get adobject's from item data and convert the list to
    // a variant
    const QList<QVariant> objects =
    [=]() {
        QList<QVariant> out;
        for (const QModelIndex index : indexes) {
            if (index.column() == 0) {
                const QVariant object_variant = index.data(ObjectRole_AdObject);
                out.append(object_variant);
            }
        }

        return out;
    }();
    const QVariant objects_as_variant(objects);

    // Convert objects variant to bytes
    QByteArray objects_as_bytes;
    QDataStream stream(&objects_as_bytes, QIODevice::WriteOnly);
    stream << objects_as_variant;
    
    data->setData(MIME_TYPE_OBJECT, objects_as_bytes);

    return data;
}

bool ConsoleDragModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
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

    bool ok;
    emit can_drop(data, parent, &ok);

    return ok;
}

bool ConsoleDragModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &parent) {
    emit drop(data, parent);

    return true;
}

QList<AdObject> mimedata_to_object_list(const QMimeData *data) {
    // Convert from bytes to variant
    QByteArray objects_as_bytes = data->data(MIME_TYPE_OBJECT);
    QDataStream stream(&objects_as_bytes, QIODevice::ReadOnly);
    QVariant objects_as_variant;
    stream >> objects_as_variant;

    // Convert from variant to list of objects
    const QList<QVariant> objects_as_variant_list = objects_as_variant.toList();

    QList<AdObject> out;

    for (const QVariant &e : objects_as_variant_list) {
        const AdObject object = e.value<AdObject>();
        out.append(object);
    }

    return out;
}
