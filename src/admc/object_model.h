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

#include "console_widget/console_widget.h"

class QStandardItem;
class AdObject;
template <typename T> class QList;

/**
 * Some f-ns used for models that store objects.
 */

enum ObjectRole {
    ObjectRole_DN = ConsoleRole_LAST + 1,
    ObjectRole_ObjectClasses = ConsoleRole_LAST + 2,
    ObjectRole_CannotMove = ConsoleRole_LAST + 3,
    ObjectRole_CannotRename = ConsoleRole_LAST + 4,
    ObjectRole_CannotDelete = ConsoleRole_LAST + 5,
    ObjectRole_AccountDisabled = ConsoleRole_LAST + 6,
    
    ObjectRole_LAST = ConsoleRole_LAST + 7,
};

void load_object_row(const QList<QStandardItem *> row, const AdObject &object);
void load_object_item_data(QStandardItem *item, const AdObject &object);
QList<QString> object_model_header_labels();
QList<int> object_model_default_columns();
QList<QString> object_model_search_attributes();

#endif /* OBJECT_MODEL_H */
