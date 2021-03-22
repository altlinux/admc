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

#include "console_widget/console_widget.h"

class QMimeData;
class QModelIndex;
class QString;
class QStandardItem;
class AdObject;

/**
 * Some f-ns used for models that store objects.
 */

enum ObjectRole {
    ObjectRole_DN = ConsoleRole_LAST + 1,
    ObjectRole_ObjectClasses = ConsoleRole_LAST + 2,
    ObjectRole_LAST = ConsoleRole_LAST + 3,
};

void load_object_row(const QList<QStandardItem *> row, const AdObject &object);
void load_object_item_data(QStandardItem *item, const AdObject &object);
QList<QString> object_model_header_labels();
QList<int> object_model_default_columns();

#endif /* OBJECT_MODEL_H */
