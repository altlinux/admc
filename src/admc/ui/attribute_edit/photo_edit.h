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

#ifndef PHOTO_EDIT_H
#define PHOTO_EDIT_H

#include "attribute_edit.h"

class QLabel;

class PhotoEdit final : public AttributeEdit {
    Q_OBJECT

public:
    PhotoEdit(QLabel *label, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;
    QPixmap get_thumbnail_photo() const;
    void set_thumbnail_photo(QPixmap &photo);

private:
    QLabel *photo_label;
    QPixmap thumbnail_photo;

    void load_thumbnail_photo(QByteArray &data);
    void load_thumbnail_photo(const QPixmap &photo);
    void clear_photo();
};

#endif // ifndef PHOTO_EDIT_H
