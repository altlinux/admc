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

#ifndef SITES_LINK_CREATION_DIALOG_H
#define SITES_LINK_CREATION_DIALOG_H

#include "ui/dialog/create/object.h"
#include "ui/widget/tab/sites_link/type.h"

namespace Ui {
class CreateSitesLinkDialog;
}

class AdInterface;

class CreateSitesLinkDialog : public CreateObjectDialog {
    Q_OBJECT

public:
    explicit CreateSitesLinkDialog(AdInterface &ad, SitesLinkType type_arg, const QString parent_dn_arg, QWidget *parent = nullptr);
    ~CreateSitesLinkDialog();

    void accept() override;
    QString get_created_dn() const override;

private:
    Ui::CreateSitesLinkDialog *ui;
    SitesLinkType type;
    QString created_dn;
    QString parent_dn;
};

#endif // SITES_LINK_CREATION_DIALOG_H
