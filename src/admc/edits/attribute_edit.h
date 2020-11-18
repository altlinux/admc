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

/**
 * AttributeEdit's wrap regular Qt widgets so that they can
 * be used to edit attributes of an AD object. Edits
 * themselves are wrapped in parent objects where they are
 * shown in groups. Edits are mostly used by DetailsTab's
 * and some dialogs. Depending on what kind of attribute is
 * being edited, different widgets may be used. For example,
 * QLineEdit for string attributes and QDateTimeEdit for
 * datetimes. At creation, edits are clear and out of sync
 * with the AD server. Calling load() loads the state of an
 * object, at which point edit state becomes equal to
 * current object state. load() is also used to reset back
 * to original state to cancel input or to load updated
 * state if the object changed. When user edits the edit
 * through the wrapped qt widgets, edit becomes out of sync
 * with current object state, which is represented as edit
 * being in the modified() state. Note that edit may exit
 * modified() state due to a load() call OR the user
 * manually undoing changes by retyping original string
 * value for example. When edit is modified(), it may be
 * applied, but verify() should be called first to show any
 * input erros. If verify() succeeds, apply() can be called
 * to apply changes to the object on the AD server. verify()
 * and apply() need to be two separate f-ns because for a
 * group of edits, all of them need to be verified first and
 * then all of them must be applied, so mixing verify()'s
 * and apply()'s wouldn't work. It is recommended to operate
 * on lists of edits, rather than singular edits. Typically,
 * an apply button on edit's parent object would call edit's
 * verify() and apply() f-ns and a cancel button would call
 * load().
 */

class DetailsTab;
class QFormLayout;

class AttributeEdit : public QObject {
Q_OBJECT
public:
    using QObject::QObject;

    // Load state from object
    // NOTE: emit edited() signal at the end when
    // implementing this f-n
    virtual void load(const AdObject &object) = 0;

    virtual void set_read_only(const bool read_only) = 0;

    // Layout all widgets that are part of this edit
    virtual void add_to_layout(QFormLayout *layout) = 0;

    // Returns whether edit's value has been edited by the user
    virtual bool modified() const = 0;

    // Check that current input is valid and show errors
    // to user in a warning message
    virtual bool verify() const = 0;

    // Apply current input by making a modification to the
    // AD server
    // NOTE: edit should be applied only if it's modified()
    virtual bool apply(const QString &dn) const = 0;

signals:
    // Emitted to notify parent object that edit was edited.
    void edited();

protected:
    void append_to_list(QList<AttributeEdit *> *edits_out);

};

#define DECL_ATTRIBUTE_EDIT_VIRTUALS()\
void load(const AdObject &object);\
void set_read_only(const bool read_only);\
void add_to_layout(QFormLayout *layout);\
bool modified() const;\
bool verify() const;\
bool apply(const QString &dn) const;


// Helper f-ns that iterate over edit lists for you
void edits_connect_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
void edits_connect_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
void edits_add_to_layout(QList<AttributeEdit *> edits, QFormLayout *layout);
bool edits_modified(QList<AttributeEdit *> edits);
bool edits_verify(QList<AttributeEdit *> edits);
bool edits_apply(QList<AttributeEdit *> edits, const QString &dn);
void edits_load(QList<AttributeEdit *> edits, const AdObject &object);

#endif /* ATTRIBUTE_EDIT_H */
