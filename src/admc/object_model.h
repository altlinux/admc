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

#ifndef OBJECT_MODEL_H
#define OBJECT_MODEL_H

#include <QStandardItemModel>
#include <QList>

class QMimeData;
class QModelIndex;
class QString;
class QStandardItem;
class AdObject;

/**
 * Some f-ns used for models that store objects.
 */

enum ObjectRole {
    Role_DN = Qt::UserRole + 1,
    Role_ObjectClass = Qt::UserRole + 2,
};

void load_object_row(const QList<QStandardItem *> row, const AdObject &object);
void load_object_item_data(QStandardItem *item, const AdObject &object);
QList<QString> object_model_header_labels();

#endif /* OBJECT_MODEL_H */
