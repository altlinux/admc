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

#include "edits/expiry_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QDateEdit>
#include <QFrame>

// TODO: do end of day by local time or UTC????

const QTime END_OF_DAY(23, 59);

ExpiryEdit::ExpiryEdit(QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent)
{
    never_check = new QCheckBox(tr("Never"));
    end_of_check = new QCheckBox(tr("End of:"));

    never_check->setAutoExclusive(true);
    end_of_check->setAutoExclusive(true);

    edit = new QDateEdit();

    auto button_group = new QButtonGroup(this);
    button_group->addButton(never_check);
    button_group->addButton(end_of_check);

    frame = new QFrame();
    frame->setFrameStyle(QFrame::Raised);
    frame->setFrameShape(QFrame::Box);

    auto frame_layout = new QVBoxLayout();
    frame->setLayout(frame_layout);
    frame_layout->addWidget(never_check);
    frame_layout->addWidget(end_of_check);
    frame_layout->addWidget(edit);

    connect(
        never_check, &QCheckBox::stateChanged,
        this, &ExpiryEdit::on_never_check);
    connect(
        end_of_check, &QCheckBox::stateChanged,
        this, &ExpiryEdit::on_end_of_check);
    connect(
        edit, &QDateEdit::dateChanged,
        [this]() {
            emit edited();
        });
}

void ExpiryEdit::load_internal(const AdObject &object) {
    const bool never =
    [object]() {
        const QString expiry_string = object.get_string(ATTRIBUTE_ACCOUNT_EXPIRES);
        return large_integer_datetime_is_never(expiry_string);
    }();

    if (never) {
        never_check->setChecked(true);

        end_of_check->setChecked(false);
        edit->setEnabled(false);
    } else {
        never_check->setChecked(false);

        end_of_check->setChecked(true);
        edit->setEnabled(true);
    }
    
    const QDate date =
    [=]() {
        if (never) {
            // Default to current date when expiry is never
            return QDate::currentDate();
        } else {
            const QDateTime datetime = object.get_datetime(ATTRIBUTE_ACCOUNT_EXPIRES, adconfig);
            return datetime.date();
        }
    }();

    edit->setDate(date);
}

void ExpiryEdit::set_read_only(const bool read_only) {
    never_check->setDisabled(read_only);
    end_of_check->setDisabled(read_only);
    edit->setReadOnly(read_only);
}

void ExpiryEdit::add_to_layout(QFormLayout *layout) {
    const QString label_text = adconfig->get_attribute_display_name(ATTRIBUTE_ACCOUNT_EXPIRES, "") + ":";

    layout->addRow(label_text, frame);
}

bool ExpiryEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool never = never_check->isChecked();

    if (never) {
        return ad.attribute_replace_string(dn, ATTRIBUTE_ACCOUNT_EXPIRES, AD_LARGE_INTEGER_DATETIME_NEVER_2);
    } else {
        const QDate date = edit->date();
        const QDateTime datetime(date, END_OF_DAY);

        return ad.attribute_replace_datetime(dn, ATTRIBUTE_ACCOUNT_EXPIRES, datetime);
    }
}

void ExpiryEdit::on_never_check() {
    if (never_check->isChecked()) {
        edit->setEnabled(false);

        emit edited();
    }
}

void ExpiryEdit::on_end_of_check() {
    if (end_of_check->isChecked()) {
        edit->setEnabled(true);

        emit edited();
    }
}
