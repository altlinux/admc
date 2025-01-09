/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "console_widget/console_drag_model.h"

#include "console_widget/console_widget.h"
#include "console_widget/console_widget_p.h"

#include <QMimeData>

#define MIME_TYPE_CONSOLE "MIME_TYPE_CONSOLE"

QModelIndex prev_parent = QModelIndex();
bool drag_start = false;

ConsoleDragModel::ConsoleDragModel(ConsoleWidget *console_arg)
: QStandardItemModel(console_arg) {
    console = console_arg;
}

QMimeData *ConsoleDragModel::mimeData(const QModelIndexList &indexes) const {
    const QList<QPersistentModelIndex> main_indexes = [&]() {
        QList<QPersistentModelIndex> out;

        for (const QModelIndex &index : indexes) {
            if (index.column() == 0) {
                out.append(QPersistentModelIndex(index));
            }
        }

        return out;
    }();

    console->d->start_drag(main_indexes);

    auto data = new QMimeData();

    // NOTE: even though we don't store any data in
    // mimedata, set empty data with our type so that we can
    // reject any mimedata not created in console drag model
    // by checking if mimedata has our mime type.
    data->setData(MIME_TYPE_CONSOLE, QByteArray());

    drag_start = true;

    return data;
}

bool ConsoleDragModel::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    if (!data->hasFormat(MIME_TYPE_CONSOLE)) {
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
    // hovered over empty space. Also return true for the
    // first canDrop() call when drag just started.
    // https://bugreports.qt.io/browse/QTBUG-76418
    if (prev_parent != parent || parent == QModelIndex() || drag_start) {
        prev_parent = parent;
        drag_start = false;

        return true;
    }

    // NOTE: using index at 0th column because that column
    // has the item roles
    const QModelIndex target = parent.siblingAtColumn(0);

    const bool can_drop = console->d->can_drop(target);

    return can_drop;
}

bool ConsoleDragModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &parent) {
    UNUSED_ARG(action);

    if (!data->hasFormat(MIME_TYPE_CONSOLE)) {
        return false;
    }

    // NOTE: using index at 0th column because that column
    // has the item roles
    const QModelIndex target = parent.siblingAtColumn(0);

    console->d->drop(target);

    return true;
}
