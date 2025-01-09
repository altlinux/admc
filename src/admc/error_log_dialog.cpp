/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "error_log_dialog.h"
#include "ui_error_log_dialog.h"

#include "settings.h"

ErrorLogDialog::ErrorLogDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ErrorLogDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    settings_setup_dialog_geometry(SETTING_error_log_dialog_geometry, this);
}

ErrorLogDialog::~ErrorLogDialog() {
    delete ui;
}

void ErrorLogDialog::set_text(const QString &text) {
    ui->text_edit->setPlainText(text);
}
