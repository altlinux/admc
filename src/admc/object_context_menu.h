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

/**
 * This menu should be setup to open by right clicking an
 * object. It is also used as the "Action" menu in the
 * menubar. 
 */

#include <QMenu>

class QAbstractItemView;

class ObjectContextMenu final : public QMenu {
Q_OBJECT

public:
    static void setup_as_context_menu(QAbstractItemView *view, const int dn_column);

    using QMenu::QMenu;
    void change_target(const QString &new_target);

protected:
    void showEvent(QShowEvent *event);

private:
    QString target;

    void details() const;
    void delete_object() const;
    void move() const;
    void add_to_group() const;
    void rename() const;
    void create(const QString &object_class) const;
    void reset_password() const;
    void enable_account() const;
    void disable_account() const;
};

#endif /* OBJECT_CONTEXT_MENU_H */
