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

#include "ad_interface.h"

#include <QString>
#include <QList>
#include <QGridLayout>
#include <QMap>

class QCheckBox;
class QLineEdit;
class QComboBox;
class AttributeEdit;
class StringEdit;
class AccountOptionEdit;
class QCalendarWidget;
class QDateTimeEdit;
class DetailsTab;
class QLabel;

enum EditReadOnly {
    EditReadOnly_Yes,
    EditReadOnly_No
};

void setup_edit_marker(AttributeEdit *edit, QLabel *label);
void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout);
void connect_edits_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);
bool any_edits_changed(QList<AttributeEdit *> edits);

QMap<AccountOption, AccountOptionEdit *> make_account_option_edits(const QList<AccountOption> options, QWidget *parent);
void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out);

// Helper f-ns that iterate over edit lists for you
// Verify before applying!
void any_(QList<AttributeEdit *> edits, const QString &dn);
void load_attribute_edits(QList<AttributeEdit *> edits, const QString &dn);
bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent);
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, QWidget *parent);

void autofill_full_name(QMap<QString, StringEdit *> string_edits);
void autofill_sama_name(StringEdit *sama_edit, StringEdit *name_edit);

class AttributeEdit : public QObject {
Q_OBJECT
public:
    EditReadOnly read_only;

    AttributeEdit();
    AttributeEdit(EditReadOnly read_only_arg);

    virtual void add_to_layout(QGridLayout *layout) = 0;

    // Load value from server for display
    virtual void load(const QString &dn) = 0;

    // Returns whether edit's value has been changed by the user
    // Resets on reload
    virtual bool changed() const = 0;

    // Check that current input is valid for conditions that can be checked without contacting the AD server, for example name input not being empty
    virtual bool verify_input(QWidget *parent) = 0;

    // Apply current input by making a modification to the AD server
    virtual bool apply(const QString &dn) = 0;

signals:
    void edited();
};

#define DECL_ATTRIBUTE_EDIT_VIRTUALS()\
void add_to_layout(QGridLayout *layout);\
void load(const QString &dn);\
bool changed() const;\
bool verify_input(QWidget *parent);\
bool apply(const QString &dn);

class StringEdit final : public AttributeEdit {
Q_OBJECT
public:
    QLineEdit *edit;

    StringEdit(const QString &attribute_arg, const EditReadOnly read_only_arg = EditReadOnly_No);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QString attribute;
    QString original_value;
};

class GroupScopeEdit final : public AttributeEdit {
Q_OBJECT
public:
    GroupScopeEdit();
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QComboBox *combo;
    int original_value;
};

class GroupTypeEdit final : public AttributeEdit {
Q_OBJECT
public:
    QComboBox *combo;

    GroupTypeEdit();
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    int original_value;
};

class AccountOptionEdit final : public AttributeEdit {
Q_OBJECT
public:
    QCheckBox *check;

    AccountOptionEdit(const AccountOption option_arg);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    AccountOption option;
    bool original_value;
};

class PasswordEdit final : public AttributeEdit {
Q_OBJECT
public:
    QLineEdit *edit;
    QLineEdit *confirm_edit;

    PasswordEdit();
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    AccountOption option;
};

class DateTimeEdit final : public AttributeEdit {
Q_OBJECT
public:
    QDateTimeEdit *edit;

    DateTimeEdit(const QString &attribute_arg, const EditReadOnly read_only_arg = EditReadOnly_No);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QString attribute;
    QDateTime original_value;
};

#endif /* ATTRIBUTE_EDIT_H */
