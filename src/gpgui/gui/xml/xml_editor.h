/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
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

#ifndef XML_EDITOR_H
#define XML_EDITOR_H

#include "xml_attribute.h"

#include <QDialog>
#include <QString>
#include <QList>
#include <QHash>
#include <QDomDocument>

class XmlEdit;
class QPushButton;

class XmlEditor final : public QDialog {
Q_OBJECT

public:
    static QList<XmlAttribute> schema_attributes;
    static QHash<QString, XmlAttribute> schema_attributes_by_name;

    XmlEditor(const QString &path_arg);

    static void load_schema();

private slots:
    void enable_buttons_if_changed();

private:
    QString path;
    QList<XmlEdit *> edits;
    QPushButton *apply_button;
    QPushButton *reset_button;

    void reload();
    bool apply();
};

#endif /* XML_EDITOR_H */
