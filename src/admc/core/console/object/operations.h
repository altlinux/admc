/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Artyom V. Poptsov
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

#ifndef CORE_CONSOLE_OBJECT_OPERATIONS_H
#define CORE_CONSOLE_OBJECT_OPERATIONS_H

#include <QHash>
#include <QList>
#include <QString>

#include "ad_interface.h"
#include "ad_object.h"

void object_update_move(AdInterface &ad,
                        const QHash<QString, AdObject> &object_map,
                        const QHash<QString, QString> &old_to_new_dn_map);
void object_update_delete(AdInterface &ad,
                          const QString &object_class,
                          const QString &target_dn);
QList<AdObject> object_search_all(AdInterface &ad,
                                  const QList<QString> &dn_list);
QString object_make_display_value(const AdObject &object,
                                  const QString &attribute);
QList<QString> object_column_labels();
bool object_should_be_in_scope(const AdObject &object);
bool object_is_site(const AdObject &object);

#endif  /* ifndef CORE_CONSOLE_OBJECT_OPERATIONS_H */
