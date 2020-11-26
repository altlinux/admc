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

#ifndef MOVE_DIALOG_H
#define MOVE_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>

class QTreeView;

enum SelectDialogMultiSelection {
    SelectDialogMultiSelection_Yes,
    SelectDialogMultiSelection_No
};

class SelectDialog final : public QDialog {
Q_OBJECT

public:
    static QList<QString> open(QList<QString> classes, SelectDialogMultiSelection multi_selection, const QString &title, QWidget *parent);

private slots:
    void accept();

private:
    QTreeView *view;
    QList<QString> selected_objects;

    SelectDialog(QList<QString> classes, SelectDialogMultiSelection multi_selection, QWidget *parent);
};

#endif /* MOVE_DIALOG_H */
