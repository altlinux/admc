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

#ifndef AD_H
#define AD_H

#include <QHash>
#include <QList>
#include <QString>

#include "ad_defines.h"
#include "ad_object.h"

QHash<QString, AdObject> ad_search_objects(AdInterface &ad,
                                           const QList<QString> list);
void ad_add_members_to_groups(AdInterface &ad,
                              const QList<QString> &targets,
                              const QList<QString> &groups);
QHash<QString, QString> ad_select_changed_dn(QHash<QString, QString> map);
bool ad_object_is_person(const AdObject &object);
QString ad_current_dc_dns_host_name(AdInterface &ad);
int ad_get_range_upper(const QString &attribute);

#endif  /* ifndef AD_H */
