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

#include "edits/unlock_edit.h"

#include "adldap.h"
#include "utils.h"

#include <QCheckBox>
#include <QFormLayout>

UnlockEdit::UnlockEdit(QList<AttributeEdit *> *edits_out, const UnlockEditStyle style_arg, QObject *parent)
: AttributeEdit(edits_out, parent) {
    auto check_arg = new QCheckBox();

    init(style_arg, check_arg);
}

UnlockEdit::UnlockEdit(QList<AttributeEdit *> *edits_out, QCheckBox *check_arg, QObject *parent)
: AttributeEdit(edits_out, parent) {
    init(UnlockEditStyle_CheckOnLeft, check_arg);
}

QString UnlockEdit::label_text() {
    return tr("Unlock account");
}

void UnlockEdit::load_internal(AdInterface &ad, const AdObject &object) {
    check->setChecked(false);
}

void UnlockEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

void UnlockEdit::add_to_layout(QFormLayout *layout) {
    switch (style) {
        case UnlockEditStyle_CheckOnLeft: {
            layout->addRow(check);

            break;
        }
        case UnlockEditStyle_CheckOnRight: {
            layout->addRow(QString("%1:").arg(label_text()), check);


            break;
        }
    }

}

bool UnlockEdit::apply(AdInterface &ad, const QString &dn) const {

    if (check->isChecked()) {
        const bool result = ad.user_unlock(dn);
        check->setChecked(false);

        return result;
    } else {
        return true;
    }
}

void UnlockEdit::init(const UnlockEditStyle style_arg, QCheckBox *check_arg) {
    style = style_arg;

    check = check_arg;

    // NOTE: if check is on left, then put text in the
    // checkbox
    const QString check_text = [&]() {
        switch (style) {
            case UnlockEditStyle_CheckOnLeft: return label_text();
            case UnlockEditStyle_CheckOnRight: return QString();
        }
        return QString();
    }();

    check->setText(check_text);

    connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}
