/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

class QStandardItemModel;
class AdObject;
class AdInterface;

namespace Ui {
class SelectObjectDialog;
}

enum SelectObjectDialogMultiSelection {
    SelectObjectDialogMultiSelection_Yes,
    SelectObjectDialogMultiSelection_No
};

struct SelectedObjectData {
    QString dn;
    QString category;
};

class SelectObjectDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::SelectObjectDialog *ui;

    SelectObjectDialog(const QList<QString> class_list_arg, const SelectObjectDialogMultiSelection multi_selection_arg, QWidget *parent);
    ~SelectObjectDialog();

    static QList<QString> header_labels();

    QList<QString> get_selected() const;
    QList<SelectedObjectData> get_selected_advanced() const;

public slots:
    void accept() override;

private:
    QStandardItemModel *model;
    QList<QString> class_list;
    SelectObjectDialogMultiSelection multi_selection;

    void on_add_button();
    void on_remove_button();
    void add_objects_to_list(const QList<QString> &dn_list);
    void add_objects_to_list(const QList<QString> &dn_list, AdInterface &ad);
    void open_advanced_dialog();
};

void add_select_object_to_model(QStandardItemModel *model, const AdObject &object);

#endif /* SELECT_OBJECT_DIALOG_H */
