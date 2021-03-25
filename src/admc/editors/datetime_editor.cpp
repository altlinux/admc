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

#include "editors/datetime_editor.h"
#include "ad/ad_config.h"
#include "globals.h"
#include "ad/ad_utils.h"
#include "utils.h"

#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

DateTimeEditor::DateTimeEditor(const QString attribute, const QList<QByteArray> values, QWidget *parent)
: AttributeEditor(parent)
{
    setWindowTitle(tr("Edit datetime"));

    QLabel *attribute_label = make_attribute_label(attribute);

    edit = new QDateTimeEdit();

    const QByteArray value = values.value(0, QByteArray());
    const QString value_string = QString(value);
    const QDateTime value_datetime = datetime_string_to_qdatetime(attribute, value_string, adconfig);
    edit->setDateTime(value_datetime);

    QDialogButtonBox *button_box = make_button_box(attribute);;

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(attribute_label);
    layout->addWidget(edit);
    layout->addWidget(button_box);

    const bool system_only = adconfig->get_attribute_is_system_only(attribute);
    if (system_only) {
        edit->setReadOnly(true);
    }
}

QList<QByteArray> DateTimeEditor::get_new_values() const {
    return QList<QByteArray>();
}
