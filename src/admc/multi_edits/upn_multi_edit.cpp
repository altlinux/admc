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

#include "multi_edits/upn_multi_edit.h"

#include "utils.h"
#include "adldap.h"
#include "globals.h"
#include "edits/upn_suffix_widget.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>

UpnMultiEdit::UpnMultiEdit(QList<AttributeMultiEdit *> &edits_out, AdInterface &ad, QObject *parent)
: AttributeMultiEdit(edits_out, parent)
{
    upn_suffix_widget = new UpnSuffixWidget(ad);

    const QString label_text = g_adconfig->get_attribute_display_name(ATTRIBUTE_USER_PRINCIPAL_NAME, CLASS_USER) + ":";
    label->setText(label_text);

    connect(
        upn_suffix_widget, &UpnSuffixWidget::edited,
        this, &UpnMultiEdit::edited);

    set_enabled(false);
}

void UpnMultiEdit::add_to_layout(QFormLayout *layout) {
    layout->addRow(check_and_label_wrapper, upn_suffix_widget);
}

bool UpnMultiEdit::apply_internal(AdInterface &ad, const QString &target) {
    const QString new_value =
    [&]() {
        const AdObject current_object = ad.search_object(target);
        const QString current_prefix = current_object.get_upn_prefix();
        const QString new_suffix = upn_suffix_widget->get_suffix();
        
        return QString("%1@%2").arg(current_prefix, new_suffix);
    }();

    return ad.attribute_replace_string(target, ATTRIBUTE_USER_PRINCIPAL_NAME, new_value);
}

void UpnMultiEdit::set_enabled(const bool enabled) {
    upn_suffix_widget->set_enabled(enabled);
}
