/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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

#include <QPixmap>

/**
 * Crop a photo to the specified size.  The crop are is centered.
 *
 * @param photo A photo to crop.
 * @param width A target width.
 * @param height A target height.
 * @return A cropped photo.
 */
QPixmap user_crop_photo(const QPixmap &photo, int width, int height) {
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
QPixmap user_scale_photo(const QPixmap &photo, int width, int height) {
    int w = photo.width();
    int h = photo.height();
    QPixmap scaled;
    if (w < h) {
        scaled = photo.scaledToWidth(width);
    } else {
        scaled = photo.scaledToHeight(height);
    }
    return user_crop_photo(scaled, width, height);
}
