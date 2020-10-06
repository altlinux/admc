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

#ifndef ATTRIBUTE_EDIT_H
#define ATTRIBUTE_EDIT_H

#include "ad_object.h"

#include <QObject>
#include <QString>
#include <QList>

class DetailsTab;
class QLabel;
class QGridLayout;

class AttributeEdit : public QObject {
Q_OBJECT
public:
    using QObject::QObject;

    virtual void set_read_only(const bool read_only) = 0;
    virtual void add_to_layout(QGridLayout *layout) = 0;

    // Returns whether edit's value has been changed by the user
    // Edit should be applied only if it changed() 
    virtual bool changed() const = 0;

    // Check that current input is valid for conditions that can be checked without contacting the AD server, for example name input not being empty
    virtual bool verify_input(QWidget *parent) = 0;

    // Apply current input by making a modification to the AD server
    virtual bool apply(const QString &dn) = 0;

signals:
    // Emit this signal when user edits subwidget(s)
    // (by connecting to the widget's version of edited signal)
    void edited();
};

#define DECL_ATTRIBUTE_EDIT_VIRTUALS()\
void set_read_only(const bool read_only);\
void add_to_layout(QGridLayout *layout);\
bool changed() const;\
bool verify_input(QWidget *parent);\
bool apply(const QString &dn);

void connect_changed_marker(AttributeEdit *edit, QLabel *label);
void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout);
void connect_edits_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
bool any_edits_changed(QList<AttributeEdit *> edits);

// Helper f-ns that iterate over edit lists for you
// Verify before applying!
bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent);
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn);

#endif /* ATTRIBUTE_EDIT_H */
