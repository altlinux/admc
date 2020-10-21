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
#include <QMap>

class DetailsTab;
class QLabel;
class QGridLayout;
class StringEdit;

class AttributeEdit : public QObject {
Q_OBJECT
public:
    using QObject::QObject;

    // Load original value
    virtual void load(const AdObject &object) = 0;

    // Reset input to original value
    virtual void reset() = 0;

    virtual void set_read_only(const bool read_only) = 0;
    virtual void add_to_layout(QGridLayout *layout) = 0;

    // Returns whether edit's value has been changed by the user
    // Edit should be applied only if it changed() 
    virtual bool changed() const = 0;

    // Check that current input is valid and show errors
    // to user in a warning message
    virtual bool verify() const = 0;

    // Apply current input by making a modification to the AD server
    virtual bool apply(const QString &dn) const = 0;

signals:
    // Emit this signal when user edits subwidget(s)
    // (by connecting to the widget's version of edited signal)
    // Also emit this at the end of reset() because widgets don't
    // always emit signals when they are edited programmatically.
    // For example QLineEdit::setText() does NOT emit 
    // QLineEdit::textChanged
    void edited();

protected:
    void connect_changed_marker(QLabel *label);
    void append_to_list(QList<AttributeEdit *> *edits_out);

};

#define DECL_ATTRIBUTE_EDIT_VIRTUALS()\
void load(const AdObject &object);\
void reset();\
void set_read_only(const bool read_only);\
void add_to_layout(QGridLayout *layout);\
bool changed() const;\
bool verify() const;\
bool apply(const QString &dn) const;


// Helper f-ns that iterate over edit lists for you
void edits_connect_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
void edits_connect_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
void edits_add_to_layout(QList<AttributeEdit *> edits, QGridLayout *layout);
bool edits_changed(QList<AttributeEdit *> edits);
bool edits_verify(QList<AttributeEdit *> edits);
bool edits_apply(QList<AttributeEdit *> edits, const QString &dn);
void edits_reset(QList<AttributeEdit *> edits);
void edits_load(QList<AttributeEdit *> edits, const AdObject &object);

#endif /* ATTRIBUTE_EDIT_H */
