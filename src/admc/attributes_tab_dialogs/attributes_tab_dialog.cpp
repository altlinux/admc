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

#include "attributes_tab_dialog.h"
#include "attributes_tab_dialog_string.h"
#include "attributes_tab_dialog_string_multi.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "status.h"
#include "utils.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

AttributesTabDialog *AttributesTabDialog::make(const QString attribute, const QList<QByteArray> values) {
    const bool single_valued = ADCONFIG()->get_attribute_is_single_valued(attribute);
    const AttributeType type = ADCONFIG()->get_attribute_type(attribute);

    switch (type) {
        case AttributeType_StringCase: {
            if (single_valued) {
                return new AttributesTabDialogString(attribute, values);
            } else {
                return new AttributesTabDialogStringMulti(attribute, values);
            }
        }
        case AttributeType_Unicode: {
            if (single_valued) {
                return new AttributesTabDialogString(attribute, values);
            } else {
                return new AttributesTabDialogStringMulti(attribute, values);
            }
        }
        default: return nullptr;
    }
}
