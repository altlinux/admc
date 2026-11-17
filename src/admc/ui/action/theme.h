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

/**
 * @file theme.h
 * @brief Icon theme menu action.
 */

#ifndef THEME_ACTION_H
#define THEME_ACTION_H

#include <QAction>
#include <QActionGroup>

class ThemeAction : public QAction {
    Q_OBJECT

public:
    ThemeAction(const QString &name, QActionGroup *group);
    bool has_localized_name() const;

private:
    bool m_has_localized_name;
};

#endif  // ifndef THEME_ACTION_H
