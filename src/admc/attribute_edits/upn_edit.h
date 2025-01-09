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

#ifndef UPN_EDIT_H
#define UPN_EDIT_H

#include "attribute_edits/attribute_edit.h"

class QLineEdit;
class AdInterface;
class QComboBox;

class UpnEdit final : public AttributeEdit {
    Q_OBJECT
public:
    UpnEdit(QLineEdit *prefix_edit, QComboBox *suffix_combo, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool verify(AdInterface &ad, const QString &dn) const override;
    bool apply(AdInterface &ad, const QString &dn) const override;

    void init_suffixes(AdInterface &ad);

private:
    QLineEdit *prefix_edit;
    QComboBox *upn_suffix_combo;

    friend class StringOtherEdit;

    QString get_new_value() const;
    void on_suffix_combo_changed();
    void update_prefix_limit();
};

#endif /* UPN_EDIT_H */
