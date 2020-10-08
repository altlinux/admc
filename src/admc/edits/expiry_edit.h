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

#ifndef EXPIRY_EDIT_H
#define EXPIRY_EDIT_H

#include "edits/attribute_edit.h"

#include <QString>

class QCheckBox;
class QLabel;
class QPushButton;

class ExpiryEdit final : public AttributeEdit {
Q_OBJECT
public:
    ExpiryEdit(QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private slots:
    void on_never_check();
    void on_end_of_check();
    void on_edit_button();

private:
    QCheckBox *never_check;
    QCheckBox *end_of_check;
    QLabel *display_label;
    QPushButton *edit_button;
    QString original_value;

    QString get_new_value() const;
};

#endif /* EXPIRY_EDIT_H */
