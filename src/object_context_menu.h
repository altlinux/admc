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
class MoveDialog;

class ObjectContextMenu final : public QMenu {
Q_OBJECT

public:
    ObjectContextMenu(QWidget *parent);

    void connect_view(QAbstractItemView *view, int dn_column);

signals:
    void details(const QString &dn);
    
private:
    MoveDialog *move_dialog = nullptr;
    
    void open(const QPoint &global_pos, const QString &dn, const QString &parent_dn);
    void delete_entry(const QString &dn);
    void new_entry_dialog(const QString &parent_dn, NewEntryType type);
    void new_user(const QString &dn);
    void new_computer(const QString &dn);
    void new_group(const QString &dn);
    void new_ou(const QString &dn);
    void rename(const QString &dn);
    void edit_policy(const QString &dn);

};

#endif /* OBJECT_CONTEXT_MENU_H */
