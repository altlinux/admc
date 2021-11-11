/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "attribute_edits/upn_edit.h"

#include "adldap.h"
#include "attribute_edits/upn_suffix_combo.h"
#include "globals.h"
#include "utils.h"

#include <QComboBox>
#include <QLineEdit>

UpnEdit::UpnEdit(QLineEdit *prefix_edit_arg, QComboBox *upn_suffix_combo_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    prefix_edit = prefix_edit_arg;
    upn_suffix_combo = upn_suffix_combo_arg;

    limit_edit(prefix_edit, ATTRIBUTE_USER_PRINCIPAL_NAME);

    connect(
        prefix_edit, &QLineEdit::textChanged,
        this, &AttributeEdit::edited);
    connect(
        upn_suffix_combo, &QComboBox::currentTextChanged,
        this, &AttributeEdit::edited);
}

void UpnEdit::init_suffixes(AdInterface &ad) {
    upn_suffix_combo_init(upn_suffix_combo, ad);
}

void UpnEdit::load_internal(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    upn_suffix_combo_load(upn_suffix_combo, object);

    const QString prefix = object.get_upn_prefix();
    prefix_edit->setText(prefix);
}

void UpnEdit::set_read_only(const bool read_only) {
    prefix_edit->setDisabled(read_only);
}

bool UpnEdit::verify(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();

    if (new_value.isEmpty()) {
        const QString text = tr("UPN may not be empty.");
        message_box_warning(prefix_edit, tr("Error"), text);

        return false;
    }

    // Check that new upn is unique
    // NOTE: filter has to also check that it's not the same object because of attribute edit weirdness. If user edits logon name, then retypes original, then applies, the edit will apply because it was modified by the user, even if the value didn't change. Without "not_object_itself", this check would determine that object's logon name conflicts with itself.
    const QString base = g_adconfig->domain_dn();
    const SearchScope scope = SearchScope_All;
    const QString filter = [=]() {
        const QString not_object_itself = filter_CONDITION(Condition_NotEquals, ATTRIBUTE_DN, dn);
        const QString same_upn = filter_CONDITION(Condition_Equals, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);

        return filter_AND({same_upn, not_object_itself});
    }();
    const QList<QString> attributes = QList<QString>();

    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    const bool upn_not_unique = (results.size() > 0);

    if (upn_not_unique) {
        const QString text = tr("The specified user logon name already exists.");
        message_box_warning(prefix_edit, tr("Error"), text);

        return false;
    }

    return true;
}

bool UpnEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();
    const bool success = ad.attribute_replace_string(dn, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);

    return success;
}

QString UpnEdit::get_new_value() const {
    const QString prefix = prefix_edit->text();
    const QString suffix = upn_suffix_combo->currentText();
    return QString("%1@%2").arg(prefix, suffix);
}
