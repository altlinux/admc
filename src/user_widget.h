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

#ifndef USER_WIDGET_H
#define USER_WIDGET_H

#include <QWidget>

class QString;

// Details tab with user-related stuff
class UserWidget final : public QWidget {
Q_OBJECT

public:
    UserWidget(QWidget *parent);

    void change_target(const QString &dn);

private slots:
    void on_reset_password_button();

private:
    QString target;
};

#endif /* USER_WIDGET_H */
