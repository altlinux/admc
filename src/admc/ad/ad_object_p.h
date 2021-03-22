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

#ifndef AD_OBJECT_P_H
#define AD_OBJECT_P_H

#include <QString>
#include <QHash>
#include <QList>
#include <QByteArray>

class AdObjectPrivate {

public:
    AdObjectPrivate();
    
    QString dn;
    QHash<QString, QList<QByteArray>> attributes_data;
};

#endif /* AD_OBJECT_P_H */
