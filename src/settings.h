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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class QAction;

class Settings final : public QObject {
Q_OBJECT

public:
    explicit Settings(QObject *parent);

    QAction *toggle_advanced_view = nullptr;
    QAction *toggle_show_dn_column = nullptr;
    QAction *details_on_containers_click = nullptr;
    QAction *details_on_contents_click = nullptr;
    QAction *confirm_actions = nullptr;

private:
    void save_settings();

};

const Settings *SETTINGS();

#endif /* SETTINGS_H */
