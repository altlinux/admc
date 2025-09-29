/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
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

#ifndef LAPS_ENCRYPED_ATTRIBUTE_EDIT_H
#define LAPS_ENCRYPED_ATTRIBUTE_EDIT_H

#include "attribute_edits/attribute_edit.h"

class QLineEdit;
class QJsonDocument;

class LAPSEncryptedAttributeEdit final : public AttributeEdit {
    Q_OBJECT
public:
    LAPSEncryptedAttributeEdit(QLineEdit *edit_arg, const QString &attribute_arg, const QString &json_field_arg, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;
    void set_enabled(const bool enabled) override;

signals:
    void show_error_dialog() const;

private:
    QLineEdit *edit;
    QString attribute;
    QString json_field;

    friend class StringOtherEdit;

    QJsonDocument get_jsondocument_from_attribute_value(AdInterface &ad, const AdObject &object, const QString &attribute_name) const;
    QByteArray create_attribute_value_from_jsondocument(AdInterface &ad, const QJsonDocument* document) const;
    char* get_default_principal_name() const;

};

#endif /* LAPS_ENCRYPED_ATTRIBUTE_EDIT_H */
