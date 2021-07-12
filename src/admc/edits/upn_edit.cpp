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

#include "edits/upn_edit.h"

#include "adldap.h"
#include "edits/upn_suffix_widget.h"
#include "globals.h"
#include "utils.h"

#include <QFormLayout>
#include <QLineEdit>

UpnEdit::UpnEdit(QList<AttributeEdit *> *edits_out, AdInterface &ad, QObject *parent)
: AttributeEdit(edits_out, parent) {
    prefix_edit = new QLineEdit();
    upn_suffix_widget = new UpnSuffixWidget(ad);

    limit_edit(prefix_edit, ATTRIBUTE_USER_PRINCIPAL_NAME);

    QObject::connect(
        prefix_edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
    QObject::connect(
        upn_suffix_widget, &UpnSuffixWidget::edited,
        this, &UpnEdit::edited);
}

void UpnEdit::load_internal(AdInterface &ad, const AdObject &object) {
    upn_suffix_widget->load(object);

    const QString prefix = object.get_upn_prefix();
    prefix_edit->setText(prefix);
}

void UpnEdit::set_read_only(const bool read_only) {
    prefix_edit->setReadOnly(read_only);
}

void UpnEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_USER_PRINCIPAL_NAME, CLASS_USER) + ":";

    auto sublayout = new QHBoxLayout();
    sublayout->addWidget(prefix_edit);
    sublayout->addWidget(upn_suffix_widget);

    layout->addRow(label_text, sublayout);
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
    const QString base = g_adconfig->domain_head();
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

QLineEdit *UpnEdit::get_edit() const {
    return prefix_edit;
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
    const QString suffix = upn_suffix_widget->get_suffix();
    return QString("%1@%2").arg(prefix, suffix);
}
