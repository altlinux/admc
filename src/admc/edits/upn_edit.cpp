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
#include <QComboBox>

UpnEdit::UpnEdit(QList<AttributeEdit *> *edits_out, AdInterface &ad, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    prefix_edit = new QLineEdit();
    suffix_combo = new QComboBox();

    limit_edit(prefix_edit, ATTRIBUTE_USER_PRINCIPAL_NAME);

    const QList<QString> suffixes =
    [&]() {
        QList<QString> out;

        const QString partitions_dn = g_adconfig->partitions_dn();
        const AdObject partitions_object = ad.search_object(partitions_dn);

        out = partitions_object.get_strings(ATTRIBUTE_UPN_SUFFIXES);

        const QString domain = g_adconfig->domain();
        const QString domain_suffix = domain.toLower();
        if (!out.contains(domain_suffix)) {
            out.append(domain_suffix);
        }

        return out;
    }();

    for (const QString &suffix : suffixes) {
        suffix_combo->addItem(suffix);
    }

    QObject::connect(
        prefix_edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
    QObject::connect(
        suffix_combo, &QComboBox::currentTextChanged,
        [this]() {
            emit edited();
        });
}

void UpnEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QString upn = object.get_string(ATTRIBUTE_USER_PRINCIPAL_NAME);
    const int split_index = upn.lastIndexOf('@');
    const QString prefix = upn.left(split_index);
    const QString current_suffix = upn.mid(split_index + 1);

    // Select current suffix in suffix combo. Add current
    // suffix to combo if it's not there already.
    const int current_suffix_index = suffix_combo->findText(current_suffix);
    if (current_suffix_index != -1) {
        suffix_combo->setCurrentIndex(current_suffix_index);
    } else {
        suffix_combo->addItem(current_suffix);
        
        const int added_index = suffix_combo->findText(current_suffix);
        suffix_combo->setCurrentIndex(added_index);
    }
    
    prefix_edit->setText(prefix);
}

void UpnEdit::set_read_only(const bool read_only) {
    prefix_edit->setReadOnly(read_only);
}

void UpnEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_USER_PRINCIPAL_NAME, CLASS_USER) + ":";
    
    auto sublayout = new QHBoxLayout();
    sublayout->addWidget(prefix_edit);
    sublayout->addWidget(suffix_combo);

    layout->addRow(label_text, sublayout);
}

bool UpnEdit::verify(AdInterface &ad, const QString &dn) const {
    const QString new_value = get_new_value();

    if (new_value.isEmpty()) {
        const QString text = tr("UPN may not be empty.");
        QMessageBox::warning(prefix_edit, tr("Error"), text);

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
    const QString base = g_adconfig->domain_head();

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All, base);

    const bool upn_not_unique = (search_results.size() > 0);

    if (upn_not_unique) {
        const QString text = tr("The specified user logon name already exists.");
        QMessageBox::warning(prefix_edit, tr("Error"), text);

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
    return prefix_edit->text();
}

QString UpnEdit::get_new_value() const {
    const QString prefix = prefix_edit->text();
    const QString suffix = suffix_combo->currentText();
    return QString("%1@%2").arg(prefix, suffix);
}
