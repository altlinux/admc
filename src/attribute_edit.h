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

enum ApplyAttributeEditBatch {
    ApplyAttributeEditBatch_Yes,
    ApplyAttributeEditBatch_No
};

void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout, QWidget *parent);
bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent);
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, ApplyAttributeEditBatch batch, QWidget *parent);
void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out);
void make_accout_option_edits(const QList<AccountOption> options, QMap<AccountOption, AccountOptionEdit *> *edits_out);

// NOTE: verify_input reuses AdResult even though it doesn't actually call any AdInterface functions, not sure about this but seems to be decent for now. Might make more sense to have an overall Result, not specific to Ad, though for that only bool + string make sense. The thing with error vs error_with_context only makes sense for AdInterface. Though maybe that should be changed, leaving only error(with context), because error alone is barely used and doesn't seem necessary

class AttributeEdit {
public:
    virtual void add_to_layout(QGridLayout *layout) = 0;
    virtual bool verify_input(QWidget *parent) = 0;
    virtual AdResult apply(const QString &dn) = 0;
};

class StringEdit final : public AttributeEdit {
public:
    QLineEdit *edit;

    void add_to_layout(QGridLayout *layout);
    StringEdit(const QString &attribute_arg);

    bool verify_input(QWidget *parent);
    AdResult apply(const QString &dn);

private:
    QString attribute;
};

class GroupScopeEdit final : public AttributeEdit {
public:
    GroupScopeEdit();

    void add_to_layout(QGridLayout *layout);
    bool verify_input(QWidget *parent);
    AdResult apply(const QString &dn);

private:
    QComboBox *combo;
};

class GroupTypeEdit final : public AttributeEdit {
public:
    QComboBox *combo;

    GroupTypeEdit();

    void add_to_layout(QGridLayout *layout);
    bool verify_input(QWidget *parent);
    AdResult apply(const QString &dn);
};

class AccountOptionEdit final : public AttributeEdit {
public:
    QCheckBox *check;

    AccountOptionEdit(const AccountOption option_arg);

    void add_to_layout(QGridLayout *layout);
    bool verify_input(QWidget *parent);
    AdResult apply(const QString &dn);

private:
    AccountOption option;
};

class PasswordEdit final : public AttributeEdit {
public:
    QLineEdit *edit;
    QLineEdit *confirm_edit;

    PasswordEdit();

    void add_to_layout(QGridLayout *layout);
    bool verify_input(QWidget *parent);
    AdResult apply(const QString &dn);

private:
    AccountOption option;
};

#endif /* ATTRIBUTE_EDIT_H */
