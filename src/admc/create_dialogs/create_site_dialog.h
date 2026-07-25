/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
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

#ifndef CREATE_SITE_DIALOG_H
#define CREATE_SITE_DIALOG_H

#include "create_object_dialog.h"

class CreateObjectHelper;
class AdInterface;

namespace Ui {
class CreateSiteDialog;
}

class CreateSiteDialog : public CreateObjectDialog {
    Q_OBJECT

public:
    explicit CreateSiteDialog(AdInterface &ad, QWidget *parent = nullptr);
    ~CreateSiteDialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    Ui::CreateSiteDialog *ui;

    CreateObjectHelper *create_site_helper;
    CreateObjectHelper *create_ntds_settings_helper;
    CreateObjectHelper *create_servers_container_helper;
};

#endif // CREATE_SITE_DIALOG_H
