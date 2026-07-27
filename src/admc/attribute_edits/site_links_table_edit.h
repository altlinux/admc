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

#ifndef SITELINKSTABLEEDIT_H
#define SITELINKSTABLEEDIT_H

#include "attribute_edit.h"

class QTableWidget;

enum SiteLinksTableColumn {
    SiteLinksTableColumn_ChainName,
    SiteLinksTableColumn_Transport,

    SiteLinksTableColumn_COUNT
};

class SiteLinksTableEdit final : public AttributeEdit {
    Q_OBJECT

public:
    SiteLinksTableEdit(QTableWidget *site_links_table_arg, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;

private:
    QTableWidget *site_links_table;

    bool site_exists(AdInterface &ad, const QString &site_dn) const;
};

#endif // SITELINKSTABLEEDIT_H
