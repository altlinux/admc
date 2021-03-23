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

#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

/**
 * Find objects and perform actions on them.
 */

#include <QDialog>

class QString;
class QMenu;
template <typename T> class QList;

class FindDialog final : public QDialog {
Q_OBJECT

public:
    FindDialog(const QList<QString> classes, const QString 
        default_search_base, QWidget *parent);

private:
    QMenu *action_menu;

    void on_context_menu(const QPoint pos);
};

#endif /* FIND_DIALOG_H */
