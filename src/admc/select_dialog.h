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
class QStandardItemModel;
template <typename T> class QList;

enum SelectDialogMultiSelection {
    SelectDialogMultiSelection_Yes,
    SelectDialogMultiSelection_No
};

class SelectDialog final : public QDialog {
Q_OBJECT

public:
    SelectDialog(QList<QString> classes_arg, SelectDialogMultiSelection multi_selection, QWidget *parent);
    
    QList<QString> get_selected() const;

public slots:
    void accept();

private slots:
    void open_find_dialog();
    void remove_from_list();

private:
    QTreeView *view;
    QStandardItemModel *model;
    QList<QString> classes;
    SelectDialogMultiSelection multi_selection;


    void showEvent(QShowEvent *event);
};

#endif /* SELECT_DIALOG_H */
