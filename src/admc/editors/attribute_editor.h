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

#ifndef ATTRIBUTE_EDITOR_H
#define ATTRIBUTE_EDITOR_H

#include <QDialog>

class QLabel;

/**
 * Gets input from user, which can be obtained through
 * get_value_list(). Different from AttributeEdit because it
 * is opened as a separate dialog and parent object is
 * responsible for actually applying the changes.
 */

class AttributeEditor : public QDialog {
    Q_OBJECT

public:
    using QDialog::QDialog;

    // Store attribute and also configure widget settings so
    // that they are appropriate for given attribute.
    virtual void set_attribute(const QString &attribute);

    // NOTE: this exists so that it's possible to make a
    // read only editor for writable attributes
    virtual void set_read_only(const bool read_only);

    QString get_attribute() const;
    bool get_read_only() const;

    virtual void set_value_list(const QList<QByteArray> &value_list) = 0;

    // Returns current value list, which includes
    // modifications made by the user
    virtual QList<QByteArray> get_value_list() const = 0;

protected:
    // The text of this label will be updated when attribute
    // changes. Call this in ctor of subclass
    void set_attribute_label(QLabel *attribute_label);

private:
    QString m_attribute;
    bool m_read_only;
    QLabel *m_attribute_label;
};

#endif /* ATTRIBUTE_EDITOR_H */
