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

#ifndef FIND_SELECT_DIALOG_H
#define FIND_SELECT_DIALOG_H

/**
 * Find objects and select them. 
 */

#include <QDialog>

class FindWidget;
class QStandardItem;
template <typename T> class QList;

class FindSelectDialog final : public QDialog {
Q_OBJECT

public:
    FindSelectDialog(const QList<QString> classes, QWidget *parent);

    QList<QList<QStandardItem *>> get_selected_rows() const;

private:
    FindWidget *find_widget;
    
};

#endif /* FIND_SELECT_DIALOG_H */
