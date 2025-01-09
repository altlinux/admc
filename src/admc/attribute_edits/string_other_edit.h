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

#ifndef STRING_OTHER_EDIT_H
#define STRING_OTHER_EDIT_H

#include "attribute_edits/attribute_edit.h"

class StringEdit;
class QPushButton;
class QLineEdit;

/**
 * Edit for attributes which have "other" version which is
 * multi-valued. For example "telephone" and
 * "otherTelephone". Contains a StringEdit for the attribute
 * itself and an "Other" button next to it which opens a
 * dialog through which other values are edited.
 */

class StringOtherEdit final : public AttributeEdit {
    Q_OBJECT
public:
    StringOtherEdit(QLineEdit *line_edit, QPushButton *other_button, const QString &main_attribute_arg, const QString &other_attribute_arg, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;

    // Sets read only property on both main edit and
    // edit dialog for "Other" values. Dialog for
    // "Other" values can still be opened to view them,
    // while read only is active.
    void set_read_only(const bool read_only);

    bool apply(AdInterface &ad, const QString &dn) const override;

private:
    StringEdit *main_edit;
    QPushButton *other_button;
    QLineEdit *line_edit;
    bool read_only;

    const QString other_attribute;
    QList<QByteArray> other_values;

    void on_other_button();
};

#endif /* STRING_OTHER_EDIT_H */
