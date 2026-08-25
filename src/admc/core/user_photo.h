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

#ifndef CORE_USER_PHOTO_H
#define CORE_USER_PHOTO_H

#include "ad_interface.h"

class QPixmap;
class QString;

// According to:
// <https://learn.microsoft.com/en-us/windows/win32/adschema/a-thumbnailphoto>
const qint64 MAX_THUMBNAIL_PHOTO_SIZE = 102400; // bytes

QPixmap user_crop_photo(const QPixmap &photo, int width, int height);
QPixmap user_scale_photo(const QPixmap &photo, int width, int height);
bool user_set_thumbnail_photo(AdInterface &ad,
                              const QString &dn,
                              const QPixmap &photo);
bool user_thumbnail_photo_file_check(const QString &file_name, QString &error);

#endif  /* ifndef CORE_USER_PHOTO_H */
