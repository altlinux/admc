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
#include "details_tab.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QLabel>

void connect_changed_marker(AttributeEdit *edit, QLabel *label) {
    if (edit->read_only == EditReadOnly_Yes) {
        return;
    }

    QObject::connect(edit, &AttributeEdit::edited,
        [=]() {
            const QString current_text = label->text();
            const QString new_text = set_changed_marker(current_text, edit->changed());
            label->setText(new_text);
        });
}

void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}

bool any_edits_changed(QList<AttributeEdit *> edits) {
    for (auto edit : edits) {
        if (edit->changed()) {
            return true;
        }
    }

    return false;
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

bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, QWidget *parent) {
    bool success = true;

    for (auto edit : edits) {
        if (edit->changed()) {
            const bool apply_success = edit->apply(dn);
            if (!apply_success) {
                success = false;
            }
        }
    }

    return success;
}

void load_attribute_edits(QList<AttributeEdit *> edits, const QString &dn) {
    for (auto edit : edits) {
        edit->load(dn);
    }
}

void make_string_edits(const QList<QString> attributes, QMap<QString, StringEdit *> *edits_out) {
    for (auto attribute : attributes) {
        auto edit = new StringEdit(attribute);
        edits_out->insert(attribute, edit);
    }
}

QMap<AccountOption, AccountOptionEdit *> make_account_option_edits(const QList<AccountOption> options, QWidget *parent) {
    QMap<AccountOption, AccountOptionEdit *> edits;

    for (auto option : options) {
        auto edit = new AccountOptionEdit(option);
        edits.insert(option, edit);
    }

    // PasswordExpired conflicts with (DontExpirePassword and CantChangePassword)
    // When PasswordExpired is set, the other two can't be set
    // When any of the other two are set, PasswordExpired can't be set
    // Implement this by connecting to state changes of all options and
    // resetting to previous state if state transition is invalid
    auto setup_conflict =
    [parent, edits](const AccountOption subject, const AccountOption blocker) {
        QCheckBox *subject_check = edits[subject]->check;
        QCheckBox *blocker_check = edits[blocker]->check;

        QObject::connect(subject_check, &QCheckBox::stateChanged,
            [subject, blocker, subject_check, blocker_check, parent]() {
                if (checkbox_is_checked(subject_check) && checkbox_is_checked(blocker_check)) {
                    subject_check->setCheckState(Qt::Unchecked);

                    const QString subject_name = get_account_option_description(subject);
                    const QString blocker_name = get_account_option_description(blocker);
                    const QString error = QString(QObject::tr("Can't set \"%1\" when \"%2\" is set.")).arg(blocker_name, subject_name);
                    QMessageBox::warning(parent, QObject::tr("Error"), error);
                }
            }
            );
    };

    // NOTE: only setup conflicts for options that exist
    if (options.contains(AccountOption_PasswordExpired)) {
        const QList<AccountOption> other_two_options = {
            AccountOption_DontExpirePassword,
            // TODO: AccountOption_CantChangePassword
        };

        for (auto other_option : other_two_options) {
            if (options.contains(other_option)) {
                setup_conflict(AccountOption_PasswordExpired, other_option);
                setup_conflict(other_option, AccountOption_PasswordExpired);
            }
        }
    }

    return edits;
}

void connect_edits_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab) {
    for (auto edit : edits) {
        QObject::connect(
            edit, &AttributeEdit::edited,
            tab, &DetailsTab::on_edit_changed);
    }
}

void autofill_full_name(QMap<QString, StringEdit *> string_edits) {
    const char *name_attributes[] = {
        ATTRIBUTE_FIRST_NAME,
        ATTRIBUTE_LAST_NAME,
        ATTRIBUTE_DISPLAY_NAME
    };

    // Get QLineEdit's out of string edits
    QMap<QString, QLineEdit *> edits;
    for (auto attribute : name_attributes) {
        if (string_edits.contains(attribute)) {
            edits[attribute] = string_edits[attribute]->edit;
        } else {
            printf("Error in autofill_full_name(): first, last or full name is not present in edits list!");
            return;
        }
    }

    auto autofill =
    [=]() {
        const QString first_name = edits[ATTRIBUTE_FIRST_NAME]->text(); 
        const QString last_name = edits[ATTRIBUTE_LAST_NAME]->text();
        const QString full_name = first_name + " " + last_name; 

        edits[ATTRIBUTE_DISPLAY_NAME]->setText(full_name);
    };

    QObject::connect(
        edits[ATTRIBUTE_FIRST_NAME], &QLineEdit::textChanged,
        autofill);
    QObject::connect(
        edits[ATTRIBUTE_LAST_NAME], &QLineEdit::textChanged,
        autofill);
}

void autofill_sama_name(StringEdit *sama_edit, StringEdit *name_edit) {
    QObject::connect(
        sama_edit->edit, &QLineEdit::textChanged,
        [=] () {
            sama_edit->edit->setText(name_edit->edit->text());
        });
}

AttributeEdit::AttributeEdit() {
    read_only = EditReadOnly_No;
}

AttributeEdit::AttributeEdit(EditReadOnly read_only_arg) {
    read_only = read_only_arg;
}

StringEdit::StringEdit(const QString &attribute_arg, const EditReadOnly read_only_arg)
: AttributeEdit(read_only_arg)
{
    edit = new QLineEdit();
    attribute = attribute_arg;

    if (read_only == EditReadOnly_Yes) {
        edit->setReadOnly(true);
    }

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::load(const QString &dn) {
    QString value;
    if (attribute == ATTRIBUTE_OBJECT_CLASS) {
        // NOTE: object class is multi-valued so need to get the "primary" class
        // TODO: not sure how to get the "primary" attribute, for now just getting the last one. I think what I need to do is get the most "derived" class? and that info should be in the scheme.
        const QList<QString> classes = AdInterface::instance()->attribute_get_multi(dn, attribute);
        value = classes.last();
    } else {
        value = AdInterface::instance()->attribute_get(dn, attribute);
    }

    edit->blockSignals(true);
    edit->setText(value);
    edit->blockSignals(false);

    original_value = value;

    emit edited();
}

void StringEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_string(attribute);
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, edit);
}

bool StringEdit::verify_input(QWidget *parent) {
    static const QList<QString> cant_be_empty = {
        ATTRIBUTE_NAME,
        ATTRIBUTE_SAMACCOUNT_NAME
    };

    if (cant_be_empty.contains(attribute)) {
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

bool StringEdit::changed() const {
    const QString new_value = edit->text();
    return (new_value != original_value);
}

bool StringEdit::apply(const QString &dn) {
    // NOTE: name can't be replaced regularly so don't apply it. Need to get value from this edit and manually rename/create object
    if (attribute == ATTRIBUTE_NAME) {
        return true;
    }

    const QString new_value = edit->text();
    const bool success = AdInterface::instance()->attribute_replace(dn, attribute, new_value);

    return success;
}

GroupScopeEdit::GroupScopeEdit() {
    combo = new QComboBox();

    for (int i = 0; i < GroupScope_COUNT; i++) {
        const GroupScope type = (GroupScope) i;
        const QString type_string = group_scope_to_string(type);

        combo->addItem(type_string, (int)type);
    }

    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this]() {
            emit edited();
        });
}

void GroupScopeEdit::load(const QString &dn) {
    const GroupScope scope = AdInterface::instance()->group_get_scope(dn);
    const int scope_int = (int)scope;

    combo->blockSignals(true);
    combo->setCurrentIndex(scope_int);
    combo->blockSignals(false);

    original_value = scope_int;

    emit edited();
}

void GroupScopeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group scope");
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, combo);
}

bool GroupScopeEdit::verify_input(QWidget *parent) {
    return true;
}

bool GroupScopeEdit::changed() const {
    const int new_value = combo->currentData().toInt();
    return (new_value != original_value);
}

bool GroupScopeEdit::apply(const QString &dn) {
    const GroupScope new_value = (GroupScope)combo->currentData().toInt();
    const bool success = AdInterface::instance()->group_set_scope(dn, new_value);

    return success;
}

GroupTypeEdit::GroupTypeEdit() {
    combo = new QComboBox();

    for (int i = 0; i < GroupType_COUNT; i++) {
        const GroupType type = (GroupType) i;
        const QString type_string = group_type_to_string(type);

        combo->addItem(type_string, (int)type);
    }

    QObject::connect(
        combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this]() {
            emit edited();
        });
}

void GroupTypeEdit::load(const QString &dn) {
    const GroupType type = AdInterface::instance()->group_get_type(dn);
    const int type_int = (int)type;

    combo->blockSignals(true);
    combo->setCurrentIndex(type_int);
    combo->blockSignals(false);

    original_value = type_int;

    emit edited();
}

void GroupTypeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = QObject::tr("Group type");
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, combo);
}

bool GroupTypeEdit::verify_input(QWidget *parent) {
    return true;
}

bool GroupTypeEdit::changed() const {
    const int new_value = combo->currentData().toInt();
    return (new_value != original_value);
}

bool GroupTypeEdit::apply(const QString &dn) {
    const GroupType new_value = (GroupType)combo->currentData().toInt();
    const bool success = AdInterface::instance()->group_set_type(dn, new_value);

    return success;
}

AccountOptionEdit::AccountOptionEdit(const AccountOption option_arg) {
    option = option_arg;
    check = new QCheckBox();

    QObject::connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void AccountOptionEdit::load(const QString &dn) {
    const bool option_is_set = AdInterface::instance()->user_get_account_option(dn, option);

    Qt::CheckState check_state;
    if (option_is_set) {
        check_state = Qt::Checked;
    } else {
        check_state = Qt::Unchecked;
    }
    
    check->blockSignals(true);
    check->setCheckState(check_state);
    check->blockSignals(false);

    original_value = option_is_set;

    emit edited();
}

void AccountOptionEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_account_option_description(option);
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);
    append_to_grid_layout_with_label(layout, label, check);
}

bool AccountOptionEdit::verify_input(QWidget *parent) {
    return true;
}

bool AccountOptionEdit::changed() const {
    const bool new_value = checkbox_is_checked(check);
    return (new_value != original_value);
}

bool AccountOptionEdit::apply(const QString &dn) {
    const bool new_value = checkbox_is_checked(check);
    const bool success = AdInterface::instance()->user_set_account_option(dn, option, new_value);

    return success;
}

PasswordEdit::PasswordEdit() {
    edit = new QLineEdit();
    confirm_edit = new QLineEdit();

    edit->setEchoMode(QLineEdit::Password);
    confirm_edit->setEchoMode(QLineEdit::Password);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void PasswordEdit::load(const QString &dn) {
    // NOTE: PasswordEdit does not load current value, it starts out blank and is not reloaded
    emit edited();
}

void PasswordEdit::add_to_layout(QGridLayout *layout) {
    const auto password_label = new QLabel(QObject::tr("Password"));
    const auto confirm_label = new QLabel(QObject::tr("Confirm password"));

    connect_changed_marker(this, password_label);
    connect_changed_marker(this, confirm_label);

    append_to_grid_layout_with_label(layout, password_label, edit);
    append_to_grid_layout_with_label(layout, confirm_label, confirm_edit);
}

bool PasswordEdit::verify_input(QWidget *parent) {
    const QString pass = edit->text();
    const QString confirm_pass = confirm_edit->text();

    if (pass != confirm_pass) {
        const QString error_text = QString(QObject::tr("Passwords don't match!"));
        QMessageBox::warning(parent, QObject::tr("Error"), error_text);

        return false;
    }

    return true;
}

bool PasswordEdit::changed() const {
    return true;
}

bool PasswordEdit::apply(const QString &dn) {
    const QString new_value = edit->text();

    const bool success = AdInterface::instance()->set_pass(dn, new_value);

    return success;
}

DateTimeEdit::DateTimeEdit(const QString &attribute_arg, EditReadOnly read_only_arg)
: AttributeEdit(read_only_arg)
{
    edit = new QDateTimeEdit();
    attribute = attribute_arg;

    if (read_only == EditReadOnly_Yes) {
        edit->setReadOnly(true);
    }

    QObject::connect(
        edit, &QDateTimeEdit::dateTimeChanged,
        [this]() {
            emit edited();
        });
}

void DateTimeEdit::load(const QString &dn) {
    const QDateTime value = AdInterface::instance()->attribute_datetime_get(dn, attribute);

    edit->blockSignals(true);
    edit->setDateTime(value);
    edit->blockSignals(false);

    original_value = value;

    emit edited();
}

void DateTimeEdit::add_to_layout(QGridLayout *layout) {
    const QString label_text = get_attribute_display_string(attribute);
    const auto label = new QLabel(label_text);

    connect_changed_marker(this, label);

    append_to_grid_layout_with_label(layout, label, edit);
}

bool DateTimeEdit::verify_input(QWidget *parent) {
    // TODO: datetime should fit within bounds of it's format, so greater than start of epoch for NTFS format?

    return true;
}

bool DateTimeEdit::changed() const {
    const QDateTime new_value = edit->dateTime();
    return (new_value != original_value);
}

bool DateTimeEdit::apply(const QString &dn) {
    const QDateTime new_value = edit->dateTime();

    const bool success = AdInterface::instance()->attribute_datetime_replace(dn, attribute, new_value);

    return success;
}
