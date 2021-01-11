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

#ifndef SELECT_DIALOG_H
#define SELECT_DIALOG_H

#include <QDialog>

class QTreeView;
class ObjectModel;
class QString;
template <typename T> class QList;

enum SelectDialogMultiSelection {
    SelectDialogMultiSelection_Yes,
    SelectDialogMultiSelection_No
};

class SelectDialog final : public QDialog {
Q_OBJECT

public:
    static QList<QString> open(QList<QString> classes, SelectDialogMultiSelection multi_selection_arg, const QString &title, QWidget *parent);

    QList<QString> get_selected() const;

private slots:
    void accept();
    void open_find_dialog();
    void remove_from_list();

private:
    QTreeView *view;
    ObjectModel *model;
    QList<QString> classes;
    SelectDialogMultiSelection multi_selection;

    SelectDialog(QList<QString> classes_arg, SelectDialogMultiSelection multi_selection, QWidget *parent);

    void showEvent(QShowEvent *event);
};

#endif /* SELECT_DIALOG_H */
