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

#ifndef LIST_ATTRIBUTE_DIALOG_H
#define LIST_ATTRIBUTE_DIALOG_H

#include "attribute_dialogs/attribute_dialog.h"

class QListWidgetItem;

enum ListAttributeDialogType {
    ListAttributeDialogType_String,
    ListAttributeDialogType_Octet,
    ListAttributeDialogType_Datetime,
};

namespace Ui {
class ListAttributeDialog;
}

class ListAttributeDialog final : public AttributeDialog {
    Q_OBJECT

public:
    Ui::ListAttributeDialog *ui;

    ListAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent);
    ~ListAttributeDialog();

    void accept() override;

    QList<QByteArray> get_value_list() const override;

    // Sets max length for input of individual values
    // of the list attribute, if it's of string type.
    // Note that this *does not* apply to the list
    // attribute itself. By default the limit from
    // schema is used, but most list attributes are
    // unlimited.
    void set_value_max_length(const int max_length);

private:
    int max_length;

    void on_add_button();
    void on_remove_button();
    void add_value(const QByteArray value);
    QString bytes_to_string(const QByteArray bytes) const;
    QByteArray string_to_bytes(const QString string) const;
    ListAttributeDialogType get_type() const;
    void add_values_from_editor(AttributeDialog *editor);
    void edit_values_from_editor(AttributeDialog *editor);
};

#endif /* LIST_ATTRIBUTE_DIALOG_H */
