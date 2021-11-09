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

#ifndef DATETIME_ATTRIBUTE_DIALOG_H
#define DATETIME_ATTRIBUTE_DIALOG_H

/**
 * Editor for datetimes. Note that this is read-only and get
 * f-nd always returns an empty list because all datetime
 * attributes are read-only.
 */

#include "attribute_dialogs/attribute_dialog.h"

namespace Ui {
class DatetimeAttributeDialog;
}

class DatetimeAttributeDialog final : public AttributeDialog {
    Q_OBJECT

public:
    Ui::DatetimeAttributeDialog *ui;

    DatetimeAttributeDialog(QWidget *parent);
    ~DatetimeAttributeDialog();

    void set_read_only(const bool read_only) override;
    void set_value_list(const QList<QByteArray> &values) override;
    QList<QByteArray> get_value_list() const override;
};

#endif /* DATETIME_ATTRIBUTE_DIALOG_H */
