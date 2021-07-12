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

#ifndef UPN_EDIT_H
#define UPN_EDIT_H

#include "edits/attribute_edit.h"

class QLineEdit;
class AdInterface;
class UpnSuffixWidget;

class UpnEdit final : public AttributeEdit {
    Q_OBJECT
public:
    UpnEdit(QList<AttributeEdit *> *edits_out, AdInterface &ad, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

    QString get_input() const;
    bool verify(AdInterface &ad, const QString &dn) const override;
    QLineEdit *get_edit() const;

private:
    QLineEdit *prefix_edit;
    UpnSuffixWidget *upn_suffix_widget;

    friend class StringOtherEdit;

    QString get_new_value() const;
};

#endif /* UPN_EDIT_H */
