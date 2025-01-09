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

#ifndef UPN_MULTI_EDIT_H
#define UPN_MULTI_EDIT_H

/**
 * Edit for editing UPN suffixes of multiple objects.
 * This special version is needed because when editing
 * multiple objects, only the suffix is edited, so
 * normal UpnEdit doesn't work for this.
 */

#include "attribute_edits/attribute_edit.h"

class QComboBox;

class UpnMultiEdit final : public AttributeEdit {
    Q_OBJECT
public:
    UpnMultiEdit(QComboBox *upn_suffix_combo, AdInterface &ad, QObject *parent);

    bool apply(AdInterface &ad, const QString &target) const override;
    void set_enabled(const bool enabled) override;

private:
    QComboBox *upn_suffix_combo;
};

#endif /* UPN_MULTI_EDIT_H */
