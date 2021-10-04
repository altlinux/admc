/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef WIDGET_STATE_H
#define WIDGET_STATE_H

/**
 * Saves/restores state of all the widgets in the list.
 */

class WidgetStatePrivate;
template<typename>
class QList;
class QWidget;

class WidgetState final {
public:
    WidgetState();
    ~WidgetState();

    void set_widget_list(const QList<QWidget *> &widget_list);
    void save();
    void restore();

private:
    WidgetStatePrivate *d;
};

#endif /* WIDGET_STATE_H */
