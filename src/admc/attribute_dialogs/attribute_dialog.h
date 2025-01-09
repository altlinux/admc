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

#ifndef ATTRIBUTE_DIALOG_H
#define ATTRIBUTE_DIALOG_H

#include <QDialog>

class QLabel;

/**
 * Dialog for viewing and editing attribute values.
 */

class AttributeDialog : public QDialog {
    Q_OBJECT

public:
    static AttributeDialog *make(const QString &attribute, const QList<QByteArray> &value_list, const bool read_only, const bool single_valued, QWidget *parent);

    AttributeDialog(const QString &attribute, const bool read_only, QWidget *parent);

    QString get_attribute() const;
    bool get_read_only() const;

    // Returns current value list, which includes
    // modifications made by the user
    virtual QList<QByteArray> get_value_list() const = 0;

protected:
    // Load text of attribute label based on attribute
    void load_attribute_label(QLabel *attribute_label);

private:
    QString m_attribute;
    bool m_read_only;
};

#endif /* ATTRIBUTE_DIALOG_H */
