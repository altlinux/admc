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

#ifndef OBJECT_CONTEXT_MENU_H
#define OBJECT_CONTEXT_MENU_H

#include "ad_interface.h"

#include <QMenu>

class QString;
class QPoint;
class QAbstractItemView;
class SelectDialog;

class ObjectContextMenu final : public QMenu {
Q_OBJECT

public:
    static void connect_view(QAbstractItemView *view, int dn_column);

private:
    ObjectContextMenu(const QString &dn);
    void delete_object(const QString &dn, const AttributesBinary &attributes);
    void edit_policy(const QString &dn);
    void move(const QString &dn, const AttributesBinary &attributes);
    void add_to_group(const QString &dn);

};

#endif /* OBJECT_CONTEXT_MENU_H */
