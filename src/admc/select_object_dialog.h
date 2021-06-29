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

#ifndef SELECT_OBJECT_DIALOG_H
#define SELECT_OBJECT_DIALOG_H

#include <QDialog>

class QLineEdit;
class QTreeView;
class QStandardItemModel;
class SelectClassesWidget;
class AdObject;
class SelectBaseWidget;

enum SelectObjectDialogMultiSelection {
    SelectObjectDialogMultiSelection_Yes,
    SelectObjectDialogMultiSelection_No
};

class SelectObjectDialog final : public QDialog {
Q_OBJECT

public:
    SelectObjectDialog(const QList<QString> class_list_arg, const SelectObjectDialogMultiSelection multi_selection_arg, QWidget *parent);
    SelectObjectDialog(const QList<QString> class_list_arg, const SelectObjectDialogMultiSelection multi_selection_arg, const QString &default_base, QWidget *parent);

    QList<QString> get_selected() const;

public slots:
    void accept() override;

private:
    QStandardItemModel *model;
    QTreeView *view;
    QLineEdit *edit;
    SelectClassesWidget *select_classes;
    SelectBaseWidget *select_base_widget;
    QList<QString> class_list;
    SelectObjectDialogMultiSelection multi_selection;

    void on_add_button();
    void on_advanced_button();
    bool is_duplicate(const AdObject &object) const;
    void duplicate_message_box();
};

class SelectObjectMatchDialog final : public QDialog {
Q_OBJECT

public:
    SelectObjectMatchDialog(const QHash<QString, AdObject> &search_results, QWidget *parent);

    QList<QString> get_selected() const;

private:
    QTreeView *view;
};

#endif /* SELECT_OBJECT_DIALOG_H */
