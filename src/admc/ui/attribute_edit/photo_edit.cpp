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

#include <QBuffer>
#include <QLabel>
#include <QPixmap>

#include "ad_defines.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "core/globals.h"
#include "photo_edit.h"
#include "ui/status.h"

static const int THUMBNAIL_HEIGHT = 96;
static const int THUMBNAIL_WIDTH  = 96;

PhotoEdit::PhotoEdit(QLabel *label, QObject *parent)
    : AttributeEdit(parent), photo_label(label)
{
    // Do nothing.
}

QPixmap PhotoEdit::get_thumbnail_photo() const {
    return thumbnail_photo;
}

void PhotoEdit::clear_photo() {
    thumbnail_photo = QPixmap();
    photo_label->clear();
}

/**
 * Crop a photo to the specified size.  The crop are is centered.
 *
 * @param photo A photo to crop.
 * @param width A target width.
 * @param height A target height.
 * @return A cropped photo.
 */
QPixmap PhotoEdit::crop_photo(const QPixmap &photo,
                              int width,
                              int height) const {
    int w = photo.width();
    int h = photo.height();
    int dw = w - width;
    int dh = h - height;
    int x = (dw > 0) ? dw / 2 : 0;
    int y = (dh > 0) ? dh / 2 : 0;
    QRect rect(x, y, width, height);
    return photo.copy(rect);
}

/**
 * Scale user photo to fit it into specified dimensions.
 *
 * @param photo A user photo to scale.
 * @param width A target photo width.
 * @param height A Target photo height.
 * @return A scaled photo.
 */
QPixmap PhotoEdit::scale_photo(const QPixmap &photo,
                               int width,
                               int height) const {
    int w = photo.width();
    int h = photo.height();
    QPixmap scaled;
    if (w < h) {
        scaled = photo.scaledToWidth(width);
    } else {
        scaled = photo.scaledToHeight(height);
    }
    return crop_photo(scaled, width, height);
}

void PhotoEdit::set_thumbnail_photo(QPixmap &photo) {
    thumbnail_photo = photo;
    emit AttributeEdit::edited();
}

void PhotoEdit::load_thumbnail_photo(QByteArray &data) {
    bool result = thumbnail_photo.loadFromData(data, "JPEG");
    if (result) {
        int w = thumbnail_photo.width();
        int h = thumbnail_photo.height();
        if ((w > THUMBNAIL_WIDTH) || (h > THUMBNAIL_HEIGHT)) {
            QPixmap scaled_photo = scale_photo(thumbnail_photo,
                                               THUMBNAIL_WIDTH,
                                               THUMBNAIL_HEIGHT);
            photo_label->setPixmap(scaled_photo);
        } else {
            photo_label->setPixmap(thumbnail_photo);
        }
    } else {
        g_status->add_message(tr("Could not load user photo"),
                              StatusType_Error);
        clear_photo();
    }
}

void PhotoEdit::load(AdInterface &ad, const AdObject &object) {
    Q_UNUSED(ad);
    QByteArray thumbnail_data = object.get_value(ATTRIBUTE_THUMBNAIL_PHOTO);
    if (! thumbnail_data.isEmpty()) {
        load_thumbnail_photo(thumbnail_data);
    } else {
        clear_photo();
    }
}

bool PhotoEdit::apply(AdInterface &ad, const QString &dn) const {
    if (! ad.is_connected()) {
        return false;
    }

    bool thumbnail_result = false;
    if (! thumbnail_photo.isNull()) {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        thumbnail_photo.save(&buffer, "JPEG");
        thumbnail_result = ad.attribute_replace_value(dn,
                                                      ATTRIBUTE_THUMBNAIL_PHOTO,
                                                      data);
    } else {
        QByteArray data;
        thumbnail_result = ad.attribute_replace_value(dn,
                                                      ATTRIBUTE_THUMBNAIL_PHOTO,
                                                      data);
    }

    return thumbnail_result;
}
