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

#ifndef IMAGE_VIEW_DIALOG_H
#define IMAGE_VIEW_DIALOG_H

#include <QDialog>

namespace Ui {
    class ImageViewDialog;
}

/**
 * A simple image viewer.
 */
class ImageViewDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::ImageViewDialog *ui;

    ImageViewDialog(QPixmap &image, QWidget *parent);
    ~ImageViewDialog();
};

#endif /* ifndef IMAGE_VIEW_DIALOG_H */

