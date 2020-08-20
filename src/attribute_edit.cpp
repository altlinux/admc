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

#include "attribute_edit.h"
#include "attribute_display_strings.h"
#include "utils.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QMessageBox>

void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout, QWidget *parent) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}

bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent) {
    bool success = true;

    for (auto edit : edits) {
        const bool verify_success = edit->verify_input(parent);

        if (!verify_success) {
            success = false;
        }
    }

    return success;
}

// NOTE: apply and collect results, THEN show error popups, so that all AD requests are done first and there are no stutters between popups
// NOTE: turn off batch if you already started a batch outside
bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, ApplyAttributeEditBatch batch, QWidget *parent) {
    if (batch == ApplyAttributeEditBatch_Yes) {
        AdInterface::instance()->start_batch();
    }

    bool success = true;

    QList<AdResult> results;
    for (auto edit : edits) {
        const AdResult result = edit->apply(dn);
        results.append(result);
    }

    for (auto result : results) {
        if (!result.success) {
            success = false;

            QMessageBox::critical(parent, QObject::tr("Error"), result.error_with_context);
        }
    }

    if (batch == ApplyAttributeEditBatch_Yes) {
        AdInterface::instance()->end_batch();
    }

    return success;
}

void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out) {
    for (auto attribute : attributes) {
        auto edit = new StringEdit(attribute);
        edits_out->insert(attribute, edit);
    }
}

void make_accout_option_edits(const QList<AccountOption> options, QMap<AccountOption, AccountOptionEdit *> *edits_out) {
    for (auto option : options) {
        auto edit = new AccountOptionEdit(option);
        edits_out->insert(option, edit);
    }
}

StringEdit::StringEdit(const QString &attribute_arg) {
    edit = new QLineEdit();
    attribute = attribute_arg;
}

void StringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_string(attribute);
    QWidget *widget = edit;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

bool StringEdit::verify_input(QWidget *parent) {
    if (attribute == ATTRIBUTE_SAMACCOUNT_NAME) {
        const QString new_value = edit->text();

        if (new_value.isEmpty()) {
            const QString attribute_string = get_attribute_display_string(attribute);
            const QString error_text = QString(QObject::tr("Attribute \"%1\" cannot be empty!").arg(attribute_string));
            QMessageBox::warning(parent, QObject::tr("Error"), error_text);

            return false;
        }
    }

    return true;
}

AdResult StringEdit::apply(const QString &dn) {
    const QString new_value = edit->text();
    const QString old_value = AdInterface::instance()->attribute_get(dn, attribute);

    const bool need_to_apply = (new_value != old_value);

    AdResult result(true);
    if (need_to_apply) {
        result = AdInterface::instance()->attribute_replace(dn, attribute, new_value);
    }

    return result;
}

GroupScopeEdit::GroupScopeEdit() {
    combo = new QComboBox();

    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope type = (GroupScope) i;
        const QString type_string = group_scope_to_string(type);

        combo->addItem(type_string, (int)type);
    }
}

void GroupScopeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group scope");
    QWidget *widget = combo;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

bool GroupScopeEdit::verify_input(QWidget *parent) {
    return true;
}

AdResult GroupScopeEdit::apply(const QString &dn) {
    const GroupScope new_value = (GroupScope)combo->currentData().toInt();
    const GroupScope old_value = AdInterface::instance()->group_get_scope(dn);

    const bool need_to_apply = (new_value != old_value);

    AdResult result(true);
    if (need_to_apply) {
        result = AdInterface::instance()->group_set_scope(dn, new_value);
    }

    return result;
}

GroupTypeEdit::GroupTypeEdit() {
    combo = new QComboBox();

    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType type = (GroupType) i;
        const QString type_string = group_type_to_string(type);

        combo->addItem(type_string, (int)type);
    }
}

void GroupTypeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group type");
    QWidget *widget = combo;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

bool GroupTypeEdit::verify_input(QWidget *parent) {
    return true;
}

AdResult GroupTypeEdit::apply(const QString &dn) {
    const GroupType new_value = (GroupType)combo->currentData().toInt();
    const GroupType old_value = AdInterface::instance()->group_get_type(dn);

    const bool need_to_apply = (new_value != old_value);

    AdResult result(true);
    if (need_to_apply) {
        result = AdInterface::instance()->group_set_type(dn, new_value);
    }

    return result;
}

AccountOptionEdit::AccountOptionEdit(const AccountOption option_arg) {
    option = option_arg;
    check = new QCheckBox();
}

void AccountOptionEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_account_option_description(option);
    QWidget *widget = check;
    append_to_grid_layout_with_label(layout, label_text , widget);
}

bool AccountOptionEdit::verify_input(QWidget *parent) {
    return true;
}

AdResult AccountOptionEdit::apply(const QString &dn) {
    const bool new_value = checkbox_is_checked(check);
    const bool old_value = AdInterface::instance()->user_get_account_option(dn, option);

    const bool need_to_apply = (new_value != old_value);

    AdResult result(true);
    if (need_to_apply) {
        result = AdInterface::instance()->user_set_account_option(dn, option, new_value);
    }

    return result;
}
