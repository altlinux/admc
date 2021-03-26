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

#include "edits/upn_edit.h"

#include "utils.h"
#include "adldap.h"
#include "globals.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>

QString get_domain_as_email_suffix();

UpnEdit::UpnEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    edit = new QLineEdit();
    
    limit_edit(edit, ATTRIBUTE_USER_PRINCIPAL_NAME);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void UpnEdit::load_internal(const AdObject &object) {
    const QString value =
    [=]() {
        QString out = object.get_string(ATTRIBUTE_USER_PRINCIPAL_NAME);
        // Take "user" from "user@domain.com"
        const int at_index = out.lastIndexOf('@');
        out = out.left(at_index);

        return out;
    }();
    
    edit->setText(value);
}

void UpnEdit::set_read_only(const bool read_only) {
    edit->setReadOnly(read_only);
}

void UpnEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = adconfig->get_attribute_display_name(ATTRIBUTE_USER_PRINCIPAL_NAME, CLASS_USER) + ":";
    
    const QString extra_edit_text = get_domain_as_email_suffix();
    auto extra_edit = new QLineEdit();
    extra_edit->setEnabled(false);
    extra_edit->setText(extra_edit_text);

    auto sublayout = new QHBoxLayout();
    sublayout->addWidget(edit);
    sublayout->addWidget(extra_edit);

    layout->addRow(label_text, sublayout);
}

bool UpnEdit::verify(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();

    if (new_value.isEmpty()) {
        const QString text = tr("UPN may not be empty.");
        QMessageBox::warning(edit, tr("Error"), text);

        return false;
    }

    // Check that new upn is unique
    // NOTE: filter has to also check that it's not the same object because of attribute edit weirdness. If user edits logon name, then retypes original, then applies, the edit will apply because it was modified by the user, even if the value didn't change. Without "not_object_itself", this check would determine that object's logon name conflicts with itself.
    const QString filter =
    [=]() {
        const QString not_object_itself = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_DN, dn);
        const QString same_upn = filter_CONDITION(Condition_Equals, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);

        return filter_AND({same_upn, not_object_itself});
    }();
    const QList<QString> search_attributes;
    const QString base = adconfig->domain_head();

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All, base);

    const bool upn_not_unique = (search_results.size() > 0);

    if (upn_not_unique) {
        const QString text = tr("The specified user logon name already exists.");
        QMessageBox::warning(edit, tr("Error"), text);

        return false;
    }

    return true;
}

bool UpnEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);

    return success;
}

QString UpnEdit::get_input() const {
    return edit->text();
}

QString UpnEdit::get_new_value() const {
    // Get "user" from edit and combine into "user@domain.com"
    return edit->text() + get_domain_as_email_suffix();
}

// "DOMAIN.COM" => "@domain.com"
QString get_domain_as_email_suffix() {
    const QString domain = adconfig->domain();
    return "@" + domain.toLower();
}
