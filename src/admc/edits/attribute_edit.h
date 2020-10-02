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

#include "ad_interface_defines.h"

#include <QObject>
#include <QString>
#include <QList>

class DetailsTab;
class QLabel;
class QGridLayout;

enum EditReadOnly {
    EditReadOnly_Yes,
    EditReadOnly_No
};

class AttributeEdit : public QObject {
Q_OBJECT
public:
    EditReadOnly read_only;

    using QObject::QObject;

    virtual void set_read_only(EditReadOnly read_only_arg);

    virtual void add_to_layout(QGridLayout *layout) = 0;

    // Load value from server for display
    // NOTE: block signals for subwidget(s) when setting values so that any outside connections to those widgets aren't triggered when not inteded
    virtual void load(const AttributesBinary &attributes) = 0;

    // Returns whether edit's value has been changed by the user
    // Resets on reload
    virtual bool changed() const = 0;

    // Check that current input is valid for conditions that can be checked without contacting the AD server, for example name input not being empty
    virtual bool verify_input(QWidget *parent) = 0;

    // Apply current input by making a modification to the AD server
    virtual bool apply(const QString &dn) = 0;

signals:
    // Emit this signal when user edits subwidget(s) (by connecting
    // to the widget's version of edited signal)
    // AND at the end of load(), so that changed marker is reset
    void edited();
};

#define DECL_ATTRIBUTE_EDIT_VIRTUALS()\
void add_to_layout(QGridLayout *layout);\
void load(const AttributesBinary &attributes);\
bool changed() const;\
bool verify_input(QWidget *parent);\
bool apply(const QString &dn);

void connect_changed_marker(AttributeEdit *edit, QLabel *label);
void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout);
void connect_edits_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
bool any_edits_changed(QList<AttributeEdit *> edits);

enum ApplyIfNotChanged {
    ApplyIfNotChanged_Yes,
    ApplyIfNotChanged_No
};

// Helper f-ns that iterate over edit lists for you
// Verify before applying!
void any_(QList<AttributeEdit *> edits, const QString &dn);
void load_attribute_edits(QList<AttributeEdit *> edits, const AttributesBinary &attributes);
bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent);
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, QObject *parent, const ApplyIfNotChanged apply_if_not_changed = ApplyIfNotChanged_No);

#endif /* ATTRIBUTE_EDIT_H */
