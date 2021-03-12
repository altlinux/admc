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

#ifndef OBJECT_DRAG_H
#define OBJECT_DRAG_H

class AdInterface;
class AdObject;
class QMimeData;
class QModelIndex;
template <typename T> class QList;

#define MIME_TYPE_OBJECT "MIME_TYPE_OBJECT"

void object_drop(AdInterface &ad, const AdObject &dropped, const AdObject &target);
QMimeData *object_mime_data(const QList<QModelIndex> &indexes);
bool object_can_drop(const QMimeData *data, const QModelIndex &parent);
QList<AdObject> mimedata_to_object_list(const QMimeData *data);

#endif /* OBJECT_DRAG_H */
