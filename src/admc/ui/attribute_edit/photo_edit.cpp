/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QLabel>
#include <QPixmap>

#include "ad_defines.h"
#include "ad_object.h"
#include "core/globals.h"
#include "photo_edit.h"
#include "ui/status.h"

PhotoEdit::PhotoEdit(QLabel *label, QObject *parent)
    : AttributeEdit(parent), photo_label(label)
{
    // Do nothing.
}

void PhotoEdit::clear_photo() {
    photo_label->clear();
}

void PhotoEdit::load_photo(QByteArray &data) {
    QPixmap photo;
    bool result = photo.loadFromData(data, "JPEG");
    if (result) {
        photo_label->setPixmap(photo);
    } else {
        g_status->add_message(tr("Could not load user photo"),
                              StatusType_Error);
        clear_photo();
    }
}

void PhotoEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);
    QByteArray data = object.get_value(ATTRIBUTE_JPEG_PHOTO);
    if (! data.isEmpty()) {
        load_photo(data);
    } else {
        clear_photo();
    }
}

bool PhotoEdit::apply(AdInterface &ad, const QString &dn) const {
    Q_UNUSED(ad);
    Q_UNUSED(dn);
    // TODO: Implement photo editing.
    return true;
}
