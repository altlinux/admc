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

enum EditReadOnly {
    EditReadOnly_Yes,
    EditReadOnly_No
};

void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout, QWidget *parent);
bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent);
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, QWidget *parent);
bool apply_attribute_edit(AttributeEdit *edit, const QString &dn, QWidget *parent);
void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out);
QMap<AccountOption, AccountOptionEdit *> make_account_option_edits(const QList<AccountOption> options, QWidget *parent);
void connect_edits_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab);

class AttributeEdit {
public:
    virtual void load(const QString &dn) = 0;
    virtual void add_to_layout(QGridLayout *layout) = 0;
    virtual void connect_to_tab(DetailsTab *tab) const = 0;
    virtual bool verify_input(QWidget *parent) = 0;
    virtual bool changed(const QString &dn) const = 0;
    virtual AdResult apply(const QString &dn) = 0;
};

#define DECL_ATTRIBUTE_EDIT_VIRTUALS()\
void load(const QString &dn);\
void add_to_layout(QGridLayout *layout);\
void connect_to_tab(DetailsTab *tab) const;\
bool verify_input(QWidget *parent);\
bool changed(const QString &dn) const;\
AdResult apply(const QString &dn);

class StringEdit final : public AttributeEdit {
public:
    QLineEdit *edit;

    StringEdit(const QString &attribute_arg, const EditReadOnly read_only = EditReadOnly_No);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QString attribute;
    QString original_value;
};

class GroupScopeEdit final : public AttributeEdit {
public:
    GroupScopeEdit();
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QComboBox *combo;
    int original_value;
};

class GroupTypeEdit final : public AttributeEdit {
public:
    QComboBox *combo;

    GroupTypeEdit();
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    int original_value;
};

class AccountOptionEdit final : public AttributeEdit {
public:
    QCheckBox *check;

    AccountOptionEdit(const AccountOption option_arg);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    AccountOption option;
    bool original_value;
};

class PasswordEdit final : public AttributeEdit {
public:
    QLineEdit *edit;
    QLineEdit *confirm_edit;

    PasswordEdit();
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    AccountOption option;
};

class DateTimeEdit final : public AttributeEdit {
public:
    QDateTimeEdit *edit;

    DateTimeEdit(const QString &attribute_arg, const EditReadOnly read_only = EditReadOnly_No);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QString attribute;
    QDateTime original_value;
};

#endif /* ATTRIBUTE_EDIT_H */
